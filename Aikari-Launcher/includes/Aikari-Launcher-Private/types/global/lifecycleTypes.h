#pragma once

#include <Aikari-PLS/types/infrastructure/messageQueue.h>
#include <atomic>
#include <memory>
#include <windows.h>

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

namespace AikariShared::infrastructure::MessageQueue
{
template <typename T>
class SinglePointMessageQueue;
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

struct SharedMessageQueues
{
    std::shared_ptr<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariPLS::Types::infrastructure::InputMessageStruct>>
        plsInputQueue;

    std::shared_ptr<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariPLS::Types::infrastructure::RetMessageStruct>>
        plsRetQueue;
};

struct GlobalLifecycleStates
{
    APPLICATION_RUNTIME_MODES runtimeMode;
    long long launchTime;
    SERVICE_STATUS svcStatus;
    SERVICE_STATUS_HANDLE svcStatusHandle;
    std::atomic<bool> svcIsRunning;
    SharedInstances sharedIns;
    SharedMessageQueues sharedMsgQueue;

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
