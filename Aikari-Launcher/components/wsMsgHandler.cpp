#include "pch.h"

#include "wsMsgHandler.h"

#include <Aikari-PLS/types/infrastructure/messageQueue.h>
#include <Aikari-Shared/infrastructure/SinglePointMessageQueue.hpp>
#include <nlohmann/json.hpp>

#include "lifecycle.h"

namespace wsTypes = AikariTypes::components::websocket;
namespace messageQueue = AikariShared::infrastructure::MessageQueue;

namespace AikariLauncherComponents::AikariWebSocketHandler
{
wsTypes::MODULES getMsgModule(std::string moduleStr)
{
    if (moduleStr == "launcher")
    {
        return wsTypes::MODULES::LAUNCHER;
    }
    else if (moduleStr == "pls")
    {
        return wsTypes::MODULES::PLS;
    }
    else if (moduleStr == "auraUpdate")
    {
        return wsTypes::MODULES::AUA;
    }
    else
    {
        return wsTypes::MODULES::UNKNOWN;
    }
}

namespace RepTemplates
{
static const nlohmann::json moduleNotFoundJSON = {
    { "message", "Requested module not found" }
};

static const wsTypes::ServerWSRep moduleNotFoundRep = {
    .code = 9000, .success = false, .data = moduleNotFoundJSON
};  // namespace RepTemplates
}  // namespace RepTemplates

void handleTask(
    wsTypes::ClientWSTask task,
    messageQueue::SinglePointMessageQueue<wsTypes::ServerWSTaskRet>* retMsgQueue
)
{
    wsTypes::MODULES msgModule = getMsgModule(task.content.module);
    switch (msgModule)
    {
        case wsTypes::MODULES::LAUNCHER:
        {
            // TO DO: Handle msg for launcher
        }
        break;
        case wsTypes::MODULES::PLS:
        {
            auto& lifecycleStates =
                AikariLifecycle::AikariStatesManager::getInstance();
            auto sharedMsgQueues =
                lifecycleStates.getVal(&AikariTypes::global::lifecycle::
                                           GlobalLifecycleStates::sharedMsgQueue
                );
            if (!sharedMsgQueues.plsInputQueue)
            {
                LOG_ERROR("PLSInputQueue not found, is PLS correctly loaded?");
                wsTypes::ServerWSRep repContent{
                    .code = -1,
                    .eventId = task.content.eventId,
                    .success = false,
                    .data = { { "message", "PLS module load failed" },
                              { "diagnoseId", "E_SUBMD_PLS_QUEUE_ENOENT" } },
                };
                wsTypes::ServerWSTaskRet taskRet{
                    .result = repContent,
                    .clientId = task.clientId,
                    .isBroadcast = false,
                };
                retMsgQueue->push(std::move(taskRet));
                return;
            }

            AikariPLS::Types::infrastructure::WebSocketInfo curMsgWsInfo{
                .clientId = task.clientId
            };

            AikariPLS::Types::infrastructure::InputMessageStruct plsInputMsg{
                .method = task.content.method,
                .data = task.content.data,
                .type =
                    AikariPLS::Types::infrastructure::MESSAGE_TYPES::WS_MESSAGE,
                .fromModule = "launcher",
                .eventId = task.content.eventId,
                .wsInfo = curMsgWsInfo
            };

            sharedMsgQueues.plsInputQueue->push(std::move(plsInputMsg));
        }
        break;
        case wsTypes::MODULES::UNKNOWN:
        {
            wsTypes::ServerWSTaskRet retVal;
            wsTypes::ServerWSRep repContent;
            repContent = RepTemplates::moduleNotFoundRep;
            repContent.eventId = task.content.eventId;
            retVal.clientId = task.clientId;
            retVal.result = repContent;
            retMsgQueue->push(std::move(retVal));
        }
        break;
    }
}
}  // namespace AikariLauncherComponents::AikariWebSocketHandler
