#include "wsMsgHandler.h"

#include <Aikari-Launcher-Public/constants/ws/basic.h>
#include <Aikari-Launcher-Public/constants/ws/config.h>
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

namespace wsTypes = AikariLauncher::Public::Types::Components::WebSocket;
namespace wsConstants = AikariLauncher::Public::Constants::WebSocket;
namespace messageQueue = AikariShared::Infrastructure::MessageQueue;

namespace AikariLauncher::Components::AikariWebSocketDispatcher
{
    wsTypes::ServerWSRep dispatch(
        const wsTypes::ClientWSMsg& clientMsgProps,
        const std::vector<std::string>& methodVec
    )
    {
        const std::string& topLevelMethod = methodVec.at(0);

        if (topLevelMethod == wsConstants::Basic::_PREFIX)
        {
            return AikariLauncher::Routes::WebSocket::Basic::handleBasicMethods(
                clientMsgProps, methodVec
            );
        }
        else if (topLevelMethod == wsConstants::Config::_PREFIX)
        {
            return AikariLauncher::Routes::WebSocket::Config::handleConfigMethods(
                clientMsgProps, methodVec
            );
        }
        else
        {
            return wsConstants::Errors::Templates::METHOD_NOT_FOUND;
        };
    }
}  // namespace AikariLauncher::Components::AikariWebSocketDispatcher

namespace AikariLauncher::Components::AikariWebSocketHandler
{
    wsTypes::MODULES getMsgModule(const std::string& moduleStr)
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
                    AikariLauncher::Components::AikariWebSocketDispatcher::
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
                        .code = wsConstants::Errors::Codes::
                            MODULE_CONNECTION_BROKEN,
                        .eventId = task.content.eventId,
                        .success = false,
                        .data = { { "message", "PLS module load failed" },
                                  { "diagnoseId",
                                    wsConstants::Errors::
                                        MODULE_CONNECTION_BROKEN_VARIANT_QUEUE } },
                    };
                    wsTypes::ServerWSTaskRet taskRet{
                        .result = std::move(repContent),
                        .clientId = std::move(task.clientId),
                        .isBroadcast = false,
                    };
                    retMsgQueue->push(std::move(taskRet));
                    return;
                }

                AikariShared::Types::InterThread::MainToSubWebSocketMessage
                    curMsgWsInfo{ .method = std::move(task.content.method),
                                  .data = std::move(task.content.data),
                                  .eventId = std::move(task.content.eventId),
                                  .wsInfo = { .clientId =
                                                  std::move(task.clientId) } };

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
                wsTypes::ServerWSRep repContent = AikariLauncher::Public::
                    Constants::WebSocket::Errors::Templates::MODULE_NOT_FOUND;
                repContent.eventId = task.content.eventId;
                retVal.clientId = task.clientId;
                retVal.result = repContent;
                retMsgQueue->push(std::move(retVal));
            }
            break;
        }
    }
}  // namespace AikariLauncher::Components::AikariWebSocketHandler
