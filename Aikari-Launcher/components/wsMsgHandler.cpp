#include "wsMsgHandler.h"

#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>
#include <Aikari-Shared/types/itc/shared.h>
#include <nlohmann/json.hpp>

#include "../lifecycle.h"

namespace wsTypes = AikariTypes::Components::WebSocket;
namespace messageQueue = AikariShared::Infrastructure::MessageQueue;

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
        messageQueue::SinglePointMessageQueue<wsTypes::ServerWSTaskRet>*
            retMsgQueue
    )
    {
        wsTypes::MODULES msgModule = getMsgModule(task.content.module);
        switch (msgModule)
        {
            case wsTypes::MODULES::LAUNCHER:
            {
                // TO DO: Handle msg for launcher
                break;
            }
            case wsTypes::MODULES::PLS:
            {
                auto& lifecycleStates =
                    AikariLifecycle::AikariStatesManager::getInstance();
                auto& sharedMsgQueues = lifecycleStates.getVal(
                    &AikariTypes::Global::Lifecycle::GlobalLifecycleStates::
                        sharedMsgQueue
                );
                if (!sharedMsgQueues.plsInputQueue)
                {
                    LOG_ERROR(
                        "PLSInputQueue not found, is PLS correctly loaded?"
                    );
                    wsTypes::ServerWSRep repContent{
                        .code = -1,
                        .eventId = task.content.eventId,
                        .success = false,
                        .data = { { "message", "PLS module load failed" },
                                  { "diagnoseId",
                                    "E_SUBMD_PLS_QUEUE_ENOENT" } },
                    };
                    wsTypes::ServerWSTaskRet taskRet{
                        .result = repContent,
                        .clientId = task.clientId,
                        .isBroadcast = false,
                    };
                    retMsgQueue->push(std::move(taskRet));
                    return;
                }

                AikariShared::Types::InterThread::MainToSubWebSocketMessage
                    curMsgWsInfo{ .method = task.content.method,
                                  .data = task.content.data,
                                  .eventId = task.content.eventId,
                                  .wsInfo = { .clientId = task.clientId } };

                AikariShared::Types::InterThread::MainToSubMessageInstance
                    plsInputMsg{ .type = AikariShared::Types::InterThread::
                                     MESSAGE_TYPES::WS_MESSAGE,
                                 .msg = curMsgWsInfo };

                sharedMsgQueues.plsInputQueue->push(std::move(plsInputMsg));
                break;
            }
            case wsTypes::MODULES::UNKNOWN:
            default:
            {
                wsTypes::ServerWSTaskRet retVal;
                wsTypes::ServerWSRep repContent;
                repContent = RepTemplates::moduleNotFoundRep;
                repContent.eventId = task.content.eventId;
                retVal.clientId = task.clientId;
                retVal.result = repContent;
                retMsgQueue->push(std::move(retVal));
                break;
            }
        }
    }
}  // namespace AikariLauncherComponents::AikariWebSocketHandler
