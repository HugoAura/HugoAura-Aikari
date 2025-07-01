#pragma once

#include <memory>

namespace AikariShared::infrastructure::MessageQueue
{
template <typename T>
class SinglePointMessageQueue;
}

namespace AikariPLS::Types::infrastructure
{
struct InputMessageStruct;
struct RetMessageStruct;
}  // namespace AikariPLS::Types::infrastructure

namespace AikariPLS::Infrastructure::MsgQueue
{
class PLSThreadMsgQueueHandler;
}

namespace AikariPLS::Types::lifecycle
{
struct PLSSharedIns
{
    std::unique_ptr<
        AikariPLS::Infrastructure::MsgQueue::PLSThreadMsgQueueHandler>
        threadMsgQueueHandler;

    PLSSharedIns();
    static PLSSharedIns createDefault();
    ~PLSSharedIns();

    PLSSharedIns(PLSSharedIns&&) noexcept;
    PLSSharedIns& operator=(PLSSharedIns&&) noexcept;
};

struct PLSSharedMsgQueues
{
    std::shared_ptr<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariPLS::Types::infrastructure::InputMessageStruct>>
        inputMsgQueue;

    std::shared_ptr<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariPLS::Types::infrastructure::RetMessageStruct>>
        retMsgQueue;

    static PLSSharedMsgQueues createDefault()
    {
        return {};
    }
};
}  // namespace AikariPLS::Types::lifecycle
