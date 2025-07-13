#pragma once

#include <async_mqtt/protocol/packet/packet_variant.hpp>

namespace AikariPLS::Types::mqttMsgQueue
{
    enum class PACKET_OPERATION_TYPE
    {
        PKT_TRANSPARENT,
        PKT_MODIFIED,
        PKT_DROP,
        CTRL_THREAD_END
    };

    struct FlaggedPacket
    {
        PACKET_OPERATION_TYPE type = PACKET_OPERATION_TYPE::PKT_TRANSPARENT;
        std::optional<async_mqtt::packet_variant> packet;
    };
}  // namespace AikariPLS::Types::mqttMsgQueue
