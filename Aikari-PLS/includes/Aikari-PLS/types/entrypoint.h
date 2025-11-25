#pragma once

#include <Aikari-PLS/types/infrastructure/init.h>
#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>
#include <future>
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

        std::unique_ptr<std::jthread> plsInitThread;
        std::future<AikariPLS::Types::Infrastructure::Init::PLSInitResult>
            plsInitResultFuture;
    };
}  // namespace AikariPLS::Types::Entrypoint