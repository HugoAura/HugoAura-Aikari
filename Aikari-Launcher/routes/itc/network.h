#pragma once

#include <Aikari-PLS/types/infrastructure/messageQueue.h>
#include <nlohmann/json.hpp>

namespace AikariLauncherRoutes::InterThread::Network
{
AikariPLS::Types::infrastructure::InputMessageStruct handleCtrlMessage(
    const nlohmann::json& plsIncomingMsgDataSection
);
}
