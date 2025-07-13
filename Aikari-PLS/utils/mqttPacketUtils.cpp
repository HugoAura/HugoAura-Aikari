#include "mqttPacketUtils.h"

#include <unordered_map>

#include "../components/mqttLifecycle.h"

namespace AikariPLS::Utils::MQTTPacketUtils
{
    async_mqtt::packet_variant reconstructPacketWithPktId(
        async_mqtt::packet_variant& oldPacket,
        std::function<async_mqtt::packet_id_type()>& buildNewPacketIdFn
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
                        newPacketId, pkt.topic(), pkt.payload(), pkt.opts()
                    );
                },
                [&](async_mqtt::v3_1_1::puback_packet const& pkt)
                {
                    result =
                        pkt;  // Auto PUBACK is ON, so usually this won't be triggered
                },
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
