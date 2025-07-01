#pragma once

#include <Aikari-PLS/types/infrastructure/messageQueue.h>

namespace AikariPLS::Types::constants::msgQueue
{
namespace WebSocket
{
const char* WS_MSG_TYPE_PUSH = "PUSH";

const AikariPLS::Types::infrastructure::WebSocketInfo baseBroadcastWSInfo = {
    .isBroadcast = true
};
}  // namespace WebSocket
}  // namespace AikariPLS::Types::constants::msgQueue
