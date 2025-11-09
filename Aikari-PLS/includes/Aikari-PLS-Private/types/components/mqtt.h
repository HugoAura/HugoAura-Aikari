#pragma once

#include <async_mqtt/protocol/packet/packet_variant.hpp>
#include <string>

namespace AikariPLS::Types::MQTTMsgQueue
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
        UNKNOWN,
    };

    std::string to_string(PACKET_ENDPOINT_TYPE endpointType);

    enum class PACKET_SIDE
    {
        REQ,
        REP,
        UNKNOWN,
    };

    std::string to_string(PACKET_SIDE pktSide);

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
        std::optional<async_mqtt::packet_variant>
            packet;  // for virtual packet, this arg is nullopt, newPayload =
                     // payload
        PacketTopicProps props;
        std::optional<std::string> newPayload;
    };
}  // namespace AikariPLS::Types::MQTTMsgQueue
