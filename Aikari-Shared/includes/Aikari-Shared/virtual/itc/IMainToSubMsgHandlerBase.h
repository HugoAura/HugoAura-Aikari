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

        std::future<
            AikariShared::Types::InterThread::MainToSubControlReplyMessage>
        createCtrlMsgPromise(
            const AikariShared::Types::InterThread::eventId& eventId
        );

        std::optional<
            AikariShared::Types::InterThread::MainToSubControlReplyMessage>
        sendCtrlMsgSync(
            AikariShared::Types::InterThread::SubToMainControlMessage& ctrlMsg,
            std::chrono::milliseconds timeout = std::chrono::milliseconds(1000),
            bool retry = false,
            unsigned int maxTries = 3
        );

       protected:
        const std::string logHeader;
        std::deque<AikariShared::Types::InterThread::MainToSubMessageInstance>
            msgTempStore;

        AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariShared::Types::InterThread::MainToSubMessageInstance>*
            srcQueue;
        AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariShared::Types::InterThread::SubToMainMessageInstance>*
            destQueue;

        std::map<
            AikariShared::Types::InterThread::eventId,
            std::vector<std::move_only_function<
                void(AikariShared::Types::InterThread::
                         MainToSubControlReplyMessage msg)>>>
            listeners;

        std::unique_ptr<AikariShared::Infrastructure::MessageQueue::PoolQueue<
            AikariShared::Types::InterThread::MainToSubMessageInstance>>
            threadPool;
        std::unique_ptr<std::jthread> srcMsgWorkerThread;

        virtual void onControlMessage(
            AikariShared::Types::InterThread::MainToSubControlMessage& srcMsg
        ) {};
        virtual void onControlReply(
            AikariShared::Types::InterThread::MainToSubControlReplyMessage&
                retMsg
        ) {};
        virtual void onWebSocketMessage(
            AikariShared::Types::InterThread::MainToSubWebSocketMessage& srcMsg
        ) {};

        void inputMsgWorker() override;

        virtual void handleMsg(
            AikariShared::Types::InterThread::MainToSubMessageInstance& msgIns
        );
    };
}  // namespace AikariShared::Infrastructure::InterThread
