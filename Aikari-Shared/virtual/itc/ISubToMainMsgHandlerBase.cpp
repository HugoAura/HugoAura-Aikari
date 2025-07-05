#include <Aikari-Shared/virtual/itc/ISubToMainMsgHandlerBase.h>

// mostly same as IMTSMsgHandlerBase, but some logic & types are different

namespace AikariShared::infrastructure::InterThread
{

    // ↓ public

    SubToMainMsgHandlerBase::SubToMainMsgHandlerBase(
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariShared::Types::InterThread::SubToMainMessageInstance>*
            srcQueue,
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariShared::Types::InterThread::MainToSubMessageInstance>*
            reportQueue,
        const std::string subModuleName
    )
        : srcQueue(srcQueue),
          reportQueue(reportQueue),
          logHeader("[" + subModuleName + "->Main]")
    {
        this->srcMsgWorkerThread = std::make_unique<std::jthread>(
            &SubToMainMsgHandlerBase::retMsgWorker, this
        );
        this->threadPool = std::make_unique<
            AikariShared::infrastructure::MessageQueue::PoolQueue<
                AikariShared::Types::InterThread::SubToMainMessageInstance>>(
            DEFAULT_THREAD_COUNT,
            [this](AikariShared::Types::InterThread::SubToMainMessageInstance
                       content)
            {
                this->handleMsg(content);
            }
        );
    };

    void SubToMainMsgHandlerBase::manualDestroy()
    {
        // No need to push, this is triggered by main proc
        if (this->srcMsgWorkerThread->joinable())
        {
            this->srcMsgWorkerThread->join();
        }
        return;
    };

    void SubToMainMsgHandlerBase::addCtrlMsgCallbackListener(
        const AikariShared::Types::InterThread::eventId& eventId,
        std::move_only_function<
            void(AikariShared::Types::InterThread::SubToMainControlReplyMessage
            )> callbackLambda
    )
    {
        this->listeners[eventId].emplace_back(std::move(callbackLambda));
    };

    // ↓ protected

    void SubToMainMsgHandlerBase::retMsgWorker()
    {
        std::string logHeader = this->logHeader;
        try
        {
            LOG_INFO(logHeader + " Starting message queue handler...");
            while (true)
            {
                auto srcMsg = this->srcQueue->pop();
                if (srcMsg.type == AikariShared::Types::InterThread::
                                       MESSAGE_TYPES::DESTROY_MESSAGE)
                {
                    LOG_INFO(
                        "{} Destroy SIG received, exiting loop...", logHeader
                    );
                    break;
                }
                this->threadPool->pushTask(srcMsg);
            }
        }
        catch (const std::exception& err)
        {
            LOG_CRITICAL(
                "{} Critical error occurred running msg handling loop, error: "
                "{}",
                logHeader,
                err.what()
            );
        }
    };

    void SubToMainMsgHandlerBase::handleMsg(
        AikariShared::Types::InterThread::SubToMainMessageInstance& msgIns
    )
    {
        try
        {
            switch (msgIns.type)
            {
                case AikariShared::Types::InterThread::MESSAGE_TYPES::
                    CONTROL_MESSAGE:
                {
                    auto& msgContent =
                        std::get<AikariShared::Types::InterThread::
                                     SubToMainControlMessage>(msgIns.msg);

                    this->onControlMessage(msgContent);
                }
                break;

                case AikariShared::Types::InterThread::MESSAGE_TYPES::
                    CONTROL_MESSAGE_REPLY:
                {
                    auto& msgContent =
                        std::get<AikariShared::Types::InterThread::
                                     SubToMainControlReplyMessage>(msgIns.msg);

                    auto& eventId = msgContent.eventId;

                    {
                        if (this->listeners.find(eventId) !=
                            this->listeners.end())
                        {
                            while (!this->listeners[eventId].empty())
                            {
                                auto callbackLambda =
                                    std::move(this->listeners[eventId].front());

                                this->listeners[eventId].erase(
                                    this->listeners[eventId].begin()
                                );

                                std::jthread listenerThread(
                                    std::move(callbackLambda), msgContent
                                );
                                listenerThread.detach();
                            }

                            if (this->listeners[eventId].empty())
                            {
                                this->listeners.erase(eventId);
                            }
                        }
                    }

                    this->onControlReply(msgContent);
                }
                break;

                case AikariShared::Types::InterThread::MESSAGE_TYPES::
                    WS_MESSAGE:
                {
                    auto& msgContent =
                        std::get<AikariShared::Types::InterThread::
                                     SubToMainWebSocketReply>(msgIns.msg);

                    this->onWebSocketMessage(msgContent);
                }
                break;

                default:
                    LOG_ERROR(
                        "{} Received unknown message type: TypeID={}",
                        this->logHeader,
                        static_cast<int>(msgIns.type)
                    );
            }
        }
        catch (const std::exception& err)
        {
            LOG_ERROR(
                "{} Unexpected error processing message: {}",
                this->logHeader,
                err.what()
            );
        }
    };
};  // namespace AikariShared::infrastructure::InterThread
