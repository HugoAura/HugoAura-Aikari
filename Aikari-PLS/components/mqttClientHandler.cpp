#include "mqttClientHandler.h"

#define CUSTOM_LOG_HEADER "[MQTT Client]"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>

#include "../lifecycle.h"
#include "../utils/mqttPacketUtils.h"
#include "mqttLifecycle.h"

namespace AikariPLS::Components::MQTTClient::Class
{
    MQTTClientConnection::MQTTClientConnection(
        std::function<void(const async_mqtt::packet_variant& packet)>
            onAscendLambda,
        std::function<void()> onCloseLambda,
        std::function<void(async_mqtt::error_code errCode)> onErrorLambda,
        std::function<void()> onConnAckSuccessLambda
    )
        : ClientConnection(async_mqtt::protocol_version::v3_1_1),
          onAscendLambda_(std::move(onAscendLambda)),
          onCloseLambda_(std::move(onCloseLambda)),
          onErrorLambda_(std::move(onErrorLambda)),
          onConnAckSuccessLambda_(std::move(onConnAckSuccessLambda))
    {
        this->set_auto_pub_response(true);
        // client dont need auto pingresp

        auto& sharedStates =
            AikariPLS::Lifecycle::PLSSharedStates::getInstance();
        this->isDebugEnv =
            (sharedStates.getVal(
                 &AikariPLS::Types::lifecycle::PLSSharedStates::runtimeMode
             ) == AikariLauncherPublic::Constants::Lifecycle::
                      APPLICATION_RUNTIME_MODES::DEBUG);
    };

    void MQTTClientConnection::checkTimerTimeout()
    {
        auto curTime = std::chrono::steady_clock::now();
        if (curTime > this->nextPingReqEtc &&
            this->nextPingReqEtc != std::chrono::steady_clock::time_point())
        {
            this->notify_timer_fired(async_mqtt::timer_kind::pingreq_send);
            this->nextPingReqEtc = {};
        }
    }

    void MQTTClientConnection::on_send(
        async_mqtt::packet_variant packet,
        std::optional<async_mqtt::packet_id_type>
            release_packet_id_if_send_error
    )
    {
        if (auto connectPkt =
                packet.get_if<async_mqtt::v3_1_1::connect_packet>();
            connectPkt != nullptr)
        {
            this->logSendPacket(*connectPkt);
            this->onAscendLambda_(packet);
            std::chrono::seconds sendInterval(connectPkt->keep_alive() / 3 * 2);
            CUSTOM_LOG_DEBUG("PINGREQ send interval will be {}", sendInterval);
            this->keepAliveDuration = sendInterval;
            this->set_pingreq_send_interval(sendInterval);
            this->set_pingresp_recv_timeout(std::chrono::milliseconds(0));
            return;
        }

        if (!isConnected)
        {
            this->pktTempStore.emplace_back(std::move(packet));
            return;
        }

        if (isConnected && !this->pktTempStore.empty())
        {
            for (auto pendingPkt : this->pktTempStore)
            {
                this->logSendPacket(packet);
                this->onAscendLambda_(pendingPkt);
            }
        }
        this->logSendPacket(packet);
        this->onAscendLambda_(packet);
    };

    void MQTTClientConnection::on_receive(async_mqtt::packet_variant packet)
    {
        auto& sharedMqttQueues =
            AikariPLS::Lifecycle::MQTT::PLSMQTTMsgQueues::getInstance();
        auto* clientToBrokerQueue =
            sharedMqttQueues.getPtr(&AikariPLS::Types::lifecycle::MQTT::
                                        PLSMQTTMsgQueues::clientToBrokerQueue);

        packet.visit(
            async_mqtt::overload{
                /// --- ↓ CONNACK ↓ --- ///
                [&](async_mqtt::v3_1_1::connack_packet const& pkt)
                {
                    auto ackCode = pkt.code();
                    if (ackCode == async_mqtt::connect_return_code::accepted)
                    {
                        if (!this->isConnected)
                        {
                            this->isConnected = true;
                        }
                        CUSTOM_LOG_DEBUG(
                            "{} Received CONNACK packet, status code eqeqeq 0, "
                            "successfully connected to broker.",
                            Constants::recvOper
                        );

                        this->onConnAckSuccessLambda_();
                    }
                    else
                    {
                        CUSTOM_LOG_ERROR(
                            "{} Failed connecting to Seewo MQTT broker, "
                            "CONNACK "
                            "returned code: {}",
                            Constants::recvOper,
                            static_cast<uint8_t>(ackCode)
                        );
                        if (this->isConnected)
                        {
                            this->isConnected = false;
                        }
                    }

                    AikariPLS::Types::mqttMsgQueue::FlaggedPacket
                        connackTransparentPassPkt = {
                            .type = AikariPLS::Types::mqttMsgQueue::
                                PACKET_OPERATION_TYPE::PKT_TRANSPARENT,
                            .packet = pkt
                        };

                    clientToBrokerQueue->push(
                        std::move(connackTransparentPassPkt)
                    );
                },
                /// --- ↑ CONNACK ↑ --- ///
                /// --- ↓ SUBACK ↓ --- ///
                [&](async_mqtt::v3_1_1::suback_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Received SUBACK packet.", Constants::recvOper
                    );

                    AikariPLS::Types::mqttMsgQueue::FlaggedPacket
                        subackTransparentPassPkt = {
                            .type = AikariPLS::Types::mqttMsgQueue::
                                PACKET_OPERATION_TYPE::PKT_TRANSPARENT,
                            .packet = pkt
                        };

                    clientToBrokerQueue->push(
                        std::move(subackTransparentPassPkt)
                    );
                },
                /// --- ↑ SUBACK ↑ --- ///
                /// --- ↓ UNSUBACK ↓ --- ///
                [&](async_mqtt::v3_1_1::unsuback_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Received UNSUBACK packet.", Constants::recvOper
                    );

                    AikariPLS::Types::mqttMsgQueue::FlaggedPacket
                        unsubackTransparentPassPkt = {
                            .type = AikariPLS::Types::mqttMsgQueue::
                                PACKET_OPERATION_TYPE::PKT_TRANSPARENT,
                            .packet = pkt
                        };

                    clientToBrokerQueue->push(
                        std::move(unsubackTransparentPassPkt)
                    );
                },
                /// --- ↑ UNSUBACK ↑ --- ///
                /// --- ↓ PUBLISH ↓ --- ///
                [&](async_mqtt::v3_1_1::publish_packet const& pkt)
                {
                    const auto publishOpts = pkt.opts();

                    CUSTOM_LOG_DEBUG(
                        "{} Received PUBLISH packet\n{}\n"
                        "Payload:\n{}"
                        "\n{}\n"
                        "Topic: {} | QoS: {} | Packet ID: {}",
                        Constants::recvOper,
                        Constants::dataHr,
                        pkt.payload(),
                        Constants::propHr,
                        pkt.topic(),
                        static_cast<std::uint8_t>(publishOpts.get_qos()),
                        pkt.packet_id()
                    );

                    auto packetProps =
                        AikariPLS::Utils::MQTTPacketUtils::getPacketProps(
                            pkt.topic()
                        );

                    if (packetProps.endpointType ==
                            AikariPLS::Types::mqttMsgQueue::
                                PACKET_ENDPOINT_TYPE::RPC &&
                        endpointRpcIgnoredIds.erase(packetProps.msgId.value()
                        ) != 0)
                    {
                        return;
                    }
                    if (packetProps.endpointType ==
                            AikariPLS::Types::mqttMsgQueue::
                                PACKET_ENDPOINT_TYPE::GET &&
                        endpointGetIgnoredIds.erase(packetProps.msgId.value()
                        ) != 0)
                    {
                        return;
                    }

                    // TODO: Run hooks

                    AikariPLS::Types::mqttMsgQueue::FlaggedPacket publishPkt = {
                        .type = AikariPLS::Types::mqttMsgQueue::
                            PACKET_OPERATION_TYPE::PKT_TRANSPARENT,
                        .packet = pkt,
                        .props = packetProps
                    };

                    clientToBrokerQueue->push(std::move(publishPkt));
                },
                /// --- ↑ PUBLISH ↑ --- ///
                /// --- ↓ PUBACK ↓ --- ///
                [&](async_mqtt::v3_1_1::puback_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Received PUBACK packet, packetId: {}",
                        Constants::recvOper,
                        pkt.packet_id()
                    );
                },
                /// --- ↑ PUBACK ↑ --- ///
                /// --- ↓ PINGRESP ↓ --- ///
                [&](async_mqtt::v3_1_1::pingresp_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Received PINGRESP packet.", Constants::recvOper
                    );
                    this->on_timer_op(
                        async_mqtt::timer_op::reset,
                        async_mqtt::timer_kind::pingreq_send,
                        this->keepAliveDuration
                    );
                },
                /// --- ↑ PINGRESP ↑ --- ///
                /// --- × DISCONNECT × --- ///
                /// in MQTT v3_1_1, broker wont send DISCONN pkt to client ///
                /// so no need to handle ///
                [](auto const&)
                {
                } }
        );
    }

    void MQTTClientConnection::on_close()
    {
        this->onCloseLambda_();
    }

    void MQTTClientConnection::on_error(async_mqtt::error_code errCode)
    {
        CUSTOM_LOG_ERROR(
            "An error occurred with connection between Fake Client <-> Real "
            "Broker, error: {}",
            errCode.message()
        );

        this->onErrorLambda_(errCode);
    }

    void MQTTClientConnection::on_packet_id_release(
        async_mqtt::packet_id_type packetId
    )
    {
#ifdef _DEBUG
        CUSTOM_LOG_TRACE("Packet ID released: {}", packetId);
#endif
    }

    void MQTTClientConnection::on_timer_op(
        async_mqtt::timer_op op,
        async_mqtt::timer_kind kind,
        std::optional<std::chrono::milliseconds> ms
    )
    {
#ifdef _DEBUG
        CUSTOM_LOG_TRACE(
            "Timer action triggerred, op {}, type {}, ms {}",
            static_cast<int>(op),
            static_cast<int>(kind),
            ms.value_or(std::chrono::milliseconds::zero())
        );
#endif
        switch (kind)
        {
            case (async_mqtt::timer_kind::pingreq_send):
            {
                switch (op)
                {
                    case async_mqtt::timer_op::reset:
                    {
                        this->nextPingReqEtc =
                            std::chrono::steady_clock::now() +
                            ms.value_or(std::chrono::seconds(0));
                        break;
                    }
                    case async_mqtt::timer_op::cancel:
                    {
                        this->nextPingReqEtc = {};
                    }
                }
                break;
            }

            default:
            {
                break;
            }
        }
    }

    async_mqtt::packet_id_type MQTTClientConnection::getPacketId()
    {
        const auto packetIdOption = this->acquire_unique_packet_id();
        if (packetIdOption.has_value())
        {
            return packetIdOption.value();
        }
        else
        {
            CUSTOM_LOG_WARN(
                "{} Warning: no free packet ID remains.", Constants::ascendOper
            );
            return static_cast<async_mqtt::packet_id_type>(0);
        }
    }

    void MQTTClientConnection::logSendPacket(
        const async_mqtt::packet_variant& packet
    ) const
    {
        if (!isDebugEnv)
            return;
        packet.visit(
            // Some packets will never be overridden, so only log basic pkt type
            async_mqtt::overload{
                [&](async_mqtt::v3_1_1::connect_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Sending CONNECT packet", Constants::ascendOper
                    );
                },
                [&](async_mqtt::v3_1_1::publish_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Sending PUBLISH packet\n{}\n"
                        "Payload:\n{}"
                        "\n{}\n"
                        "Topic: {} | QoS: {} | Packet ID: {}",
                        Constants::ascendOper,
                        Constants::dataHr,
                        pkt.payload(),
                        Constants::propHr,
                        pkt.topic(),
                        static_cast<std::uint8_t>(pkt.opts().get_qos()),
                        pkt.packet_id()
                    );
                },
                [&](async_mqtt::v3_1_1::subscribe_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Sending SUBSCRIBE packet", Constants::ascendOper
                    );
                },
                [&](async_mqtt::v3_1_1::unsubscribe_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Sending UNSUBSCRIBE packet", Constants::ascendOper
                    );
                },
                [&](async_mqtt::v3_1_1::disconnect_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Sending DISCONNECT packet", Constants::ascendOper
                    );
                },

                [&](async_mqtt::v3_1_1::pingreq_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Sending PINGREQ packet", Constants::ascendOper
                    );
                },
                [](auto const&)
                {
                } }
        );
    }

}  // namespace AikariPLS::Components::MQTTClient::Class
