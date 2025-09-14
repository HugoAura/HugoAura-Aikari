#pragma once

#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>
#include <memory>
#include <optional>

namespace AikariShared::Types::InterThread
{
    struct SubToMainMessageInstance;
}  // namespace AikariShared::Types::InterThread

namespace AikariPLS::Types::Entrypoint
{
    struct EntrypointRet
    {
        bool success;
        std::optional<std::shared_ptr<
            AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::SubToMainMessageInstance>>>
            retMessageQueue;

        std::unique_ptr<std::jthread> plsRuntimeThread;
    };
}  // namespace AikariPLS::Types::Entrypoint