#pragma once

#include <Aikari-PLS/types/infrastructure/messageQueue.h>
#include <Aikari-Shared/infrastructure/SinglePointMessageQueue.hpp>
#include <memory>
#include <optional>

namespace AikariPLS::Types::entrypoint
{
struct EntrypointRet
{
    bool success;
    std::optional<std::shared_ptr<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariPLS::Types::infrastructure::RetMessageStruct>>>
        retMessageQueue;
};
}  // namespace AikariPLS::Types::entrypoint