#include "pch.h"

#include <Aikari-Shared/virtual/itc/IMainToSubMsgHandlerBase.h>

namespace AikariShared::infrastructure::InterThread
{

// ↓ public

MainToSubMsgHandlerBase::MainToSubMsgHandlerBase(
    AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
        AikariShared::Types::InterThread::MainToSubMessageInstance>* srcQueue,
    AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
        AikariShared::Types::InterThread::SubToMainMessageInstance>* destQueue,
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
        std::make_unique<AikariShared::infrastructure::MessageQueue::PoolQueue<
            AikariShared::Types::InterThread::MainToSubMessageInstance>>(
            DEFAULT_THREAD_COUNT,
            [this](AikariShared::Types::InterThread::MainToSubMessageInstance
                       content)
            {
                this->handleMsg(content);
            }
        );
};

void MainToSubMsgHandlerBase::manualDestroy()
{
    AikariShared::Types::InterThread::MainToSubDestroyMessage subDestoryMsg;
    AikariShared::Types::InterThread::MainToSubMessageInstance
        subDestoryMsgIns = { .type = AikariShared::Types::InterThread::
                                 MESSAGE_TYPES::DESTROY_MESSAGE,
                             .msg = subDestoryMsg };
    this->srcQueue->push(std::move(subDestoryMsgIns));

    AikariShared::Types::InterThread::SubToMainDestroyMessage mainDestroyMsg;
    AikariShared::Types::InterThread::SubToMainMessageInstance
        mainDestroyMsgIns = { .type = AikariShared::Types::InterThread::
                                  MESSAGE_TYPES::DESTROY_MESSAGE,
                              .msg = mainDestroyMsg };
    this->destQueue->push(std::move(mainDestroyMsgIns));

    if (this->srcMsgWorkerThread->joinable())
    {
        this->srcMsgWorkerThread->join();
    }
};

void MainToSubMsgHandlerBase::addCtrlMsgCallbackListener(
    const AikariShared::Types::InterThread::eventId& eventId,
    std::function<
        void(AikariShared::Types::InterThread::MainToSubControlReplyMessage msg
        )>& callbackLambda
)
{
    this->listeners[eventId].emplace_back(std::move(callbackLambda));
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
            if (srcMsg.type == AikariShared::Types::InterThread::MESSAGE_TYPES::
                                   DESTROY_MESSAGE)
            {
                LOG_INFO("{} Destroy SIG received, exiting loop...", logHeader);
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
    AikariShared::Types::InterThread::MainToSubMessageInstance& msgIns
)
{
    try
    {
        switch (msgIns.type)
        {
            case AikariShared::Types::InterThread::MESSAGE_TYPES::
                CONTROL_MESSAGE:
            {
                auto& msgContent = std::get<
                    AikariShared::Types::InterThread::MainToSubControlMessage>(
                    msgIns.msg
                );  // if convert failed, error will be directly
                    // catched

                this->onControlMessage(msgContent
                );  // we're in a standalone thread, so just go sync
            }
            break;

            case AikariShared::Types::InterThread::MESSAGE_TYPES::
                CONTROL_MESSAGE_REPLY:
            {
                auto& msgContent =
                    std::get<AikariShared::Types::InterThread::
                                 MainToSubControlReplyMessage>(msgIns.msg);

                auto& eventId = msgContent.eventId;

                {
                    if (this->listeners.find(eventId) != this->listeners.end())
                    {
                        while (!this->listeners[eventId].empty())
                        {
                            auto callbackLambda =
                                std::move(this->listeners[eventId].front());

                            this->listeners[eventId].erase(
                                this->listeners[eventId].begin()
                            );

                            std::jthread listenerThread(
                                callbackLambda, msgContent
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

            case AikariShared::Types::InterThread::MESSAGE_TYPES::WS_MESSAGE:
            {
                auto& msgContent =
                    std::get<AikariShared::Types::InterThread::
                                 MainToSubWebSocketMessage>(msgIns.msg);

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
