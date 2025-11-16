#include "threadMsgHandlers.h"

#include <Aikari-Launcher-Public/constants/itcCtrl/base.h>
#include <Aikari-Launcher-Public/constants/itcCtrl/errorTemplates.h>
#include <Aikari-Launcher-Public/constants/itcCtrl/filesystem.h>
#include <Aikari-Launcher-Public/constants/itcCtrl/network.h>
#include <Aikari-Shared/types/itc/shared.h>
#include <Aikari-Shared/utils/string.h>
#include <exception>
#include <memory>
#include <string>

#include "../lifecycle.h"
#include "../routes/itc/baseRoute.h"
#include "../routes/itc/filesystemRoute.h"
#include "../routes/itc/networkRoute.h"
#include "wsServer.h"

namespace itcConstants = AikariLauncher::Public::Constants::InterThread;

namespace AikariLauncher::Components::SubModuleSystem::ThreadMsgHandlers
{
    void PLSMsgHandler::onControlMessage(
        AikariShared::Types::InterThread::SubToMainControlMessage& retMsg
    )
    {
        std::vector<std::string> route =
            AikariShared::Utils::String::split(retMsg.method, '.');
        std::string& rootRoute = route.at(0);

        AikariShared::Types::InterThread::MainToSubControlReplyMessage result;

        if (rootRoute == itcConstants::Network::_PREFIX)
        {
            result = AikariLauncher::Routes::InterThread::Network::
                handleNetworkCtrlMessage(retMsg, route);
        }
        else if (rootRoute == itcConstants::FileSystem::_PREFIX)
        {
            result = AikariLauncher::Routes::InterThread::FileSystem::
                handleFsCtrlMessage(retMsg, route);
        }
        else if (rootRoute == itcConstants::Base::_PREFIX)
        {
            result = AikariLauncher::Routes::InterThread::Base::handleBaseCtrlMsg(
                retMsg, route
            );
        }
        else
        {
            // handle route not found
            LOG_WARN(
                "{} Src sent an invalid method: {}",
                this->logHeader,
                retMsg.method
            );
            result.eventId = retMsg.eventId;
            result.data = AikariLauncher::Public::Constants::InterThread::Errors::
                Templates::ROUTE_NOT_FOUND;
        }

        AikariShared::Types::InterThread::MainToSubMessageInstance reply = {
            .type = AikariShared::Types::InterThread::MESSAGE_TYPES::
                CONTROL_MESSAGE_REPLY,
            .msg = result
        };

        this->reportQueue->push(std::move(reply));
    };

    void PLSMsgHandler::onWebSocketMessage(
        AikariShared::Types::InterThread::SubToMainWebSocketReply& wsReply
    )
    {
        auto& sharedIns = AikariLifecycle::AikariSharedInstances::getInstance();
        auto* wsMgrPtr = sharedIns.getPtr(
            &AikariTypes::Global::Lifecycle::SharedInstances::wsServerMgrIns
        );

        AikariLauncher::Public::Types::Components::WebSocket::ServerWSRep
            repFinData{
                .code = wsReply.code,
                .eventId = std::move(wsReply.eventId),
                .success = wsReply.success,
                .data = std::move(wsReply.data),
            };

        AikariLauncher::Public::Types::Components::WebSocket::ServerWSTaskRet
            taskFinRet{ .result = std::move(repFinData),
                        .clientId = std::move(wsReply.wsInfo.clientId),
                        .isBroadcast =
                            wsReply.wsInfo.isBroadcast.value_or(false) };

        wsMgrPtr->pushRetQueue(taskFinRet);
    };
}  // namespace AikariLauncher::Components::SubModuleSystem::ThreadMsgHandlers
