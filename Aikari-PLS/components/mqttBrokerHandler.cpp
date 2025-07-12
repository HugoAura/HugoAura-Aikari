#include "mqttBrokerHandler.h"

#include <Aikari-PLS-Private/types/constants/mqtt.h>
#include <Aikari-PLS/types/constants/init.h>
#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <functional>

#include "mqttClient.h"

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
        packet.visit(
            async_mqtt::overload{
                /// --- ↓ CONNECT ↓ --- ///
                //| Transparently forward & Rep CONNACK |//
                [&](async_mqtt::v3_1_1::connect_packet const& pkt)
                {
                    auto username = pkt.user_name().value_or("UNKNOWN");
                    auto password = pkt.password().value_or("UNKNOWN");

                    LOG_DEBUG(
                        "{} Received CONNECT packet\n{}\n"
                        "Client ID: {} | UserName: {} | Password: {}",
                        logHeader,
                        dataHr,
                        pkt.client_id(),
                        username,
                        password
                    );

                    this->clientId = pkt.client_id();

                    AikariPLS::Components::MQTTClient::ClientLaunchArg
                        clientLaunchArg = { .targetHost =
                                                AikariPLS::Types::constants::
                                                    init::networkInit::HOSTNAME,
                                            .targetPort =
                                                AikariPLS::Types::constants::
                                                    init::networkInit::PORT,

                                            .clientId = this->clientId,
                                            .username = username,
                                            .password = password,
                                            .keepAliveSec = pkt.keep_alive() };

                    // Init MQTT client
                    AikariPLS::Components::MQTTClient::
                        ClientLifecycleController::initAndMountClientIns(
                            std::move(clientLaunchArg)
                        );

                    const async_mqtt::v3_1_1::connack_packet connAckPkt(
                        true,
                        (async_mqtt::connect_return_code
                        )mqttConstants::Broker::CONNACK::CONN_ACCEPT
                    );
                    this->send(std::move(connAckPkt));
                },
                /// --- ↑ CONNECT ↑ --- ///
                /// --- ↓ SUBSCRIBE ↓ --- ///
                //| Transparently forward & Rep SUBACK |//
                [&](async_mqtt::v3_1_1::subscribe_packet const& pkt)
                {
                    const auto subOpts = pkt.entries();

                    std::vector<async_mqtt::suback_return_code> ackReturns;
#ifdef _DEBUG
                    std::string opts("");
#endif
                    for (const auto& opt : subOpts)
                    {
#ifdef _DEBUG
                        opts += opt.all_topic() + "\n";
#endif
                        ackReturns.emplace_back(
                            async_mqtt::suback_return_code::
                                success_maximum_qos_2
                        );
                    }
#ifdef _DEBUG
                    LOG_TRACE(
                        "{} Received SUBSCRIBE packet\n{}\n"
                        "Topics:\n{}"  // No need for \n, opts already have one
                        "{}\n"
                        "Packet ID: {} | Client ID: {}",
                        logHeader,
                        dataHr,
                        opts,
                        propHr,
                        pkt.packet_id(),
                        this->clientId
                    );
#endif
                    async_mqtt::v3_1_1::suback_packet subAckPkt(
                        pkt.packet_id(), std::move(ackReturns)
                    );

                    this->send(std::move(subAckPkt));
                },
                /// --- ↑ SUBSCRIBE ↑ --- ///
                /// --- ↓ UNSUBSCRIBE ↓ --- ///
                //| Transparently forward & Rep UNSUBACK |//
                [&](async_mqtt::v3_1_1::unsubscribe_packet const& pkt)
                {
#ifdef _DEBUG
                    const auto unsubOpts = pkt.entries();
                    std::string opts("");
                    for (const auto& opt : unsubOpts)
                    {
                        opts += opt.all_topic() + "\n";
                    }
                    LOG_TRACE(
                        "{} Received UNSUBSCRIBE packet\n{}\n"
                        "Topics:\n{}"
                        "{}\n"
                        "Packet ID: {} | Client ID: {}",
                        logHeader,
                        dataHr,
                        opts,
                        propHr,
                        pkt.packet_id(),
                        this->clientId
                    );
#endif
                    async_mqtt::v3_1_1::unsuback_packet unsubAckPkt(
                        pkt.packet_id()
                    );

                    this->send(std::move(unsubAckPkt));
                },
                /// --- ↑ UNSUBSCRIBE ↑ --- ///
                /// --- ↓ PUBLISH ↓ --- ///
                [&](async_mqtt::v3_1_1::publish_packet const& pkt)
                {
                    const auto publishOpts = pkt.opts();

                    LOG_DEBUG(
                        "{} Received PUBLISH packet\n{}\n"
                        "Payload:\n{}"
                        "\n{}\n"
                        "Topic: {} | QoS: {} | Packet ID: {} | Client ID: {}",
                        logHeader,
                        dataHr,
                        pkt.payload(),
                        propHr,
                        pkt.topic(),
                        static_cast<std::uint8_t>(publishOpts.get_qos()),
                        pkt.packet_id(),
                        this->clientId
                    );

                    // No need for ack, auto reply is ON
                },
                /// --- ↑ PUBLISH ↑ --- ///
                /// --- ↓ DISCONNECT ↓ --- ///
                [&](async_mqtt::v3_1_1::disconnect_packet const& pkt)
                {
                    LOG_DEBUG(
                        "{} Received DISCONNECT packet\n"
                        "Closing connection for client {}, bye...",
                        logHeader,
                        this->clientId
                    );
                },
                /// --- ↑ DISCONNECT ↑ --- ///
                [](auto const&)
                {
                } }
        );
    };

    void MQTTBrokerConnection::on_close()
    {
        AikariPLS::Components::MQTTClient::ClientLifecycleController::
            resetClientIns(false);

        this->onCloseLambda_();
    };

    void MQTTBrokerConnection::on_error(async_mqtt::error_code errCode)
    {
        LOG_ERROR(
            "{} An error occurred with MQTT connection, error code: {}",
            logHeader,
            errCode.message()
        );
        this->onErrorLambda_(std::move(errCode));
    };

    void MQTTBrokerConnection::on_packet_id_release(
        async_mqtt::packet_id_type packetId
    )
    {
#ifdef _DEBUG
        LOG_TRACE("{} Packet ID released: {}", logHeader, packetId);
#endif
    };

    void MQTTBrokerConnection::on_timer_op(
        async_mqtt::timer_op op,
        async_mqtt::timer_kind kind,
        std::optional<std::chrono::milliseconds> ms
    ) {};

    async_mqtt::packet_id_type MQTTBrokerConnection::getPacketId()
    {
        const auto packetIdOption = this->acquire_unique_packet_id();
        if (packetIdOption.has_value())
        {
            return packetIdOption.value();
        }
        else
        {
            LOG_WARN(
                "{} Warning: no free packet ID remains.",
                logHeader
            );  // no need for retry, 因为现在这个场景中几乎不可能触发这种情况
            return static_cast<async_mqtt::packet_id_type>(0);
        }
    }
}  // namespace AikariPLS::Components::MQTTBroker::Class
