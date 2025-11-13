#pragma once

#include <Aikari-Launcher-Private/types/components/wsTypes.h>
#include <string>

namespace AikariShared::Infrastructure::MessageQueue
{
    template <typename MessageType>
    class SinglePointMessageQueue;
};  // namespace AikariShared::Infrastructure::MessageQueue

namespace AikariLauncherComponents::AikariWebSocketDispatcher
{
    AikariTypes::Components::WebSocket::ServerWSRep dispatch(
        const AikariTypes::Components::WebSocket::ClientWSMsg& clientMsgProps,
        const std::vector<std::string>& methodVec
    );
}

namespace AikariLauncherComponents::AikariWebSocketHandler
{
    AikariTypes::Components::WebSocket::MODULES getMsgModule(
        std::string& moduleStr
    );

    void handleTask(
        AikariTypes::Components::WebSocket::ClientWSTask task,
        AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariTypes::Components::WebSocket::ServerWSTaskRet>* retMsgQueue
    );
}  // namespace AikariLauncherComponents::AikariWebSocketHandler
