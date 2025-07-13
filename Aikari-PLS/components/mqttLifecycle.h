#pragma once

#include <Aikari-Shared/base/SingleIns.hpp>
#include <async_mqtt/protocol/packet/packet_id_type.hpp>
#include <string>
#include <unordered_map>

namespace AikariShared::infrastructure::MessageQueue
{
    template <typename T>
    class SinglePointMessageQueue;
}

namespace AikariPLS::Types::mqttMsgQueue
{
    struct FlaggedPacket;
}

namespace AikariPLS::Types::lifecycle::MQTT
{
    struct PLSMQTTRealClientInfo
    {
        bool isInitialized = false;
        std::string productKey;
        std::string deviceId;

        static PLSMQTTRealClientInfo createDefault()
        {
            return {};
        };
    };

    struct PLSMQTTMsgQueues
    {
        typedef AikariShared::infrastructure::MessageQueue::
            SinglePointMessageQueue<
                AikariPLS::Types::mqttMsgQueue::FlaggedPacket>
                FlaggedPacketQueue;

        std::unique_ptr<FlaggedPacketQueue> brokerToClientQueue;
        std::unique_ptr<FlaggedPacketQueue> clientToBrokerQueue;

        std::unordered_map<
            async_mqtt::packet_id_type,
            async_mqtt::packet_id_type>
            packetIdMap;

        PLSMQTTMsgQueues();
        static PLSMQTTMsgQueues createDefault();
        ~PLSMQTTMsgQueues();

        PLSMQTTMsgQueues(PLSMQTTMsgQueues&&) noexcept;
        PLSMQTTMsgQueues& operator=(PLSMQTTMsgQueues&&) noexcept;
    };
}  // namespace AikariPLS::Types::lifecycle::MQTT

namespace AikariPLS::Lifecycle::MQTT
{
    typedef AikariShared::base::AikariStatesManagerTemplate<
        AikariPLS::Types::lifecycle::MQTT::PLSMQTTRealClientInfo>
        PLSMQTTSharedRealClientInfo;

    typedef AikariShared::base::AikariStatesManagerTemplate<
        AikariPLS::Types::lifecycle::MQTT::PLSMQTTMsgQueues>
        PLSMQTTMsgQueues;
}  // namespace AikariPLS::Lifecycle::MQTT
