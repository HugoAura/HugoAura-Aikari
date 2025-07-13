#pragma once

#include <async_mqtt/protocol/packet/packet_variant.hpp>

namespace AikariPLS::Utils::MQTTPacketUtils
{
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

    async_mqtt::packet_variant reconstructPacketWithPktId(
        async_mqtt::packet_variant& oldPacket,
        std::function<async_mqtt::packet_id_type()>& buildNewPacketIdFn
    );
}  // namespace AikariPLS::Utils::MQTTPacketUtils
