#include "wsMsgHandler.h"

#include <Aikari-Launcher-Public/constants/ws/basic.h>
#include <Aikari-Launcher-Public/constants/ws/errorTemplates.h>
#include <Aikari-Launcher-Public/constants/ws/errors.h>
#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>
#include <Aikari-Shared/types/itc/shared.h>
#include <Aikari-Shared/utils/string.h>
#include <nlohmann/json.hpp>

#include "../lifecycle.h"
#include "../routes/ws/basicRoute.h"
#include "../routes/ws/configRoute.h"
#include "Aikari-Launcher-Public/constants/ws/config.h"

namespace wsTypes = AikariTypes::Components::WebSocket;
namespace wsConstants = AikariLauncherPublic::Constants::WebSocket;
namespace messageQueue = AikariShared::Infrastructure::MessageQueue;

namespace AikariLauncherComponents::AikariWebSocketDispatcher
{
    AikariTypes::Components::WebSocket::ServerWSRep dispatch(
        const AikariTypes::Components::WebSocket::ClientWSMsg& clientMsgProps,
        const std::vector<std::string>& methodVec
    )
    {
        const std::string& topLevelMethod = methodVec.at(0);

        if (topLevelMethod == wsConstants::Basic::_PREFIX)
        {
            return AikariLauncherRoutes::WebSocket::Basic::handleBasicMethods(
                clientMsgProps, methodVec
            );
        }
        else if (topLevelMethod == wsConstants::Config::_PREFIX)
        {
            return AikariLauncherRoutes::WebSocket::Config::handleConfigMethods(
                clientMsgProps, methodVec
            );
        }
        else
        {
            return AikariLauncherPublic::Constants::WebSocket::Errors::
                Templates::METHOD_NOT_FOUND;
        };
    }
}  // namespace AikariLauncherComponents::AikariWebSocketDispatcher

namespace AikariLauncherComponents::AikariWebSocketHandler
{
    wsTypes::MODULES getMsgModule(std::string& moduleStr)
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

    void handleTask(
        wsTypes::ClientWSTask task,
        messageQueue::SinglePointMessageQueue<wsTypes::ServerWSTaskRet>*
            retMsgQueue
    )
    {
        switch (wsTypes::MODULES msgModule = getMsgModule(task.content.module);
                msgModule)
        {
            case wsTypes::MODULES::LAUNCHER:
            {
                std::vector<std::string> methodVec =
                    AikariShared::Utils::String::split(
                        task.content.method, '.'
                    );

                wsTypes::ServerWSRep result =
                    AikariLauncherComponents::AikariWebSocketDispatcher::
                        dispatch(task.content, methodVec);

                result.eventId = task.content.eventId;

                wsTypes::ServerWSTaskRet retMsg = { .result = std::move(result),
                                                    .clientId = task.clientId,
                                                    .isBroadcast = false };

                retMsgQueue->push(std::move(retMsg));
            }
            break;
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
                        .code = AikariLauncherPublic::Constants::WebSocket::
                            Errors::Codes::MODULE_CONNECTION_BROKEN,
                        .eventId = task.content.eventId,
                        .success = false,
                        .data = { { "message", "PLS module load failed" },
                                  { "diagnoseId",
                                    AikariLauncherPublic::Constants::WebSocket::Errors::
                                        MODULE_CONNECTION_BROKEN_VARIANT_QUEUE } },
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
            }
            break;
            case wsTypes::MODULES::UNKNOWN:
            default:
            {
                wsTypes::ServerWSTaskRet retVal;
                wsTypes::ServerWSRep repContent = AikariLauncherPublic::
                    Constants::WebSocket::Errors::Templates::MODULE_NOT_FOUND;
                repContent.eventId = task.content.eventId;
                retVal.clientId = task.clientId;
                retVal.result = repContent;
                retMsgQueue->push(std::move(retVal));
            }
            break;
        }
    }
}  // namespace AikariLauncherComponents::AikariWebSocketHandler
