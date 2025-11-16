#pragma once

#include <Aikari-Launcher-Public/types/components/wsTypes.h>
#include <string>

namespace AikariShared::Infrastructure::MessageQueue
{
    template <typename MessageType>
    class SinglePointMessageQueue;
};  // namespace AikariShared::Infrastructure::MessageQueue

namespace AikariLauncher::Components::AikariWebSocketDispatcher
{
    AikariLauncher::Public::Types::Components::WebSocket::ServerWSRep dispatch(
        const AikariLauncher::Public::Types::Components::WebSocket::ClientWSMsg&
            clientMsgProps,
        const std::vector<std::string>& methodVec
    );
}

namespace AikariLauncher::Components::AikariWebSocketHandler
{
    AikariLauncher::Public::Types::Components::WebSocket::MODULES getMsgModule(
        const std::string& moduleStr
    );

    void handleTask(
        AikariLauncher::Public::Types::Components::WebSocket::ClientWSTask task,
        AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariLauncher::Public::Types::Components::WebSocket::
                ServerWSTaskRet>* retMsgQueue
    );
}  // namespace AikariLauncher::Components::AikariWebSocketHandler
