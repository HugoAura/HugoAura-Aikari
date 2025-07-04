#include "pch.h"

#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Launcher-Private/types/global/lifecycleTypes.h>
#include <Aikari-Launcher-Public/version.h>
#include <Aikari-PLS/Aikari-PLS-Exports.h>
#include <Aikari-PLS/types/entrypoint.h>
#include <Aikari-Shared/infrastructure/logger.h>
#include <chrono>
#include <csignal>
#include <cxxopts.hpp>
#include <future>
#include <ixwebsocket/IXNetSystem.h>

#include "components/config.h"
#include "components/threadMsgHandlers.h"
#include "components/wsServer.h"
#include "infrastructure/cliParse.h"
#include "infrastructure/fileSystem.h"
#include "infrastructure/registry.h"
#include "infrastructure/winSvc.h"
#include "lifecycle.h"
#include "resource.h"
#include "utils/sslUtils.h"

namespace lifecycleTypes = AikariTypes::global::lifecycle;

std::promise<bool> aikariAlivePromise;

static void logVersion()
{
    LOG_INFO(
        "⚛️ Version: " + AikariLauncherPublic::version::Version +
        std::format(" ({})", AikariLauncherPublic::version::VersionCode)
    );
}

static void exitSignalHandler(int signum)
{
    LOG_INFO(
        "Received " + std::to_string(signum) + " signal, exiting Aikari..."
    );
    aikariAlivePromise.set_value(true);
}

int launchAikari(lifecycleTypes::APPLICATION_RUNTIME_MODES& runtimeMode)
{
    std::future<bool> aikariAliveFuture = aikariAlivePromise.get_future();

    auto curTime = std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now().time_since_epoch()
    )
                       .count();

    auto& lifecycleStates = AikariLifecycle::AikariStatesManager::getInstance();
    LOG_INFO("Initializing states manager...");
    lifecycleStates.setVal(
        &lifecycleTypes::GlobalLifecycleStates::runtimeMode, runtimeMode
    );
    lifecycleStates.setVal(
        &lifecycleTypes::GlobalLifecycleStates::launchTime, curTime
    );

    auto& sharedInstances =
        AikariLifecycle::AikariSharedInstances::getInstance();

    LOG_INFO("Initializing file system manager...");
    {
        auto fileSystemManagerIns =
            std::make_unique<AikariFileSystem::FileSystemManager>();
        fileSystemManagerIns->ensureDirExists();
        sharedInstances.setPtr(
            &lifecycleTypes::SharedInstances::fsManagerIns,
            std::move(fileSystemManagerIns)
        );
    }
    auto* fileSystemManagerIns =
        sharedInstances.getPtr(&lifecycleTypes::SharedInstances::fsManagerIns);

    LOG_INFO("Initializing registry manager...");
    {
        auto registryManagerIns =
            std::make_unique<AikariRegistry::RegistryManager>();
        sharedInstances.setPtr(
            &lifecycleTypes::SharedInstances::registryManagerIns,
            std::move(registryManagerIns)
        );
    }
    auto* registryManagerPtr = sharedInstances.getPtr(
        &lifecycleTypes::SharedInstances::registryManagerIns
    );
    int regKeyValidateResult = registryManagerPtr->ensureRegKeyExists();
    if (regKeyValidateResult == -1)
    {
        LOG_CRITICAL("Registry initialization failed, exiting Aikari...");
        return -1;
    }

    LOG_INFO("Initializing config manager...");
    {
        auto configManagerIns = std::make_unique<
            AikariLauncherComponents::AikariConfig::LauncherConfigManager>(
            "launcher",
            fileSystemManagerIns->aikariConfigDir / "config.json",
            IDR_AIKARI_DEFAULT_JSON
        );
        sharedInstances.setPtr(
            &lifecycleTypes::SharedInstances::configManagerIns,
            std::move(configManagerIns)
        );
    }
    auto* configManagerPtr = sharedInstances.getPtr(
        &lifecycleTypes::SharedInstances::configManagerIns
    );
    bool initConfigResult = configManagerPtr->initConfig();
    if (!initConfigResult)
    {
        LOG_CRITICAL("Config initialization failed, exiting Aikari...");
        return -1;
    }

    LOG_INFO("Initializing TLS certificates...");
    std::filesystem::path certDir =
        fileSystemManagerIns->aikariConfigDir / "certs";
    bool wsCertInitResult = AikariUtils::sslUtils::initWsCert(
        certDir, configManagerPtr->config->tls.regenWsCertNextLaunch
    );
    if (!wsCertInitResult)
    {
        LOG_CRITICAL(
            "Failed to initialize WebSocket TLS cert, exiting Aikari..."
        );
        return -1;
    }

    LOG_INFO("Initializing Windows socket environment...");
    ix::initNetSystem();
    LOG_INFO("Starting Aikari WebSocket server...");
    int wsDefaultPort = configManagerPtr->config->wsPreferPort;
    {
        auto wsServerManagerIns = std::make_unique<
            AikariLauncherComponents::AikariWebSocketServer::MainWSServer>(
            "127.0.0.1", wsDefaultPort, certDir / "wss.crt", certDir / "wss.key"
        );
        sharedInstances.setPtr(
            &lifecycleTypes::SharedInstances::wsServerMgrIns,
            std::move(wsServerManagerIns)
        );
    }
    auto* wsServerMgrPtr =
        sharedInstances.getPtr(&lifecycleTypes::SharedInstances::wsServerMgrIns
        );
    bool wsLaunchResult = wsServerMgrPtr->tryLaunchWssServer();
    if (!wsLaunchResult)
    {
        LOG_CRITICAL("Failed to launch Aikari ws server, exiting Aikari...");
        return -1;
    }

    auto curSharedMsgQueues = lifecycleStates.getVal(
        &lifecycleTypes::GlobalLifecycleStates::sharedMsgQueue
    );

    auto plsInputMsgQueue = std::make_shared<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariShared::Types::InterThread::MainToSubMessageInstance>>();

    curSharedMsgQueues.plsInputQueue = plsInputMsgQueue;

    AikariPLS::Types::entrypoint::EntrypointRet plsLaunchResult =
        AikariPLS::Exports::main(
            fileSystemManagerIns->aikariRootDir, certDir, plsInputMsgQueue
        );

    if (plsLaunchResult.success && plsLaunchResult.retMessageQueue.has_value())
    {
        curSharedMsgQueues.plsRetQueue =
            plsLaunchResult.retMessageQueue.value();
        LOG_DEBUG("PLS retMessageQueue set up");

        auto plsQueueHandler =
            std::make_shared<AikariLauncherComponents::SubModuleSystem::
                                 ThreadMsgHandlers::PLSMsgHandler>(
                curSharedMsgQueues.plsRetQueue.get(),
                curSharedMsgQueues.plsInputQueue.get(),
                "PLS"
            );

        auto& handlersMgr =
            AikariLifecycle::AikariSharedHandlers::getInstance();
        handlersMgr.setVal(
            &lifecycleTypes::GlobalSharedHandlersRegistry::
                plsIncomingMsgQueueHandler,
            plsQueueHandler
        );
    }

    lifecycleStates.setVal(
        &lifecycleTypes::GlobalLifecycleStates::sharedMsgQueue,
        curSharedMsgQueues
    );

    // --- To Be Done --- //

    LOG_INFO("Aikari is loaded, waiting for further operations...");
    aikariAliveFuture.get();  // Run forever - until sig recv

    LOG_INFO("Stopping ws server...");
    wsServerMgrPtr->stopWssServer();
    ix::uninitNetSystem();

    LOG_INFO("Cleaning up sub modules...");
    if (plsLaunchResult.success)
    {
        AikariPLS::Exports::onExit();

        auto& threadsMgr = AikariLifecycle::AikariSharedHandlers::getInstance();
        if (auto plsQueueHandler = threadsMgr.getVal(
                &lifecycleTypes::GlobalSharedHandlersRegistry::
                    plsIncomingMsgQueueHandler
            ))
        {
            plsQueueHandler->manualDestroy();
        }

        if (plsLaunchResult.plsRuntimeThread->joinable())
        {
            plsLaunchResult.plsRuntimeThread->join();
        }

        LOG_INFO("Clean up for module PLS finished.");
    }

    LOG_INFO("Unloading shared instances...");
    sharedInstances.resetPtr(&lifecycleTypes::SharedInstances::configManagerIns
    );
    sharedInstances.resetPtr(
        &lifecycleTypes::SharedInstances::registryManagerIns
    );
    sharedInstances.resetPtr(&lifecycleTypes::SharedInstances::wsServerMgrIns);

    LOG_INFO("👋 Clean up completed, goodbye.");
    return 0;
}

int main(int argc, const char* argv[])
{
    AikariShared::LoggerSystem::initLogger(
        "Main", 30, 47
    );  // 30 = Black text; 47 = White background
    auto cliOptions = AikariCliUtils::constructCliOptions();
    auto parseRet = AikariCliUtils::parseCliOptions(cliOptions, argc, argv);

    LOG_INFO("❇️ Welcome to HugoAura-Aikari");
    LOG_INFO("Launching as CLI mode");
    logVersion();
    LOG_INFO(
        "Argv: [isDebug={}, serviceCtrl={}]",
        parseRet.isDebug,
        parseRet.serviceCtrl
    );

    switch (parseRet.isDebug)
    {
        case false:
            DEFAULT_LOGGER->set_level(spdlog::level::info);
            break;
        case true:
            DEFAULT_LOGGER->set_level(spdlog::level::trace);
            break;
    }

    lifecycleTypes::APPLICATION_RUNTIME_MODES curRuntimeMode =
        parseRet.isDebug ? lifecycleTypes::APPLICATION_RUNTIME_MODES::DEBUG
                         : lifecycleTypes::APPLICATION_RUNTIME_MODES::NORMAL;

    if (parseRet.serviceCtrl == "install")
    {
        auto winSvcMgrIns = std::make_unique<AikariWinSvc::WinSvcManager>();
        if (winSvcMgrIns->isServiceExists)
        {
            LOG_WARN("Aikari Svc already installed, skipping installation.");
            return 0;
        }

        bool instResult = winSvcMgrIns->installService();
        return instResult ? 0 : -1;
    }
    else if (parseRet.serviceCtrl == "uninstall")
    {
        auto winSvcMgrIns = std::make_unique<AikariWinSvc::WinSvcManager>();
        if (!winSvcMgrIns->isServiceExists)
        {
            LOG_WARN("Aikari Svc not installed, cannot uninstall.");
            return 0;
        }

        bool uninstResult = winSvcMgrIns->uninstallService();
        return uninstResult ? 0 : -1;
    }

    signal(SIGINT, exitSignalHandler);

    return launchAikari(curRuntimeMode);
}
