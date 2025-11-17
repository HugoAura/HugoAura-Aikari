#define CUSTOM_LOG_HEADER "[Lifecycle Controller]"

#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Launcher-Private/types/constants/entrypoint.h>
#include <Aikari-Launcher-Private/types/global/lifecycleTypes.h>
#include <Aikari-Launcher-Public/constants/lifecycle.h>
#include <Aikari-Launcher-Public/version.h>
#include <Aikari-PLS/Aikari-PLS-Exports.h>
#include <Aikari-PLS/types/entrypoint.h>
#include <Aikari-Shared/infrastructure/logger.h>
#include <chrono>
#include <csignal>
#include <cxxopts.hpp>
#include <future>
#include <ixwebsocket/IXNetSystem.h>
#include <shlobj_core.h>
#include <windows.h>

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
#include "virtual/lifecycle/ILaunchProgressReporter.h"
#include "winSvcHandler.h"

namespace Aikari::EternalCore
{
    namespace lifecycleTypes = AikariTypes::Global::Lifecycle;
    namespace entrypointConstants = AikariTypes::Constants::Entrypoint;

    std::promise<bool> aikariAlivePromise;

    static void exitSignalHandler(int signum)
    {
        CUSTOM_LOG_INFO(
            "Received {} signal, exiting Aikari...", std::to_string(signum)
        );
        aikariAlivePromise.set_value(true);
    }

    int launchAikari(
        const AikariLauncher::Public::Constants::Lifecycle::
            APPLICATION_RUNTIME_MODES& runtimeMode,
        std::optional<std::unique_ptr<
            AikariLauncher::Virtual::Lifecycle::IProgressReporter>>
            launchProgressReporter,
        std::optional<HANDLE> winSvcStopEvent
    )
    {
        AikariLauncher::Virtual::Lifecycle::GenericReportFn reportProgress =
            [&launchProgressReporter](
                bool isFailed,
                bool isCompleted,
                bool isStarting,
                unsigned int waitForMs,
                unsigned int percent,
                unsigned int exitCode
            )
        {
            // Maybe report to Aikari Frontend (in the future)
            if (launchProgressReporter != std::nullopt)
            {
                launchProgressReporter.value()->reportLaunchProgress(
                    isFailed,
                    isCompleted,
                    isStarting,
                    waitForMs,
                    percent,
                    exitCode
                );
            }
        };

        std::future<bool> aikariAliveFuture = aikariAlivePromise.get_future();

        reportProgress(false, false, true, 100, 0, 0);

        auto curTime = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch()
        )
                           .count();

        HINSTANCE selfHIns = GetModuleHandleW(NULL);
        if (selfHIns == NULL)
        {
            LOG_CRITICAL(
                "--- DO NOT REPORT THIS TO HUGOAURA, THIS IS NOT A BUG OR A "
                "CRASH "
                "---"
            );  // paper mc 🤣
            LOG_CRITICAL(
                "Failed to get the handle of main process, exiting..."
            );
            LOG_CRITICAL("Please check your system environment.");
            reportProgress(
                true,
                false,
                true,
                0,
                0,
                entrypointConstants::EXIT_CODES::HINS_GET_FAILED
            );
            return entrypointConstants::EXIT_CODES::HINS_GET_FAILED;
        }

        HANDLE hMutex = CreateMutexA(NULL, FALSE, AikariDefaults::mutexName);
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            CloseHandle(hMutex);
            CUSTOM_LOG_CRITICAL(
                "Another Aikari instance is running, close the existing one to "
                "continue."
            );
            reportProgress(
                true,
                false,
                true,
                0,
                0,
                entrypointConstants::EXIT_CODES::MULTI_AIKARI_INSTANCE_DETECTED
            );
            return entrypointConstants::EXIT_CODES::
                MULTI_AIKARI_INSTANCE_DETECTED;
        }

        auto& lifecycleStates =
            AikariLifecycle::AikariStatesManager::getInstance();
        CUSTOM_LOG_INFO("Initializing states manager...");
        lifecycleStates.setVal(
            &lifecycleTypes::GlobalLifecycleStates::runtimeMode, runtimeMode
        );
        lifecycleStates.setVal(
            &lifecycleTypes::GlobalLifecycleStates::launchTime, curTime
        );
        reportProgress(false, false, true, 1000, 25, 0);

        auto& sharedInstances =
            AikariLifecycle::AikariSharedInstances::getInstance();

        CUSTOM_LOG_INFO("Initializing file system manager...");
        {
            auto fileSystemManagerIns =
                std::make_unique<AikariFileSystem::FileSystemManager>();
            bool fsInitResult = fileSystemManagerIns->ensureDirExists();
            if (fsInitResult == false)
            {
                CUSTOM_LOG_CRITICAL(
                    "Aikari directories initialization failed, exiting "
                    "Aikari..."
                );
                reportProgress(
                    true,
                    false,
                    true,
                    0,
                    25,
                    entrypointConstants::EXIT_CODES::FS_INIT_FAILED
                );
                return entrypointConstants::EXIT_CODES::FS_INIT_FAILED;
            }
            sharedInstances.setPtr(
                &lifecycleTypes::SharedInstances::fsManagerIns,
                std::move(fileSystemManagerIns)
            );
        }
        auto* fileSystemManagerIns = sharedInstances.getPtr(
            &lifecycleTypes::SharedInstances::fsManagerIns
        );
        reportProgress(false, false, true, 1000, 35, 0);

        CUSTOM_LOG_INFO("Initializing registry manager...");
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
            CUSTOM_LOG_CRITICAL(
                "Registry initialization failed, exiting Aikari..."
            );
            reportProgress(
                true,
                false,
                true,
                0,
                35,
                entrypointConstants::EXIT_CODES::REG_INIT_FAILED
            );
            return entrypointConstants::EXIT_CODES::REG_INIT_FAILED;
        }
        reportProgress(false, false, true, 1000, 40, 0);

        CUSTOM_LOG_INFO("Initializing config manager...");
        {
            auto configManagerIns =
                std::make_unique<AikariLauncher::Components::AikariConfig::
                                     LauncherConfigManager>(
                    "launcher",
                    fileSystemManagerIns->aikariConfigDir / "config.json",
                    IDR_AIKARI_DEFAULT_JSON,
                    selfHIns
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
            CUSTOM_LOG_CRITICAL(
                "Config initialization failed, exiting Aikari..."
            );
            reportProgress(
                true,
                false,
                true,
                0,
                40,
                entrypointConstants::EXIT_CODES::CONFIG_INIT_FAILED
            );
            return entrypointConstants::EXIT_CODES::CONFIG_INIT_FAILED;
        }

        auto curConfigPtr = std::atomic_load(&configManagerPtr->config);
        reportProgress(false, false, true, 1250, 45, 0);

        CUSTOM_LOG_INFO("Initializing TLS certificates...");
        std::filesystem::path certDir =
            fileSystemManagerIns->aikariConfigDir / "certs";
        bool wsCertInitResult = AikariUtils::SSLUtils::initWsCert(
            certDir, curConfigPtr->tls.regenWsCertNextLaunch
        );
        if (!wsCertInitResult)
        {
            CUSTOM_LOG_CRITICAL(
                "Failed to initialize WebSocket TLS cert, exiting Aikari..."
            );
            reportProgress(
                true,
                false,
                true,
                0,
                45,
                entrypointConstants::EXIT_CODES::NETWORK_SERVICES_INIT_FAILED
            );
            return entrypointConstants::EXIT_CODES::
                NETWORK_SERVICES_INIT_FAILED;
        }
        reportProgress(false, false, true, 2000, 50, 0);

        CUSTOM_LOG_INFO("Initializing Windows socket environment...");
        ix::initNetSystem();
        CUSTOM_LOG_INFO("Starting Aikari WebSocket server...");
        {
            int wsDefaultPort = curConfigPtr->wsPreferPort;
            auto wsServerManagerIns =
                std::make_unique<AikariLauncher::Components::
                                     AikariWebSocketServer::MainWSServer>(
                    "127.0.0.1",
                    wsDefaultPort,
                    certDir / "wss.crt",
                    certDir / "wss.key"
                );
            sharedInstances.setPtr(
                &lifecycleTypes::SharedInstances::wsServerMgrIns,
                std::move(wsServerManagerIns)
            );
        }
        auto* wsServerMgrPtr = sharedInstances.getPtr(
            &lifecycleTypes::SharedInstances::wsServerMgrIns
        );
        bool wsLaunchResult = wsServerMgrPtr->tryLaunchWssServer();
        if (!wsLaunchResult)
        {
            CUSTOM_LOG_CRITICAL(
                "Failed to launch Aikari ws server, exiting Aikari..."
            );
            reportProgress(
                true,
                false,
                true,
                0,
                50,
                entrypointConstants::EXIT_CODES::NETWORK_SERVICES_INIT_FAILED
            );
            return entrypointConstants::EXIT_CODES::
                NETWORK_SERVICES_INIT_FAILED;
        }
        reportProgress(false, false, true, 1000, 60, 0);

        auto& curSharedMsgQueues = lifecycleStates.getVal(
            &lifecycleTypes::GlobalLifecycleStates::sharedMsgQueue
        );

        auto plsInputMsgQueue = std::make_shared<
            AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::MainToSubMessageInstance>>();

        curSharedMsgQueues.plsInputQueue = plsInputMsgQueue;

        AikariPLS::Types::Entrypoint::EntrypointRet plsLaunchResult =
            AikariPLS::Exports::main(
                fileSystemManagerIns->aikariRootDir,
                certDir,
                plsInputMsgQueue,
                &AikariShared::LoggerSystem::defaultLoggerSink
            );

        if (plsLaunchResult.success &&
            plsLaunchResult.retMessageQueue.has_value())
        {
            curSharedMsgQueues.plsRetQueue =
                plsLaunchResult.retMessageQueue.value();
            CUSTOM_LOG_DEBUG("<PLS> retMessageQueue set up");

            auto plsQueueHandler =
                std::make_shared<AikariLauncher::Components::SubModuleSystem::
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
        reportProgress(false, false, true, 100, 90, 0);

        // --- To Be Done --- //

        curConfigPtr.reset();

        reportProgress(false, true, true, 0, 100, 0);
        CUSTOM_LOG_INFO("Aikari is loaded, waiting for further operations...");
        if (runtimeMode == AikariLauncher::Public::Constants::Lifecycle::
                               APPLICATION_RUNTIME_MODES::NORMAL ||
            runtimeMode == AikariLauncher::Public::Constants::Lifecycle::
                               APPLICATION_RUNTIME_MODES::DEBUG)
        {
            aikariAliveFuture.get();  // Run forever - until sig recv
        }
        else if (runtimeMode == AikariLauncher::Public::Constants::Lifecycle::
                                    APPLICATION_RUNTIME_MODES::SERVICE)
        {
            WaitForSingleObject(winSvcStopEvent.value(), INFINITE);
        }
        reportProgress(false, false, false, 1750, 0, 0);

        CUSTOM_LOG_INFO("Stopping ws server...");
        wsServerMgrPtr->stopWssServer();
        ix::uninitNetSystem();
        reportProgress(false, false, false, 4000, 30, 0);

        CUSTOM_LOG_INFO("Cleaning up sub modules...");
        if (plsLaunchResult.success)
        {
            AikariPLS::Exports::onExit();

            auto& threadsMgr =
                AikariLifecycle::AikariSharedHandlers::getInstance();
            if (auto& plsQueueHandler = threadsMgr.getVal(
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

            CUSTOM_LOG_INFO("Clean up for module PLS finished.");
        }
        reportProgress(false, false, false, 2000, 50, 0);

        CUSTOM_LOG_INFO("Unloading shared instances...");
        sharedInstances.resetPtr(
            &lifecycleTypes::SharedInstances::configManagerIns
        );
        sharedInstances.resetPtr(
            &lifecycleTypes::SharedInstances::registryManagerIns
        );
        sharedInstances.resetPtr(
            &lifecycleTypes::SharedInstances::wsServerMgrIns
        );
        reportProgress(false, false, false, 100, 80, 0);

        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        CUSTOM_LOG_INFO("👋 Clean up completed, goodbye.");
        reportProgress(
            false,
            true,
            false,
            0,
            100,
            entrypointConstants::EXIT_CODES::NORMAL_EXIT
        );
        CUSTOM_LOG_DEBUG("↓ Auto deconstruction begins.");
        return entrypointConstants::EXIT_CODES::NORMAL_EXIT;
    }

    namespace Utility
    {
        static void setupLoggerMode(
            const AikariTypes::Infrastructure::CLIParse::CLIOptionsRet&
                cliParseRet
        )
        {
            if (cliParseRet.logMode == "ttyAndFile")
            {
                AikariShared::LoggerSystem::defaultLoggerSink.emplace(
                    AikariShared::LoggerSystem::LOGGER_SINK::FILE
                );
                AikariShared::LoggerSystem::defaultLoggerSink.emplace(
                    AikariShared::LoggerSystem::LOGGER_SINK::CONSOLE
                );
            }
            else if (cliParseRet.logMode == "file")
            {
                AikariShared::LoggerSystem::defaultLoggerSink.emplace(
                    AikariShared::LoggerSystem::LOGGER_SINK::FILE
                );
            }
            else
            {
                AikariShared::LoggerSystem::defaultLoggerSink.emplace(
                    AikariShared::LoggerSystem::LOGGER_SINK::CONSOLE
                );
            }
        }
    }  // namespace Utility
}  // namespace Aikari::EternalCore

namespace Aikari::AsWindowsServices
{
    class AikariWinSvcLifecycleController
        : public AikariLauncher::Core::WinSCM::IWinSvcHandler
    {
       public:
        explicit AikariWinSvcLifecycleController(
            const std::wstring& serviceName
        )
            : AikariLauncher::Core::WinSCM::IWinSvcHandler(serviceName) {};

       protected:
        void onServiceStart(DWORD dwordArgc, LPWSTR* lpszArgv) override
        {
            std::function<void(
                DWORD dwordCurState,
                DWORD dwordWin32ExitCode,
                DWORD dwordWaitHint
            )>
                reportFn = [this](
                               DWORD dwordCurState,
                               DWORD dwordWin32ExitCode,
                               DWORD dwordWaitHint
                           )
            {
                this->reportServiceStatus(
                    dwordCurState, dwordWin32ExitCode, dwordWaitHint
                );
            };

            auto progressReporter = std::make_unique<
                AikariLauncher::Core::WinSCM::WinSvcLaunchProgressReporter>(
                reportFn
            );

            Aikari::EternalCore::launchAikari(
                AikariLauncher::Public::Constants::Lifecycle::
                    APPLICATION_RUNTIME_MODES::SERVICE,
                std::move(progressReporter),
                this->hServiceStopEvent
            );
        };
    };
}  // namespace Aikari::AsWindowsServices

int main(int argc, const char* argv[])
{
    auto parseRet = AikariCliUtils::parseCliOptions(argc, argv);
    if (parseRet.exitNow)
        return 0;
    bool isRunAsSvc = (parseRet.serviceCtrl == "runAs");
    if (isRunAsSvc)
    {
        AikariShared::LoggerSystem::defaultLoggerSink.emplace(
            AikariShared::LoggerSystem::LOGGER_SINK::FILE
        );
    }
    else
    {
        Aikari::EternalCore::Utility::setupLoggerMode(parseRet);
    }
    AikariShared::LoggerSystem::initLogger(
        "Main", 30, 47
    );  // 30 = Black text; 47 = White background
    if (isRunAsSvc)
    {
        CUSTOM_LOG_INFO("❇️ Welcome to HugoAura-Aikari");
        CUSTOM_LOG_INFO("⌛ Launching as Service mode");
        Aikari::AsWindowsServices::AikariWinSvcLifecycleController
            winSvcHandler(AikariDefaults::ServiceConfig::ServiceName);
        return winSvcHandler.startAsService();
    }

    CUSTOM_LOG_INFO("❇️ Welcome to HugoAura-Aikari");
    CUSTOM_LOG_INFO("🧱 Launching as CLI mode");
    CUSTOM_LOG_INFO(
        "⚛️ Version: {} ({})",
        AikariLauncher::Public::Version::Version,
        AikariLauncher::Public::Version::VersionCode
    );
    CUSTOM_LOG_INFO(
        "🎲 Argv: [isDebug={}, serviceCtrl={}]",
        parseRet.isDebug,
        parseRet.serviceCtrl
    );

    if (!parseRet.isDebug)
    {
        DEFAULT_LOGGER->set_level(spdlog::level::info);
    }
    else
    {
        DEFAULT_LOGGER->set_level(spdlog::level::trace);
    }

    // TODO: Service
    AikariLauncher::Public::Constants::Lifecycle::APPLICATION_RUNTIME_MODES
        curRuntimeMode =
            parseRet.isDebug ? AikariLauncher::Public::Constants::Lifecycle::
                                   APPLICATION_RUNTIME_MODES::DEBUG
                             : AikariLauncher::Public::Constants::Lifecycle::
                                   APPLICATION_RUNTIME_MODES::NORMAL;

    if (parseRet.serviceCtrl == "install")
    {
        auto winSvcMgrIns = std::make_unique<AikariWinSvc::WinSvcManager>();
        if (winSvcMgrIns->isServiceExists)
        {
            CUSTOM_LOG_WARN(
                "Aikari Svc already installed, skipping installation."
            );
            return AikariTypes::Constants::Entrypoint::EXIT_CODES::NORMAL_EXIT;
        }

        bool instResult = winSvcMgrIns->installService();
        return instResult
                   ? AikariTypes::Constants::Entrypoint::EXIT_CODES::NORMAL_EXIT
                   : AikariTypes::Constants::Entrypoint::EXIT_CODES::
                         SERVICE_ACTION_FAILED;
    }
    else if (parseRet.serviceCtrl == "uninstall")
    {
        auto winSvcMgrIns = std::make_unique<AikariWinSvc::WinSvcManager>();
        if (!winSvcMgrIns->isServiceExists)
        {
            CUSTOM_LOG_WARN("Aikari Svc not installed, cannot uninstall.");
            return AikariTypes::Constants::Entrypoint::EXIT_CODES::NORMAL_EXIT;
        }

        bool uninstResult = winSvcMgrIns->uninstallService();
        return uninstResult
                   ? AikariTypes::Constants::Entrypoint::EXIT_CODES::NORMAL_EXIT
                   : AikariTypes::Constants::Entrypoint::EXIT_CODES::
                         SERVICE_ACTION_FAILED;
    }

    signal(SIGINT, Aikari::EternalCore::exitSignalHandler);

    return Aikari::EternalCore::launchAikari(
        curRuntimeMode, std::nullopt, std::nullopt
    );
}
