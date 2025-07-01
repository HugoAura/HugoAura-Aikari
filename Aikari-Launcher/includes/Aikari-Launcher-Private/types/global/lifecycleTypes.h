#pragma once

#include <Aikari-PLS/types/infrastructure/messageQueue.h>
#include <atomic>
#include <memory>
#include <windows.h>

namespace cppcoro
{
class static_thread_pool;
class io_service;
}  // namespace cppcoro

namespace AikariRegistry
{
class RegistryManager;
}

namespace AikariLauncherComponents
{
namespace AikariWebSocketServer
{
class MainWSServer;
};

namespace AikariConfig
{
class LauncherConfigManager;
};
}  // namespace AikariLauncherComponents

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

struct GlobalSharedThreadsRegistry
{
    std::shared_ptr<std::jthread> plsIncomingMsgQueueHandlerThread;

    static GlobalSharedThreadsRegistry createDefault()
    {
        return {};
    }
};

struct SharedInstances
{
    std::unique_ptr<AikariRegistry::RegistryManager> registryManagerIns;
    std::unique_ptr<
        AikariLauncherComponents::AikariWebSocketServer::MainWSServer>
        wsServerMgrIns;
    std::unique_ptr<
        AikariLauncherComponents::AikariConfig::LauncherConfigManager>
        configManagerIns;

    SharedInstances();

    static SharedInstances createDefault();

    ~SharedInstances();

    SharedInstances(SharedInstances&&) noexcept;
    SharedInstances& operator=(SharedInstances&&) noexcept;
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
