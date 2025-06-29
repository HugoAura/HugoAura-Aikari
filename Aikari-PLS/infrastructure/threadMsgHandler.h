#pragma once

#include <Aikari-PLS/types/infrastructure/messageQueue.h>
#include <Aikari-Shared/infrastructure/SinglePointMessageQueue.hpp>
#include <memory>
#include <string>

namespace AikariPLS::Infrastructure::MsgQueue
{
class PLSThreadMsgQueueHandler
{
   public:
    PLSThreadMsgQueueHandler(
        std::shared_ptr<
            AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariPLS::Types::infrastructure::InputMessageStruct>> srcQueue,
        std::shared_ptr<
            AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariPLS::Types::infrastructure::RetMessageStruct>> destQueue
    );

    void manualDestroy();

   private:
    std::shared_ptr<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariPLS::Types::infrastructure::InputMessageStruct>>
        srcQueue;
    std::shared_ptr<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariPLS::Types::infrastructure::RetMessageStruct>>
        destQueue;

    std::shared_ptr<std::jthread> inputMsgWorkerThread;

    void inputMsgWorker();
};
}  // namespace AikariPLS::Infrastructure::MsgQueue
