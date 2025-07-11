#include "mqttBrokerHandler.h"

#include <Aikari-PLS-Private/types/constants/mqtt.h>
#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <functional>

namespace mqttConstants = AikariPLS::Types::PrivateConstants::MQTT;

namespace AikariPLS::Components::MQTTBroker::Class
{
    MQTTBrokerConnection::MQTTBrokerConnection(
        std::function<void(async_mqtt::packet_variant packet)> onSendLambda,
        std::function<void()> onCloseLambda,
        std::function<void(async_mqtt::error_code errCode)> onErrLambda
    )
        : BrokerConnection(async_mqtt::protocol_version::v3_1_1),
          onSendLambda_(onSendLambda),
          onCloseLambda_(onCloseLambda),
          onErrorLambda_(onErrLambda)
    {
        this->set_auto_ping_response(true);
        this->set_auto_pub_response(true);
    };

    void MQTTBrokerConnection::on_send(
        async_mqtt::packet_variant packet,
        std::optional<async_mqtt::packet_id_type>
            release_packet_id_if_send_error
    )
    {
        this->onSendLambda_(std::move(packet));
    };

    void MQTTBrokerConnection::on_receive(async_mqtt::packet_variant packet)
    {
        packet.visit(async_mqtt::overload{
            [&](async_mqtt::v3_1_1::connect_packet const& pkt)
            {
                LOG_DEBUG(
                    "{} Received CONNECT packet\n{}\n"
                    "Client ID: {} | UserName: {} | Password: {}",
                    logHeader,
                    dataHr,
                    pkt.client_id(),
                    pkt.user_name().value_or("UNKNOWN"),
                    pkt.password().value_or("UNKNOWN")
                );
                async_mqtt::v3_1_1::connack_packet connAckPkt(
                    true,
                    (async_mqtt::connect_return_code
                    )mqttConstants::Broker::CONNACK::CONN_ACCEPT
                );
                this->send(connAckPkt);
            },
            [](auto const&)
            {
            } });
    };

    void MQTTBrokerConnection::on_close()
    {
        this->onCloseLambda_();
    };

    void MQTTBrokerConnection::on_error(async_mqtt::error_code errCode)
    {
        LOG_ERROR(
            "An error occurred with MQTT connection, error code: {}",
            errCode.message()
        );
        this->onErrorLambda_(std::move(errCode));
    };

    void MQTTBrokerConnection::on_packet_id_release(
        async_mqtt::packet_id_type packetId
    )
    {
#ifdef _DEBUG
        LOG_TRACE("Packet ID released: {}", packetId);
#endif
    };

    void MQTTBrokerConnection::on_timer_op(
        async_mqtt::timer_op op,
        async_mqtt::timer_kind kind,
        std::optional<std::chrono::milliseconds> ms
    ) {};
}  // namespace AikariPLS::Components::MQTTBroker::Class
