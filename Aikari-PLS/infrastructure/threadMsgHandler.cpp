#include "pch.h"

#include "threadMsgHandler.h"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <stdexcept>

namespace MessageQueueTypes = AikariShared::infrastructure::MessageQueue;
namespace PLSInfTypes = AikariPLS::Types::infrastructure;

static const std::string mainThreadIncomingMethodHead = "[Main->PLS]";

namespace AikariPLS::Infrastructure::MsgQueue
{
PLSThreadMsgQueueHandler::PLSThreadMsgQueueHandler(
    MessageQueueTypes::SinglePointMessageQueue<PLSInfTypes::InputMessageStruct>*
        srcQueue,
    MessageQueueTypes::SinglePointMessageQueue<PLSInfTypes::RetMessageStruct>*
        destQueue
)
    : srcQueue(srcQueue), destQueue(destQueue)
{
    this->inputMsgWorkerThread = std::make_unique<std::jthread>(
        &PLSThreadMsgQueueHandler::inputMsgWorker, this
    );
};

void PLSThreadMsgQueueHandler::manualDestroy()
{
    PLSInfTypes::InputMessageStruct inputDestroyMsg;
    inputDestroyMsg.fromModule = "PLS";
    inputDestroyMsg.type = PLSInfTypes::MESSAGE_TYPES::DESTROY_MESSAGE;
    this->srcQueue->push(std::move(inputDestroyMsg));

    // the destroy of remote thread will be processed in inputMsgWorker

    if (this->inputMsgWorkerThread->joinable())
    {
        this->inputMsgWorkerThread->join();
    }

    return;
};

void PLSThreadMsgQueueHandler::inputMsgWorker()
{
    LOG_INFO(
        "{} Starting Main->PLS message queue handler...",
        mainThreadIncomingMethodHead
    );
    try
    {
        while (true)
        {
            auto inputMsg = this->srcQueue->pop();
            switch (inputMsg.type)
            {
                case PLSInfTypes::MESSAGE_TYPES::CONTROL_MESSAGE:
                {
#ifdef _DEBUG
                    LOG_TRACE(
                        "{} Received ctrl msg, method: {}, data:\n{}",
                        mainThreadIncomingMethodHead,
                        inputMsg.method,
                        inputMsg.data.dump()
                    );
#endif
                }
                break;
                case PLSInfTypes::MESSAGE_TYPES::WS_MESSAGE:
                {
#ifdef _DEBUG
                    LOG_TRACE(
                        "{} Received ws msg, method: {}, data:\n{}",
                        mainThreadIncomingMethodHead,
                        inputMsg.method,
                        inputMsg.data.dump()
                    );
#endif
                }
                break;
                case PLSInfTypes::MESSAGE_TYPES::DESTROY_MESSAGE:
                {
                    LOG_INFO(
                        "{} Destory sig recv, running "
                        "road... (bs",
                        mainThreadIncomingMethodHead
                    );
                    PLSInfTypes::RetMessageStruct retRemoteDestroyMsg;
                    retRemoteDestroyMsg.success = true;
                    retRemoteDestroyMsg.code = -2;
                    retRemoteDestroyMsg.type =
                        PLSInfTypes::MESSAGE_TYPES::DESTROY_MESSAGE;
                    this->destQueue->push(std::move(retRemoteDestroyMsg));

                    return;  // run road!
                }
            }
        }
    }
    catch (const std::exception& err)
    {
        LOG_CRITICAL(
            "{} Critical error occurred running Main->PLS msg handling loop, "
            "error: {}",
            mainThreadIncomingMethodHead,
            err.what()
        );
    }
}
};  // namespace AikariPLS::Infrastructure::MsgQueue
