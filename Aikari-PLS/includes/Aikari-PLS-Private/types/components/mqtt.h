#pragma once

#include <async_mqtt/protocol/packet/packet_variant.hpp>
#include <string>

namespace AikariPLS::Types::mqttMsgQueue
{
    enum class PACKET_OPERATION_TYPE
    {
        PKT_TRANSPARENT,
        PKT_MODIFIED,
        PKT_VIRTUAL,
        CTRL_THREAD_END
    };

    enum class PACKET_ENDPOINT_TYPE
    {
        GET,
        POST,
        RPC,
    };

    enum class PACKET_SIDE
    {
        REQ,
        REP
    };

    struct PacketTopicProps
    {
        PACKET_ENDPOINT_TYPE endpointType;
        std::string deviceId;
        std::string productKey;
        PACKET_SIDE side;
        std::optional<std::string> msgId;
    };

    struct FlaggedPacket
    {
        PACKET_OPERATION_TYPE type = PACKET_OPERATION_TYPE::PKT_TRANSPARENT;
        std::optional<async_mqtt::packet_variant> packet;
        PacketTopicProps props;
    };
}  // namespace AikariPLS::Types::mqttMsgQueue
