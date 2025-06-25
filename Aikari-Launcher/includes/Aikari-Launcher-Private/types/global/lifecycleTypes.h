#pragma once

#include <windows.h>

#include <atomic>
#include <memory>

namespace AikariRegistry
{
class RegistryManager;
}

namespace AikariLauncherComponents::AikariWebSocketServer
{
class MainWSServer;
}

namespace AikariLauncherComponents::AikariConfig
{
class LauncherConfigManager;
}

namespace AikariTypes::global::lifecycle
{
enum class APPLICATION_RUNTIME_MODES
{
    NORMAL,
    DEBUG,
    SERVICE,
};

struct SharedInstances
{
    std::shared_ptr<AikariRegistry::RegistryManager> registryManagerIns;
    std::shared_ptr<
        AikariLauncherComponents::AikariWebSocketServer::MainWSServer>
        wsServerMgrIns;
    std::shared_ptr<
        AikariLauncherComponents::AikariConfig::LauncherConfigManager>
        configManagerIns;
};

struct GlobalLifecycleStates
{
    APPLICATION_RUNTIME_MODES runtimeMode;
    long long launchTime;
    SERVICE_STATUS svcStatus;
    SERVICE_STATUS_HANDLE svcStatusHandle;
    std::atomic<bool> svcIsRunning;
    SharedInstances sharedIns;

    static GlobalLifecycleStates createDefault()
    {
        return { .runtimeMode = APPLICATION_RUNTIME_MODES::NORMAL,
                 .launchTime = 0,
                 .svcStatus = { 0 },
                 .svcStatusHandle = NULL,
                 .svcIsRunning = false };
    }
};
}  // namespace AikariTypes::global::lifecycle
