#pragma once

#include <Aikari-Shared/base/SingleIns.hpp>
#include <async_mqtt/protocol/packet/packet_id_type.hpp>
#include <string>
#include <unordered_map>

namespace AikariShared::Infrastructure::MessageQueue
{
    template <typename T>
    class SinglePointMessageQueue;
}

namespace AikariPLS::Types::MQTTMsgQueue
{
    struct FlaggedPacket;
}

namespace AikariPLS::Types::Lifecycle::MQTT
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
        typedef AikariShared::Infrastructure::MessageQueue::
            SinglePointMessageQueue<
                AikariPLS::Types::MQTTMsgQueue::FlaggedPacket>
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
}  // namespace AikariPLS::Types::Lifecycle::MQTT

namespace AikariPLS::Lifecycle::MQTT
{
    typedef AikariShared::Base::AikariStatesManagerTemplate<
        AikariPLS::Types::Lifecycle::MQTT::PLSMQTTRealClientInfo>
        PLSMQTTSharedRealClientInfo;

    typedef AikariShared::Base::AikariStatesManagerTemplate<
        AikariPLS::Types::Lifecycle::MQTT::PLSMQTTMsgQueues>
        PLSMQTTMsgQueues;
}  // namespace AikariPLS::Lifecycle::MQTT
