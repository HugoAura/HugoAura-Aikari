#pragma once

#include <Aikari-PLS-Private/types/components/mqtt.h>
#include <Aikari-PLS-Private/types/components/rules.h>
#include <async_mqtt/protocol/packet/packet_variant.hpp>
#include <nlohmann/json.hpp>
#include <optional>

namespace AikariPLS::Utils::MQTTPacketUtils
{
    inline const std::string propTypeGetMethodName = "thing.property.get";
    inline const std::string propTypeSetMethodName = "thing.property.set";

    AikariPLS::Types::MQTTMsgQueue::PacketTopicProps getPacketProps(
        const std::string& topic
    );

    std::string mergeTopic(
        AikariPLS::Types::MQTTMsgQueue::PacketTopicProps& props
    );

    // clang-format off

    // RCID == Real Client Packet ID | RBID == Real Broker Packet ID
    // FCID == Fake Client Packet ID | FBID == Fake Broker Packet ID

    // SUB / UNSUB
    // (ASC) Real Client [RCID] -> Fake Broker -> Fake Client {RESET AND LINK: FCID->RCID} [FCID] -> Real Broker
    // (ACK) Real Client <- [RCID] <ACK> {RESET AND RETRIEVE LINK: FCID} Fake Broker <- Fake Client <- [FCID] <ACK> Real Broker

    // PUB
    // (ASCEND) Real Client [RCID] -> <AUTO ACK with RCID> Fake Broker -> Fake Client {RESET} [FCID] -> Real Broker
    // (ASCEND) RB's ACK automatically processed by FC, no transparent pass
    // (INCOME) Real Client <- [FBID] {RESET} Fake Broker <- Fake Client <AUTO ACK with RBID> <- [RBID] Real Broker
    // (INCOME) RC's ACK automatically processed by FB, no pass

    // summarize:
    // Type (ACK as Response) [e.g. sub, unsub]: Build unordered_map to link RCID & FCID
    // Type (Fire and forget) [e.g. pub]: Directly reset ID,

    // since async_mqtt didn't provide an API to set pktId prop of existing
    // pkts, we need to reconstruct every packet

    // clang-format on

    async_mqtt::packet_variant reconstructPacket(
        async_mqtt::packet_variant& oldPacket,
        const std::function<async_mqtt::packet_id_type()>& buildNewPacketIdFn,
        std::optional<std::string> topicNameForPublish,
        std::optional<std::string> newPayload
    );

    nlohmann::json _processPropertyData(
        const nlohmann::json& paramJson,
        const AikariPLS::Types::RuleSystem::RuleMapping::RewriteFeaturesStore&
            featureStore
    );  // nullopt if no changes, paramJson == rootJson["params"] or
        // {rootJson["data"] (when REQ + GET)}

    enum class RewritePacketMethodType
    {
        NORMAL,
        PROP_GET,
        PROP_SET
    };

    struct RewritePacketProcessResult
    {
        RewritePacketMethodType packetMethodType;
        std::optional<std::string> newPayload;
    };

    RewritePacketProcessResult processPacketDataWithRule(
        const std::string& fullPayloadRaw,
        const AikariPLS::Types::RuleSystem::RuleMapping::RewriteFeaturesStore&
            featureStore,
        const AikariPLS::Types::MQTTMsgQueue::PACKET_ENDPOINT_TYPE& endpointType
    );  // nullopt if no changes

    std::optional<std::string> processPropGetRepPacket(
        const std::string& fullPayloadRaw,
        const AikariPLS::Types::RuleSystem::RuleMapping::RewriteFeaturesStore&
            featureStore
    );  // only for server-side responses correspond to prev client-side
        // thing.property.get
}  // namespace AikariPLS::Utils::MQTTPacketUtils
