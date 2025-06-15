#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Launcher-Private/types/global/lifecycleTypes.h>

#include <chrono>
#include <cxxopts.hpp>

#include "infrastructure/cliParse.h"
#include "infrastructure/fileSystem.h"
#include "infrastructure/logger.h"
#include "infrastructure/registry.h"
#include "lifecycle.h"

namespace lifecycleTypes = AikariTypes::global::lifecycle;

int main(int argc, const char* argv[])
{
    AikariLoggerSystem::initLogger();
    auto cliOptions = AikariCliUtils::constructCliOptions();
    auto parseRet = AikariCliUtils::parseCliOptions(cliOptions, argc, argv);

    LOG_INFO("❇️ Welcome to HugoAura-Aikari");
    LOG_INFO("Version: " + AikariDefaults::Version + std::format(" ({})", AikariDefaults::VersionCode));
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
    auto curTime =
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    auto& lifecycleStates = AikariLifecycle::AikariStatesManager::getInstance();
    LOG_INFO("Initializing states manager...");
    lifecycleStates.setVal(&lifecycleTypes::GlobalLifecycleStates::runtimeMode, curRuntimeMode);
    lifecycleStates.setVal(&lifecycleTypes::GlobalLifecycleStates::launchTime, curTime);

    LOG_INFO("Initializing file system manager...");
    auto fileSystemManagerIns = std::make_shared<AikariFileSystem::FileSystemManager>();
    fileSystemManagerIns->ensureDirExists();

    /*
    LOG_TRACE(std::format("Current lifecycleStates: [launchTime={}, runtimeMode={}]",
                          lifecycleStates.getVal<long long>(&lifecycleTypes::GlobalLifecycleStates::launchTime),
                          static_cast<int>(lifecycleStates.getVal<lifecycleTypes::APPLICATION_RUNTIME_MODES>(
                              &lifecycleTypes::GlobalLifecycleStates::runtimeMode))));
    */

    LOG_INFO("Initializing registry manager...");
    auto registryManagerIns = std::make_shared<AikariRegistry::RegistryManager>();
    registryManagerIns->ensureRegKeyExists();
}
