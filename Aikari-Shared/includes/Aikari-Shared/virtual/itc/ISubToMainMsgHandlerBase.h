#pragma once

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/infrastructure/queue/PoolQueue.hpp>
#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>
#include <Aikari-Shared/types/itc/shared.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace AikariShared::Infrastructure::InterThread
{
    const int DEFAULT_THREAD_COUNT = 4;

    class ISubToMainMsgHandler
    {
       public:
        virtual ~ISubToMainMsgHandler() = default;

       private:
        virtual void retMsgWorker() = 0;
    };

    class SubToMainMsgHandlerBase : public ISubToMainMsgHandler
    {
       public:
        SubToMainMsgHandlerBase(
            AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::SubToMainMessageInstance>*
                srcQueue,
            AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::MainToSubMessageInstance>*
                reportQueue,
            const std::string subModuleName
        );

        void manualDestroy();

        void addCtrlMsgCallbackListener(
            const AikariShared::Types::InterThread::eventId& eventId,
            std::move_only_function<void(
                AikariShared::Types::InterThread::SubToMainControlReplyMessage
            )> callbackLambda
        );

       protected:
        const std::string logHeader;
        std::deque<AikariShared::Types::InterThread::SubToMainMessageInstance>
            msgTempStore;

        AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariShared::Types::InterThread::SubToMainMessageInstance>*
            srcQueue;
        AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariShared::Types::InterThread::MainToSubMessageInstance>*
            reportQueue;

        std::map<
            AikariShared::Types::InterThread::eventId,
            std::vector<std::move_only_function<void(
                AikariShared::Types::InterThread::SubToMainControlReplyMessage
            )>>>
            listeners;

        std::unique_ptr<AikariShared::Infrastructure::MessageQueue::PoolQueue<
            AikariShared::Types::InterThread::SubToMainMessageInstance>>
            threadPool;
        std::unique_ptr<std::jthread> srcMsgWorkerThread;

        virtual void onControlMessage(
            AikariShared::Types::InterThread::SubToMainControlMessage& retMsg
        ) {};
        virtual void onControlReply(
            const AikariShared::Types::InterThread::
                SubToMainControlReplyMessage& retMsg
        ) {};
        virtual void onWebSocketMessage(
            AikariShared::Types::InterThread::SubToMainWebSocketReply& wsReply
        ) {};

        void retMsgWorker() override;

        virtual void handleMsg(
            AikariShared::Types::InterThread::SubToMainMessageInstance& msgIns
        );
    };
}  // namespace AikariShared::Infrastructure::InterThread
