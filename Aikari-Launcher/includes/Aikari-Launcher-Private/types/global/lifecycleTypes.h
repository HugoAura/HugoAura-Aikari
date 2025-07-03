#pragma once

#include <Aikari-Shared/types/itc/shared.h>
#include <atomic>
#include <memory>
#include <windows.h>

namespace AikariRegistry
{
    class RegistryManager;
}

namespace AikariFileSystem
{
    class FileSystemManager;
}

namespace AikariLauncherComponents
{
    namespace AikariWebSocketServer
    {
        class MainWSServer;
    };

    namespace SubModuleSystem::ThreadMsgHandlers
    {
        class PLSMsgHandler;
    }

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

    struct GlobalSharedHandlersRegistry
    {
        std::shared_ptr<AikariLauncherComponents::SubModuleSystem::
                            ThreadMsgHandlers::PLSMsgHandler>
            plsIncomingMsgQueueHandler;

        static GlobalSharedHandlersRegistry createDefault()
        {
            return {};
        }
    };

    struct SharedInstances
    {
        std::unique_ptr<AikariRegistry::RegistryManager> registryManagerIns;
        std::unique_ptr<AikariFileSystem::FileSystemManager> fsManagerIns;
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
                AikariShared::Types::InterThread::MainToSubMessageInstance>>
            plsInputQueue;

        std::shared_ptr<
            AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::SubToMainMessageInstance>>
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
