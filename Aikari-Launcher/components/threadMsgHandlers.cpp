#include "threadMsgHandlers.h"

#include <Aikari-Launcher-Public/constants/itcCtrl/errorTemplates.h>
#include <Aikari-Launcher-Public/constants/itcCtrl/filesystem.h>
#include <Aikari-Launcher-Public/constants/itcCtrl/network.h>
#include <Aikari-Shared/types/itc/shared.h>
#include <Aikari-Shared/utils/string.h>
#include <memory>
#include <stdexcept>
#include <string>

#include "../lifecycle.h"
#include "../routes/itc/filesystemRoute.h"
#include "../routes/itc/networkRoute.h"
#include "wsServer.h"

namespace itcConstants = AikariLauncherPublic::Constants::InterThread;

namespace AikariLauncherComponents::SubModuleSystem::ThreadMsgHandlers
{
    void PLSMsgHandler::onControlMessage(
        const AikariShared::Types::InterThread::SubToMainControlMessage& retMsg
    )
    {
        std::vector<std::string> route =
            AikariShared::utils::string::split(retMsg.method, '.');
        std::string& rootRoute = route.at(0);

        AikariShared::Types::InterThread::MainToSubControlReplyMessage result;

        if (rootRoute == itcConstants::Network::_PREFIX)
        {
            result = AikariLauncherRoutes::InterThread::Network::
                handleNetworkCtrlMessage(retMsg, route);
        }
        else if (rootRoute == itcConstants::FileSystem::_PREFIX)
        {
            result = AikariLauncherRoutes::InterThread::FileSystem::
                handleFsCtrlMessage(retMsg, route);
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
            result.data = AikariLauncherPublic::Constants::InterThread::Errors::
                Templates::ROUTE_NOT_FOUND;
        }

        AikariShared::Types::InterThread::MainToSubMessageInstance reply = {
            .type = AikariShared::Types::InterThread::MESSAGE_TYPES::
                CONTROL_MESSAGE_REPLY,
            .msg = result
        };

        this->reportQueue->push(reply);
    };

    void PLSMsgHandler::onWebSocketMessage(
        const AikariShared::Types::InterThread::SubToMainWebSocketReply& wsReply
    )
    {
        auto& sharedIns = AikariLifecycle::AikariSharedInstances::getInstance();
        auto* wsMgrPtr = sharedIns.getPtr(
            &AikariTypes::global::lifecycle::SharedInstances::wsServerMgrIns
        );

        AikariTypes::components::websocket::ServerWSRep repFinData{
            .code = wsReply.code,
            .eventId = wsReply.eventId,
            .success = wsReply.success,
            .data = wsReply.data,
        };

        AikariTypes::components::websocket::ServerWSTaskRet taskFinRet{
            .result = repFinData,
            .clientId = wsReply.wsInfo.clientId,
            .isBroadcast = wsReply.wsInfo.isBroadcast.value_or(false)
        };

        wsMgrPtr->pushRetQueue(taskFinRet);
    };
}  // namespace AikariLauncherComponents::SubModuleSystem::ThreadMsgHandlers
