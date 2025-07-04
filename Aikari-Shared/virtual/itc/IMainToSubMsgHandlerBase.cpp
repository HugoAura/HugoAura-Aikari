#include "pch.h"

#include <Aikari-Shared/virtual/itc/IMainToSubMsgHandlerBase.h>

namespace itcTypes = AikariShared::Types::InterThread;

namespace AikariShared::infrastructure::InterThread
{
    // ↓ public

    MainToSubMsgHandlerBase::MainToSubMsgHandlerBase(
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            itcTypes::MainToSubMessageInstance>* srcQueue,
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            itcTypes::SubToMainMessageInstance>* destQueue,
        const std::string subModuleName
    )
        : srcQueue(srcQueue),
          destQueue(destQueue),
          logHeader("[Main->" + subModuleName + "]")
    {
        this->srcMsgWorkerThread = std::make_unique<std::jthread>(
            &MainToSubMsgHandlerBase::inputMsgWorker, this
        );
        this->threadPool =
            std::make_unique<AikariShared::infrastructure::MessageQueue::
                                 PoolQueue<itcTypes::MainToSubMessageInstance>>(
                DEFAULT_THREAD_COUNT,
                [this](itcTypes::MainToSubMessageInstance content)
                {
                    this->handleMsg(content);
                }
            );
    };

    void MainToSubMsgHandlerBase::manualDestroy()
    {
        itcTypes::MainToSubDestroyMessage subDestoryMsg;
        itcTypes::MainToSubMessageInstance subDestoryMsgIns = {
            .type = itcTypes::MESSAGE_TYPES::DESTROY_MESSAGE,
            .msg = subDestoryMsg
        };
        this->srcQueue->push(std::move(subDestoryMsgIns));

        itcTypes::SubToMainDestroyMessage mainDestroyMsg;
        itcTypes::SubToMainMessageInstance mainDestroyMsgIns = {
            .type = itcTypes::MESSAGE_TYPES::DESTROY_MESSAGE,
            .msg = mainDestroyMsg
        };
        this->destQueue->push(std::move(mainDestroyMsgIns));

        if (this->srcMsgWorkerThread->joinable())
        {
            this->srcMsgWorkerThread->join();
        }
    };

    void MainToSubMsgHandlerBase::addCtrlMsgCallbackListener(
        const itcTypes::eventId& eventId,
        std::move_only_function<void(itcTypes::MainToSubControlReplyMessage msg
        )> callbackLambda
    )
    {
        this->listeners[eventId].emplace_back(std::move(callbackLambda));
    };

    std::future<itcTypes::MainToSubControlReplyMessage>
    MainToSubMsgHandlerBase::createCtrlMsgPromise(
        const itcTypes::eventId& eventId
    )
    {
        std::promise<itcTypes::MainToSubControlReplyMessage> replyPromise;

        auto future = replyPromise.get_future();

        std::move_only_function<void(
            AikariShared::Types::InterThread::MainToSubControlReplyMessage msg
        )>
            lambda = [promise = std::move(replyPromise
                      )](itcTypes::MainToSubControlReplyMessage msg) mutable
        {
            promise.set_value(msg);
        };

        this->addCtrlMsgCallbackListener(eventId, std::move(lambda));

        return future;
    };

    itcTypes::MainToSubControlReplyMessage
    MainToSubMsgHandlerBase::sendCtrlMsgSync(
        itcTypes::SubToMainControlMessage& ctrlMsg
    )
    {
        itcTypes::SubToMainMessageInstance ctrlMsgIns = {
            .type = itcTypes::MESSAGE_TYPES::CONTROL_MESSAGE, .msg = ctrlMsg
        };

        std::string eventId = ctrlMsg.eventId;

        auto msgFuture = this->createCtrlMsgPromise(eventId);

        this->destQueue->push(std::move(ctrlMsgIns));

        return msgFuture.get();
    };

    // ↓ protected

    void MainToSubMsgHandlerBase::inputMsgWorker()
    {
        std::string logHeader = this->logHeader;
        try
        {
            LOG_INFO(logHeader + " Starting message queue handler...");
            while (true)
            {
                auto srcMsg = this->srcQueue->pop();
                if (srcMsg.type == itcTypes::MESSAGE_TYPES::DESTROY_MESSAGE)
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
                this->logHeader,
                err.what()
            );
        }
    };

    void MainToSubMsgHandlerBase::handleMsg(
        itcTypes::MainToSubMessageInstance& msgIns
    )
    {
        try
        {
            switch (msgIns.type)
            {
                case itcTypes::MESSAGE_TYPES::CONTROL_MESSAGE:
                {
                    auto& msgContent =
                        std::get<itcTypes::MainToSubControlMessage>(msgIns.msg
                        );  // if convert failed, error will be directly
                            // catched

                    this->onControlMessage(msgContent
                    );  // we're in a standalone thread, so just go sync
                }
                break;

                case itcTypes::MESSAGE_TYPES::CONTROL_MESSAGE_REPLY:
                {
                    auto& msgContent =
                        std::get<itcTypes::MainToSubControlReplyMessage>(
                            msgIns.msg
                        );

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

                case itcTypes::MESSAGE_TYPES::WS_MESSAGE:
                {
                    auto& msgContent =
                        std::get<itcTypes::MainToSubWebSocketMessage>(msgIns.msg
                        );

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
