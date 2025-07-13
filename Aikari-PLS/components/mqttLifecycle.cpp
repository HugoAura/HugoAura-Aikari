#include "mqttLifecycle.h"

#include <Aikari-PLS-Private/types/components/mqtt.h>
#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>

namespace AikariPLS::Types::lifecycle::MQTT
{
    PLSMQTTMsgQueues::PLSMQTTMsgQueues() = default;
    PLSMQTTMsgQueues PLSMQTTMsgQueues::createDefault()
    {
        PLSMQTTMsgQueues returnResult;
        returnResult.brokerToClientQueue =
            std::make_unique<FlaggedPacketQueue>();
        returnResult.clientToBrokerQueue =
            std::make_unique<FlaggedPacketQueue>();
        returnResult.packetIdMap = std::unordered_map<
            async_mqtt::packet_id_type,
            async_mqtt::packet_id_type>();

        return returnResult;
    };
    PLSMQTTMsgQueues::~PLSMQTTMsgQueues()
    {
        LOG_TRACE("Destroying PLS MQTT message queues...");
        this->brokerToClientQueue.reset();
        this->clientToBrokerQueue.reset();
    };

    PLSMQTTMsgQueues::PLSMQTTMsgQueues(PLSMQTTMsgQueues&&) noexcept = default;
    PLSMQTTMsgQueues& PLSMQTTMsgQueues::operator=(PLSMQTTMsgQueues&&
    ) noexcept = default;
}  // namespace AikariPLS::Types::lifecycle::MQTT
