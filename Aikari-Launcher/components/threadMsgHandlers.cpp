#include "pch.h"

#include "threadMsgHandlers.h"

#include <Aikari-PLS/types/infrastructure/messageQueue.h>
#include <memory>
#include <stdexcept>
#include <string>

#include "../lifecycle.h"
#include "wsServer.h"

namespace LogHeaders
{
static const std::string plsIncomingMethodHead = "[PLS->Main]";
}

namespace AikariLauncherComponents::SubModuleSystem::ThreadMsgHandlers
{
void plsIncomingMsgHandler(
    std::shared_ptr<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariPLS::Types::infrastructure::RetMessageStruct>> retMsgQueue
)
{
    LOG_INFO(
        "{} Starting PLS->Main message queue handler...",
        LogHeaders::plsIncomingMethodHead
    );
    auto& lifecycleStates = AikariLifecycle::AikariStatesManager::getInstance();
    auto sharedIns = lifecycleStates.getVal(
        &AikariTypes::global::lifecycle::GlobalLifecycleStates::sharedIns
    );
    auto& wsMgrPtr = sharedIns.wsServerMgrIns;

    try
    {
        while (true)
        {
            auto retMsg = retMsgQueue->pop();
            switch (retMsg.type)
            {
                case AikariPLS::Types::infrastructure::MESSAGE_TYPES::
                    CONTROL_MESSAGE:
                {
#ifdef _DEBUG
                    LOG_TRACE(
                        "{} Received ctrl msg, type: {}, data:\n{}",
                        LogHeaders::plsIncomingMethodHead,
                        static_cast<int>(retMsg.type),
                        retMsg.data.dump()
                    );
#endif
                }
                break;
                case AikariPLS::Types::infrastructure::MESSAGE_TYPES::
                    WS_MESSAGE:
                {
                    if (!retMsg.wsInfo.has_value())
                    {
                        LOG_ERROR(
                            "{} Invalid message received: Type is ws "
                            "but no wsInfo provided.",
                            LogHeaders::plsIncomingMethodHead
                        );
                        break;
                    }
                    AikariTypes::components::websocket::ServerWSRep repFinData{
                        .code = retMsg.code,
                        .eventId = retMsg.eventId.value_or(""),
                        .success = retMsg.success,
                        .data = retMsg.data,
                    };

                    AikariTypes::components::websocket::ServerWSTaskRet
                        taskFinRet{
                            .result = repFinData,
                            .clientId = retMsg.wsInfo.value().clientId,
                            .isBroadcast =
                                retMsg.wsInfo.value().isBroadcast.value_or(false
                                )
                        };

                    wsMgrPtr->pushRetQueue(taskFinRet);
                }
                break;
                case AikariPLS::Types::infrastructure::MESSAGE_TYPES::
                    DESTROY_MESSAGE:
                    LOG_INFO(
                        "{} Destroy sig recv, exiting thread...",
                        LogHeaders::plsIncomingMethodHead
                    );
                    return;
            }
        }
    }
    catch (const std::exception& err)
    {
        LOG_CRITICAL(
            "{} Critical error occurred running PLS->Main msg handling loop, "
            "error: {}",
            LogHeaders::plsIncomingMethodHead,
            err.what()
        );
    }
};
}  // namespace AikariLauncherComponents::SubModuleSystem::ThreadMsgHandlers
