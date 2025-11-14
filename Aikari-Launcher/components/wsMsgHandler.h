#pragma once

#include <Aikari-Launcher-Public/types/components/wsTypes.h>
#include <string>

namespace AikariShared::Infrastructure::MessageQueue
{
    template <typename MessageType>
    class SinglePointMessageQueue;
};  // namespace AikariShared::Infrastructure::MessageQueue

namespace AikariLauncherComponents::AikariWebSocketDispatcher
{
    AikariLauncherPublic::Types::Components::WebSocket::ServerWSRep dispatch(
        const AikariLauncherPublic::Types::Components::WebSocket::ClientWSMsg&
            clientMsgProps,
        const std::vector<std::string>& methodVec
    );
}

namespace AikariLauncherComponents::AikariWebSocketHandler
{
    AikariLauncherPublic::Types::Components::WebSocket::MODULES getMsgModule(
        std::string& moduleStr
    );

    void handleTask(
        AikariLauncherPublic::Types::Components::WebSocket::ClientWSTask task,
        AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariLauncherPublic::Types::Components::WebSocket::
                ServerWSTaskRet>* retMsgQueue
    );
}  // namespace AikariLauncherComponents::AikariWebSocketHandler
