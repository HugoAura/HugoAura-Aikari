#include "mqttPacketUtils.h"

#include <Aikari-Shared/utils/string.h>
#include <unordered_map>

#include "../components/mqtt/mqttLifecycle.h"

namespace AikariPLS::Utils::MQTTPacketUtils
{
    AikariPLS::Types::mqttMsgQueue::PacketTopicProps getPacketProps(
        const std::string& topic
    )
    {
        const auto splitResult = AikariShared::utils::string::split(topic, '/');
        // clang-format off
        // ["", "sys", "<productKey>", "<deviceId>", "<action>", "<side>", "<?id>"]
        // clang-format on
        if (splitResult.size() <= 4)
        {
            throw std::invalid_argument("Invalid topic: " + topic);
        }
        const std::string& action = splitResult.at(4);
        AikariPLS::Types::mqttMsgQueue::PacketTopicProps result;
        result.productKey = splitResult.at(2);
        result.deviceId = splitResult.at(3);
        result.side = splitResult.at(5) == "response"
                          ? AikariPLS::Types::mqttMsgQueue::PACKET_SIDE::REP
                          : AikariPLS::Types::mqttMsgQueue::PACKET_SIDE::REQ;
        if (action == "rpc")
        {
            result.endpointType =
                AikariPLS::Types::mqttMsgQueue::PACKET_ENDPOINT_TYPE::RPC;
            result.msgId = splitResult.at(6);
        }
        else if (action == "thing")
        {
            result.endpointType =
                AikariPLS::Types::mqttMsgQueue::PACKET_ENDPOINT_TYPE::POST;
        }
        else
        {
            result.endpointType =
                AikariPLS::Types::mqttMsgQueue::PACKET_ENDPOINT_TYPE::GET;
            result.msgId = splitResult.at(6);
        }
        return result;
    }

    std::string mergeTopic(
        AikariPLS::Types::mqttMsgQueue::PacketTopicProps& props
    )
    {
        const std::string sharedHead =
            std::format("/sys/{}/{}", props.productKey, props.deviceId);
        switch (props.endpointType)
        {
            case AikariPLS::Types::mqttMsgQueue::PACKET_ENDPOINT_TYPE::GET:
            {
                return std::format(
                    "{}/up/{}/{}",
                    sharedHead,
                    props.side == Types::mqttMsgQueue::PACKET_SIDE::REQ
                        ? "request"
                        : "response",
                    props.msgId.value()
                );
            }
            case AikariPLS::Types::mqttMsgQueue::PACKET_ENDPOINT_TYPE::POST:
            {
                return std::format("{}/thing/post", sharedHead);
            }
            case AikariPLS::Types::mqttMsgQueue::PACKET_ENDPOINT_TYPE::RPC:
            {
                return std::format(
                    "{}/rpc/{}/{}",
                    sharedHead,
                    props.side == Types::mqttMsgQueue::PACKET_SIDE::REQ
                        ? "request"
                        : "response",
                    props.msgId.value()
                );
            }
            default:
            {
                throw std::invalid_argument(
                    "Invalid packet prop endpointType: " +
                    static_cast<int>(props.endpointType)
                );
            }
        }
    }

    async_mqtt::packet_variant reconstructPacket(
        async_mqtt::packet_variant& oldPacket,
        const std::function<async_mqtt::packet_id_type()>& buildNewPacketIdFn,
        std::optional<std::string> topicNameForPublish,
        std::optional<std::string> newPayload
    )
    {
        auto& sharedMsgQueues =
            AikariPLS::Lifecycle::MQTT::PLSMQTTMsgQueues::getInstance();
        auto packetIdMap = sharedMsgQueues.getVal(
            &AikariPLS::Types::lifecycle::MQTT::PLSMQTTMsgQueues::packetIdMap
        );

        async_mqtt::packet_variant result(oldPacket);

        oldPacket.visit(
            async_mqtt::overload{
                [&](async_mqtt::v3_1_1::subscribe_packet const& pkt)
                {
                    auto newPacketId = buildNewPacketIdFn();
                    packetIdMap[newPacketId] = pkt.packet_id();

                    result = async_mqtt::v3_1_1::subscribe_packet(
                        newPacketId, pkt.entries()
                    );
                },
                [&](async_mqtt::v3_1_1::suback_packet const& pkt)
                {
                    auto newPacketId = packetIdMap[pkt.packet_id()];

                    result = async_mqtt::v3_1_1::suback_packet(
                        newPacketId, pkt.entries()
                    );
                },
                [&](async_mqtt::v3_1_1::unsubscribe_packet const& pkt)
                {
                    auto newPacketId = buildNewPacketIdFn();
                    packetIdMap[newPacketId] = pkt.packet_id();

                    result = async_mqtt::v3_1_1::unsubscribe_packet(
                        newPacketId, pkt.entries()
                    );
                },
                [&](async_mqtt::v3_1_1::unsuback_packet const& pkt)
                {
                    auto newPacketId = packetIdMap[pkt.packet_id()];

                    result = async_mqtt::v3_1_1::unsuback_packet(newPacketId);
                },
                [&](async_mqtt::v3_1_1::publish_packet const& pkt)
                {
                    auto newPacketId = buildNewPacketIdFn();

                    result = async_mqtt::v3_1_1::publish_packet(
                        newPacketId,
                        topicNameForPublish.value_or(pkt.topic()),
                        newPayload.value_or(pkt.payload()),
                        pkt.opts()
                    );
                },
                // Auto PUBACK is ON, so type !== PUBACK
                [&](auto const&)
                {
                    result = oldPacket;
                } }
        );

        sharedMsgQueues.setVal(
            &AikariPLS::Types::lifecycle::MQTT::PLSMQTTMsgQueues::packetIdMap,
            packetIdMap
        );

        return result;
    };

}  // namespace AikariPLS::Utils::MQTTPacketUtils
