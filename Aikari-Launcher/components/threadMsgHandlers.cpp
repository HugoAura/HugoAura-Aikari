#include "pch.h"

#include "threadMsgHandlers.h"

#include <Aikari-Launcher-Public/constants/itcCtrl/network.h>
#include <Aikari-Shared/types/itc/shared.h>
#include <Aikari-Shared/utils/string.h>
#include <memory>
#include <stdexcept>
#include <string>

#include "../lifecycle.h"
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

    if (rootRoute == itcConstants::Network::PREFIX)
    {
    }
    else
    {
    }
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
