#pragma once

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/infrastructure/queue/PoolQueue.hpp>
#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>
#include <Aikari-Shared/types/itc/shared.h>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace AikariShared::Infrastructure::InterThread
{
    const int DEFAULT_THREAD_COUNT = 4;

    class IMainToSubMsgHandler
    {
       public:
        virtual ~IMainToSubMsgHandler() = default;

       private:
        virtual void inputMsgWorker() = 0;
    };

    class MainToSubMsgHandlerBase : public IMainToSubMsgHandler
    {
       public:
        MainToSubMsgHandlerBase(
            AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::MainToSubMessageInstance>*
                srcQueue,
            AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::SubToMainMessageInstance>*
                destQueue,
            const std::string subModuleName
        );

        void manualDestroy();

        void addCtrlMsgCallbackListener(
            const AikariShared::Types::InterThread::eventId& eventId,
            std::move_only_function<void(AikariShared::Types::InterThread::
                                             MainToSubControlReplyMessage msg)>
                callbackLambda
        );

        virtual std::future<
            AikariShared::Types::InterThread::MainToSubControlReplyMessage>
        createCtrlMsgPromise(
            const AikariShared::Types::InterThread::eventId& eventId
        );

        virtual AikariShared::Types::InterThread::MainToSubControlReplyMessage
        sendCtrlMsgSync(
            AikariShared::Types::InterThread::SubToMainControlMessage& ctrlMsg
        );

       protected:
        AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariShared::Types::InterThread::MainToSubMessageInstance>*
            srcQueue;
        AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariShared::Types::InterThread::SubToMainMessageInstance>*
            destQueue;

        const std::string logHeader;

        std::map<
            AikariShared::Types::InterThread::eventId,
            std::vector<std::move_only_function<
                void(AikariShared::Types::InterThread::
                         MainToSubControlReplyMessage msg)>>>
            listeners;

        std::unique_ptr<std::jthread> srcMsgWorkerThread;
        std::unique_ptr<AikariShared::Infrastructure::MessageQueue::PoolQueue<
            AikariShared::Types::InterThread::MainToSubMessageInstance>>
            threadPool;

        virtual void onControlMessage(
            const AikariShared::Types::InterThread::MainToSubControlMessage&
                retMsg
        ) {};
        virtual void onControlReply(
            const AikariShared::Types::InterThread::
                MainToSubControlReplyMessage& retMsg
        ) {};
        virtual void onWebSocketMessage(
            const AikariShared::Types::InterThread::MainToSubWebSocketMessage&
                retMsg
        ) {};

        void inputMsgWorker() override;

        virtual void handleMsg(
            AikariShared::Types::InterThread::MainToSubMessageInstance& msgIns
        );
    };
}  // namespace AikariShared::Infrastructure::InterThread
