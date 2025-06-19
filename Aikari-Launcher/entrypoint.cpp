#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Launcher-Private/types/global/lifecycleTypes.h>

#include <chrono>
#include <cxxopts.hpp>

#include "infrastructure/cliParse.h"
#include "infrastructure/fileSystem.h"
#include "infrastructure/logger.h"
#include "infrastructure/registry.h"
#include "infrastructure/winSvc.h"
#include "lifecycle.h"
#include "utils/sslUtils.h"

namespace lifecycleTypes = AikariTypes::global::lifecycle;

static void logVersion()
{
    LOG_INFO("Version: " + AikariDefaults::Version + std::format(" ({})", AikariDefaults::VersionCode));
}

int launchAikari(lifecycleTypes::APPLICATION_RUNTIME_MODES& runtimeMode)
{
    auto curTime =
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    auto& lifecycleStates = AikariLifecycle::AikariStatesManager::getInstance();
    LOG_INFO("Initializing states manager...");
    lifecycleStates.setVal(&lifecycleTypes::GlobalLifecycleStates::runtimeMode, runtimeMode);
    lifecycleStates.setVal(&lifecycleTypes::GlobalLifecycleStates::launchTime, curTime);

    LOG_INFO("Initializing file system manager...");
    auto fileSystemManagerIns = std::make_shared<AikariFileSystem::FileSystemManager>();
    fileSystemManagerIns->ensureDirExists();

    LOG_INFO("Initializing registry manager...");
    auto registryManagerIns = std::make_shared<AikariRegistry::RegistryManager>();
    int regKeyValidateResult = registryManagerIns->ensureRegKeyExists();
    if (regKeyValidateResult == -1)
    {
        LOG_CRITICAL("Registry initialization failed, exiting Aikari...");
        return -1;
    }

    LOG_INFO("Initializing TLS certificates...");
    std::filesystem::path certDir = fileSystemManagerIns->aikariConfigDir / "certs";
    bool wsCertInitResult = AikariUtils::sslUtils::initWsCert(certDir);
    if (!wsCertInitResult)
    {
        LOG_CRITICAL("Failed to initialize WebSocket TLS cert, exiting Aikari...");
        return -1;
    }
}

int main(int argc, const char* argv[])
{
    AikariLoggerSystem::initLogger();
    auto cliOptions = AikariCliUtils::constructCliOptions();
    auto parseRet = AikariCliUtils::parseCliOptions(cliOptions, argc, argv);

    LOG_INFO("❇️ Welcome to HugoAura-Aikari");
    LOG_INFO("Launching as CLI mode");
    logVersion();
    LOG_INFO(std::format("Argv: [isDebug={}, serviceCtrl={}]", parseRet.isDebug, parseRet.serviceCtrl));

    switch (parseRet.isDebug)
    {
        case false:
            DEFAULT_LOGGER->set_level(spdlog::level::info);
            break;
        case true:
            DEFAULT_LOGGER->set_level(spdlog::level::trace);
            break;
    }

    lifecycleTypes::APPLICATION_RUNTIME_MODES curRuntimeMode = parseRet.isDebug
                                                                   ? lifecycleTypes::APPLICATION_RUNTIME_MODES::DEBUG
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

    return launchAikari(curRuntimeMode);
}
