#pragma once

#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Launcher-Private/types/components/wsTypes.h>
#include <memory>
#include <string>

namespace AikariShared::infrastructure::MessageQueue
{
    template <typename MessageType>
    class SinglePointMessageQueue;
};  // namespace AikariShared::infrastructure::MessageQueue

namespace AikariLauncherComponents::AikariWebSocketHandler
{
    AikariTypes::components::websocket::MODULES getMsgModule(
        std::string moduleStr
    );

    void handleTask(
        AikariTypes::components::websocket::ClientWSTask task,
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariTypes::components::websocket::ServerWSTaskRet>* retMsgQueue
    );
}  // namespace AikariLauncherComponents::AikariWebSocketHandler
