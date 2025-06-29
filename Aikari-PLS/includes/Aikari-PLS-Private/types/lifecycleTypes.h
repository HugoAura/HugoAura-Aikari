#pragma once

#include <memory>

namespace AikariPLS::Infrastructure::MsgQueue
{
class PLSThreadMsgQueueHandler;
}

namespace AikariPLS::Types::lifecycle
{
struct PLSSharedIns
{
    std::shared_ptr<
        AikariPLS::Infrastructure::MsgQueue::PLSThreadMsgQueueHandler>
        threadMsgQueueHandler;

    static PLSSharedIns createDefault()
    {
        return {};
    }
};
}  // namespace AikariPLS::Types::lifecycle
