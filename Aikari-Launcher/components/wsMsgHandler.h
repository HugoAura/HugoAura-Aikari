#pragma once

#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Launcher-Private/types/components/wsTypes.h>
#include <memory>
#include <string>

namespace AikariShared::Infrastructure::MessageQueue
{
    template <typename MessageType>
    class SinglePointMessageQueue;
};  // namespace AikariShared::Infrastructure::MessageQueue

namespace AikariLauncherComponents::AikariWebSocketHandler
{
    AikariTypes::Components::WebSocket::MODULES getMsgModule(
        std::string moduleStr
    );

    void handleTask(
        AikariTypes::Components::WebSocket::ClientWSTask task,
        AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariTypes::Components::WebSocket::ServerWSTaskRet>* retMsgQueue
    );
}  // namespace AikariLauncherComponents::AikariWebSocketHandler
