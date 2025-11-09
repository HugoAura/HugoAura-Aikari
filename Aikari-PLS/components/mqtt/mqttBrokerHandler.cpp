#include "./mqttBrokerHandler.h"

#define CUSTOM_LOG_HEADER "[MQTT Broker]"

#include <Aikari-PLS-Private/types/constants/mqtt.h>
#include <Aikari-PLS/types/constants/init.h>
#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>
#include <Aikari-Shared/utils/string.h>
#include <functional>

#include "../../../Aikari-Launcher/lifecycle.h"
#include "../../lifecycle.h"
#include "../../utils/mqttPacketUtils.h"
#include "mqttClient.h"
#include "mqttLifecycle.h"

/* This component handles <messages> between [Real Client] <-> [Fake Broker] */

namespace mqttConstants = AikariPLS::Types::PrivateConstants::MQTT;

namespace AikariPLS::Components::MQTTBroker::Class
{
    MQTTBrokerConnection::MQTTBrokerConnection(
        std::function<void(async_mqtt::packet_variant packet)> onSendLambda,
        std::function<void()> onCloseLambda,
        std::function<void(async_mqtt::error_code errCode)> onErrLambda
    )
        : BrokerConnection(async_mqtt::protocol_version::v3_1_1),
          onSendLambda_(std::move(onSendLambda)),
          onCloseLambda_(std::move(onCloseLambda)),
          onErrorLambda_(std::move(onErrLambda))
    {
        this->set_auto_ping_response(true);
        this->set_auto_pub_response(true);

        auto& sharedStates =
            AikariPLS::Lifecycle::PLSSharedStates::getInstance();
        this->isDebugEnv =
            (sharedStates.getVal(
                 &AikariPLS::Types::Lifecycle::PLSSharedStates::runtimeMode
             ) == AikariLauncherPublic::Constants::Lifecycle::
                      APPLICATION_RUNTIME_MODES::DEBUG);
    };

    void MQTTBrokerConnection::on_send(
        async_mqtt::packet_variant packet,
        std::optional<async_mqtt::packet_id_type>
            release_packet_id_if_send_error
    )
    {
        this->logSendPacket(packet);

        this->onSendLambda_(std::move(packet));
    };

    void MQTTBrokerConnection::on_receive(async_mqtt::packet_variant packet)
    {
        auto& sharedMqttQueues =
            AikariPLS::Lifecycle::MQTT::PLSMQTTMsgQueues::getInstance();
        auto* brokerToClientQueue =
            sharedMqttQueues.getPtr(&AikariPLS::Types::Lifecycle::MQTT::
                                        PLSMQTTMsgQueues::brokerToClientQueue);
        auto* selfQueue =
            sharedMqttQueues.getPtr(&AikariPLS::Types::Lifecycle::MQTT::
                                        PLSMQTTMsgQueues::clientToBrokerQueue);

        packet.visit(
            async_mqtt::overload{
                /// --- ↓ CONNECT ↓ --- ///
                //| Transparently forward & Rep CONNACK |//
                [&](async_mqtt::v3_1_1::connect_packet const& pkt)
                {
                    auto username = pkt.user_name().value_or("UNKNOWN");
                    auto password = pkt.password().value_or("UNKNOWN");

                    CUSTOM_LOG_DEBUG(
                        "{} Received CONNECT packet\n{}\n"
                        "Client ID: {} | UserName: {} | Password: {}",
                        Constants::recvOper,
                        Constants::dataHr,
                        pkt.client_id(),
                        username,
                        password
                    );

                    this->clientId = pkt.client_id();

                    AikariPLS::Components::MQTTClient::ClientLaunchArg
                        clientLaunchArg = { .targetHost =
                                                AikariPLS::Types::Constants::
                                                    Init::NetworkInit::HOSTNAME,
                                            .targetPort =
                                                AikariPLS::Types::Constants::
                                                    Init::NetworkInit::PORT,

                                            .clientId = this->clientId,
                                            .username = username,
                                            .password = password,
                                            .keepAliveSec = pkt.keep_alive() };

                    // Init shared client info: deviceId
                    auto& sharedClientInfo = AikariPLS::Lifecycle::MQTT::
                        PLSMQTTSharedRealClientInfo::getInstance();
                    sharedClientInfo.setVal(
                        &AikariPLS::Types::Lifecycle::MQTT::
                            PLSMQTTRealClientInfo::deviceId,
                        this->clientId
                    );

                    // Init MQTT client
                    AikariPLS::Components::MQTTClient::
                        ClientLifecycleController::initAndMountClientIns(
                            std::move(clientLaunchArg)
                        );

                    // dispatch
                    /*
                    async_mqtt::v3_1_1::connack_packet connAckPkt(
                        true,
                        (async_mqtt::connect_return_code
                        )mqttConstants::Broker::CONNACK::CONN_ACCEPT
                    );
                    this->send(std::move(connAckPkt));
                    */

                    // Wait for client's CONNACK
                },
                /// --- ↑ CONNECT ↑ --- ///
                /// --- ↓ SUBSCRIBE ↓ --- ///
                //| Transparently forward & Rep SUBACK |//
                [&](async_mqtt::v3_1_1::subscribe_packet const& pkt)
                {
                    const auto& subOpts = pkt.entries();

                    std::vector<async_mqtt::suback_return_code> ackReturns;
#ifdef _DEBUG
                    std::string opts;
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
                    CUSTOM_LOG_TRACE(
                        "{} Received SUBSCRIBE packet\n{}\n"
                        "Topics:\n{}"  // No need for \n, opts already have one
                        "{}\n"
                        "Packet ID: {} | Client ID: {}",
                        Constants::recvOper,
                        Constants::dataHr,
                        opts,
                        Constants::propHr,
                        pkt.packet_id(),
                        this->clientId
                    );
#endif

                    // Init shared client info: productKey && deviceId
                    if (!this->isSharedInfoInitialized)
                    // ↑ this variable is used for preventing exec getVal on
                    // SharedRealClientInfo every time onRecv (mutex perf cost)
                    {
                        auto& sharedClientInfo = AikariPLS::Lifecycle::MQTT::
                            PLSMQTTSharedRealClientInfo::getInstance();

                        if (auto isInitialized = sharedClientInfo.getVal(
                                &AikariPLS::Types::Lifecycle::MQTT::
                                    PLSMQTTRealClientInfo::isInitialized
                            );
                            !isInitialized)
                        {
                            std::string sampleTopic = subOpts[0].all_topic();
                            try
                            {
                                auto resolveResult =
                                    AikariPLS::Utils::MQTTPacketUtils::
                                        getPacketProps(sampleTopic);
                                sharedClientInfo.setVal(
                                    &AikariPLS::Types::Lifecycle::MQTT::
                                        PLSMQTTRealClientInfo::productKey,
                                    resolveResult.productKey
                                );
                                sharedClientInfo.setVal(
                                    &AikariPLS::Types::Lifecycle::MQTT::
                                        PLSMQTTRealClientInfo::deviceId,
                                    resolveResult.deviceId
                                );
                                sharedClientInfo.setVal(
                                    &AikariPLS::Types::Lifecycle::MQTT::
                                        PLSMQTTRealClientInfo::isInitialized,
                                    true
                                );
                                this->isSharedInfoInitialized = true;
                            }
                            catch (const std::exception& e)
                            {
                                CUSTOM_LOG_WARN(
                                    "Failed to update shared client info, "
                                    "error: {}",
                                    e.what()
                                );
                            }
                        }
                    }

                    AikariPLS::Types::MQTTMsgQueue::FlaggedPacket
                        subscribeTransparentPassPkt = {
                            .type = Types::MQTTMsgQueue::PACKET_OPERATION_TYPE::
                                PKT_TRANSPARENT,
                            .packet = pkt
                        };

                    brokerToClientQueue->push(
                        std::move(subscribeTransparentPassPkt)
                    );

                    /*
                    async_mqtt::v3_1_1::suback_packet subAckPkt(
                        pkt.packet_id(), std::move(ackReturns)
                    );

                    this->send(std::move(subAckPkt));
                    */
                },
                /// --- ↑ SUBSCRIBE ↑ --- ///
                /// --- ↓ UNSUBSCRIBE ↓ --- ///
                //| Transparently forward & Rep UNSUBACK |//
                [&](async_mqtt::v3_1_1::unsubscribe_packet const& pkt)
                {
#ifdef _DEBUG
                    const auto& unsubOpts = pkt.entries();
                    std::string opts;
                    for (const auto& opt : unsubOpts)
                    {
                        opts += opt.all_topic() + "\n";
                    }
                    CUSTOM_LOG_TRACE(
                        "{} Received UNSUBSCRIBE packet\n{}\n"
                        "Topics:\n{}"
                        "{}\n"
                        "Packet ID: {} | Client ID: {}",
                        Constants::recvOper,
                        Constants::dataHr,
                        opts,
                        Constants::propHr,
                        pkt.packet_id(),
                        this->clientId
                    );
#endif
                    async_mqtt::v3_1_1::unsuback_packet unsubAckPkt(
                        pkt.packet_id()
                    );

                    AikariPLS::Types::MQTTMsgQueue::FlaggedPacket
                        unsubscribeTransparentPassPkt = {
                            .type = Types::MQTTMsgQueue::PACKET_OPERATION_TYPE::
                                PKT_TRANSPARENT,
                            .packet = pkt
                        };

                    brokerToClientQueue->push(
                        std::move(unsubscribeTransparentPassPkt)
                    );

                    this->send(std::move(unsubAckPkt));
                },
                /// --- ↑ UNSUBSCRIBE ↑ --- ///
                /// --- ↓ PUBLISH ↓ --- ///
                [&](async_mqtt::v3_1_1::publish_packet const& pkt)
                {
                    const auto publishOpts = pkt.opts();

                    CUSTOM_LOG_DEBUG(
                        "{} Received PUBLISH packet\n{}\n"
                        "Payload:\n{}"
                        "\n{}\n"
                        "Topic: {} | QoS: {} | Packet ID: {} | Client ID: {}",
                        Constants::recvOper,
                        Constants::dataHr,
                        pkt.payload(),
                        Constants::propHr,
                        pkt.topic(),
                        static_cast<std::uint8_t>(publishOpts.get_qos()),
                        pkt.packet_id(),
                        this->clientId
                    );

                    auto packetProps =
                        AikariPLS::Utils::MQTTPacketUtils::getPacketProps(
                            pkt.topic()
                        );

                    // No need for ack, auto reply is ON

                    auto& sharedIns = AikariPLS::Lifecycle::
                        PLSSharedInsManager::getInstance();
                    auto* ruleManagerPtr = sharedIns.getPtr(
                        &AikariPLS::Types::Lifecycle::PLSSharedIns::ruleMgr
                    );

                    bool transparentPass = true;

                    std::string prevPayload{ pkt.payload() };
                    auto result = AikariPLS::Utils::MQTTPacketUtils::
                        processPacketDataWithRule(
                            prevPayload,
                            ruleManagerPtr->ruleMapping.client2broker.rewrite,
                            packetProps.endpointType
                        );
                    if (result.packetMethodType ==
                        AikariPLS::Utils::MQTTPacketUtils::
                            RewritePacketMethodType::PROP_GET)
                    {
                        this->flaggedGetMsgIds.insert(
                            packetProps.msgId.value_or("%UNKNOWN%")
                        );
                    }
                    if (result.newPayload.has_value())
                    {
                        switch (packetProps.endpointType)
                        {
                            case AikariPLS::Types::MQTTMsgQueue::
                                PACKET_ENDPOINT_TYPE::GET:
                            {
                                transparentPass = false;

                                packetProps.side = AikariPLS::Types::
                                    MQTTMsgQueue::PACKET_SIDE::REP;
                                std::string newTopicName = AikariPLS::Utils::
                                    MQTTPacketUtils::mergeTopic(packetProps);

                                auto newPkt =
                                    async_mqtt::v3_1_1::publish_packet(
                                        this->acquire_unique_packet_id()
                                            .value_or(
                                                0
                                            ),  // TODO: Error handling
                                        newTopicName,
                                        result.newPayload.value_or(
                                            pkt.payload()
                                        ),
                                        pkt.opts()
                                    );
                                AikariPLS::Types::MQTTMsgQueue::FlaggedPacket
                                    virtualPubBack = {
                                        .type = AikariPLS::Types::MQTTMsgQueue::
                                            PACKET_OPERATION_TYPE::PKT_VIRTUAL,
                                        .packet = newPkt,
                                        .props = std::move(packetProps)
                                    };
                                selfQueue->push(
                                    std::move(virtualPubBack)
                                );  // send to SELF
                                break;
                            }
                            case AikariPLS::Types::MQTTMsgQueue::
                                PACKET_ENDPOINT_TYPE::POST:
                            {
                                transparentPass = false;

                                AikariPLS::Types::MQTTMsgQueue::FlaggedPacket
                                    modifiedPubSend = {
                                        .type = AikariPLS::Types::MQTTMsgQueue::
                                            PACKET_OPERATION_TYPE::PKT_MODIFIED,
                                        .packet = pkt,
                                        .props = std::move(packetProps),
                                        .newPayload =
                                            std::move(result.newPayload)
                                    };
                                brokerToClientQueue->push(
                                    std::move(modifiedPubSend)
                                );  // send to FAKE CLIENT
                                break;
                            }
                            case AikariPLS::Types::MQTTMsgQueue::
                                PACKET_ENDPOINT_TYPE::RPC:
                            case AikariPLS::Types::MQTTMsgQueue::
                                PACKET_ENDPOINT_TYPE::UNKNOWN:
                            {
                                CUSTOM_LOG_WARN(
                                    "Received unrecognizable packet "
                                    "endpointType {} from real client, please "
                                    "report this to Aikari "
                                    "GitHub Issues.",
                                    AikariPLS::Types::MQTTMsgQueue::to_string(
                                        packetProps.endpointType
                                    )
                                );
                                break;
                            }
                        }
                    }

                    if (transparentPass)
                    {
                        AikariPLS::Types::MQTTMsgQueue::FlaggedPacket
                            publishPkt = {
                                .type = Types::MQTTMsgQueue::
                                    PACKET_OPERATION_TYPE::PKT_TRANSPARENT,
                                .packet = pkt,
                                .props = std::move(packetProps)
                            };

                        brokerToClientQueue->push(std::move(publishPkt));
                    }
                },
                /// --- ↑ PUBLISH ↑ --- ///
                /// --- ↓ DISCONNECT ↓ --- ///
                [&](async_mqtt::v3_1_1::disconnect_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Received DISCONNECT packet\n"
                        "Closing connection for client {}, bye...",
                        Constants::recvOper,
                        this->clientId
                    );

                    AikariPLS::Types::MQTTMsgQueue::FlaggedPacket
                        disconnectPkt = {
                            .type = Types::MQTTMsgQueue::PACKET_OPERATION_TYPE::
                                PKT_TRANSPARENT,
                            .packet = pkt
                        };

                    brokerToClientQueue->push(std::move(disconnectPkt));
                },
                /// --- ↑ DISCONNECT ↑ --- ///
                [](auto const&)
                {
                } }
        );
    };

    void MQTTBrokerConnection::on_close()
    {
        this->isSharedInfoInitialized = false;
        auto& sharedClientInfo = AikariPLS::Lifecycle::MQTT::
            PLSMQTTSharedRealClientInfo::getInstance();
        sharedClientInfo.setVal(
            &AikariPLS::Types::Lifecycle::MQTT::PLSMQTTRealClientInfo::
                isInitialized,
            false
        );

        AikariPLS::Components::MQTTClient::ClientLifecycleController::
            resetClientIns(false);

        this->onCloseLambda_();
    };

    void MQTTBrokerConnection::on_error(async_mqtt::error_code errCode)
    {
        CUSTOM_LOG_ERROR(
            "An error occurred with connection between Real Client <-> Fake "
            "Broker, error: {}",
            errCode.message()
        );
        this->onErrorLambda_(std::move(errCode));
    };

    void MQTTBrokerConnection::on_packet_id_release(
        async_mqtt::packet_id_type packetId
    )
    {
#ifdef _DEBUG
        CUSTOM_LOG_TRACE("Packet ID released: {}", packetId);
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
            CUSTOM_LOG_WARN(
                "{} Warning: no free packet ID remains.",
                Constants::ascendOper
            );  // no need for retry, 因为现在这个场景中几乎不可能触发这种情况
            return static_cast<async_mqtt::packet_id_type>(0);
        }
    }

    void MQTTBrokerConnection::logSendPacket(
        const async_mqtt::packet_variant& packet
    ) const
    {
        if (!isDebugEnv)
            return;
        packet.visit(
            async_mqtt::overload{
                [&](async_mqtt::v3_1_1::connack_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Replying CONNACK packet", Constants::ascendOper
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
                /*
                [&](async_mqtt::v3_1_1::suback_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Sending SUBACK packet", Constants::ascendOper
                    );
                },
                [&](async_mqtt::v3_1_1::unsuback_packet const& pkt)
                {
                    CUSTOM_LOG_DEBUG(
                        "{} Sending UNSUBACK packet", Constants::ascendOper
                    );
                }
                */
                [](auto const&)
                {
                } }
        );
    }
}  // namespace AikariPLS::Components::MQTTBroker::Class
