#include "./mqttClient.h"

#define CUSTOM_LOG_HEADER "[MQTT Client]"

#include <Aikari-PLS/types/constants/mqtt.h>
#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>
#include <Aikari-Shared/infrastructure/telemetry.h>
#include <Aikari-Shared/infrastructure/telemetryShortFn.h>
#include <Aikari-Shared/utils/network.h>
#include <Aikari-Shared/utils/windows.h>
#include <exception>
#include <winsock2.h>

#include "../../lifecycle.h"
#include "../../resource.h"
#include "../../utils/mqttPacketUtils.h"
#include "mqttLifecycle.h"

#define TELEMETRY_ACTION_CATEGORY "pls.mqtt.client"
#define TELEMETRY_MODULE_NAME "PLS - MQTT Client"

/*
 * This component handles <connection> between [Fake Client] <-> [Real Broker]
 */

namespace AikariPLS::Components::MQTTClient
{
    Client::Client(const ClientLaunchArg& arg) : launchArg(arg)
    {
        bool getIpResult = this->refreshHostRealIP();
        if (!getIpResult)
        {
            CUSTOM_LOG_ERROR(
                "Failed to get the real IP of {}, exiting Client...",
                arg.targetHost
            );
            return;
        }

        this->clientLoop =
            std::make_unique<std::jthread>(&Client::startClientLoop, this);

        this->initSendThreadPool();
        this->sendQueueWorker =
            std::make_unique<std::jthread>(&Client::startSendQueueWorker, this);
    };

    Client::~Client()
    {
        this->cleanUp(false);
    };

    void Client::cleanUp(bool ignoreThreadJoin)
    {
        if (this->cleaned)
        {
            return;
        }

        Telemetry::addBreadcrumb(
            "default",
            std::format(
                "Cleaning up MQTT Client context, ignoreThreadJoin?: {}",
                ignoreThreadJoin ? "true" : "false"
            ),
            TELEMETRY_ACTION_CATEGORY,
            "info"
        );
        CUSTOM_LOG_DEBUG("Cleaning up MQTT Client context...");
        this->cleaned = true;
        this->shouldExit = true;

        auto& mqttSharedQueues =
            AikariPLS::Lifecycle::MQTT::PLSMQTTMsgQueues::getInstance();
        auto* sendQueuePtr =
            mqttSharedQueues.getPtr(&AikariPLS::Types::Lifecycle::MQTT::
                                        PLSMQTTMsgQueues::brokerToClientQueue);

        if (sendQueuePtr != nullptr)
        {
            sendQueuePtr->push(
                {
                    .type = Types::MQTTMsgQueue::PACKET_OPERATION_TYPE::
                        CTRL_THREAD_END,
                }
            );
        }

        if (this->clientLoop->joinable() && !ignoreThreadJoin)
        {
            this->clientLoop->join();
        }
        if (this->sendQueueWorker->joinable() && !ignoreThreadJoin)
        {
            this->sendQueueWorker->join();
        }

        std::lock_guard<std::mutex> lock(this->sslCtxLock);
        mbedtls_net_free(&this->netCtxs.serverFd);
        mbedtls_x509_crt_free(&this->mbedCtx.caCrt);
        mbedtls_ssl_free(&this->sslCtx);
        mbedtls_ssl_config_free(&this->mbedCtx.sslConfig);
        mbedtls_ctr_drbg_free(&this->mbedCtx.ctrDrbg);
        mbedtls_entropy_free(&this->mbedCtx.entropyCtx);
    };

    void Client::startClientLoop()
    {
        Telemetry::addBreadcrumb(
            "default",
            "MQTT Client is starting",
            TELEMETRY_ACTION_CATEGORY,
            "debug"
        );
        try
        {
            int taskTempRet = 0;

            mbedtls_net_init(&this->netCtxs.serverFd);
            mbedtls_ssl_init(&this->sslCtx);
            mbedtls_ssl_config_init(&this->mbedCtx.sslConfig);
            mbedtls_x509_crt_init(&this->mbedCtx.caCrt);
            mbedtls_entropy_init(&this->mbedCtx.entropyCtx);
            mbedtls_ctr_drbg_init(&this->mbedCtx.ctrDrbg);

            CUSTOM_LOG_INFO("MQTT Client TLS context init success.");

            CUSTOM_LOG_DEBUG("Setting up random num generator...");

            taskTempRet = mbedtls_ctr_drbg_seed(
                &this->mbedCtx.ctrDrbg,
                mbedtls_entropy_func,
                &this->mbedCtx.entropyCtx,
                reinterpret_cast<const unsigned char*>(pers),
                strlen(pers)
            );

            if (taskTempRet != 0)
            {
                CUSTOM_LOG_ERROR(
                    "Failed to set up ctrDrbg, error code: {}", taskTempRet
                );
                this->cleanUp(true);
                return;
            }

            CUSTOM_LOG_DEBUG("Loading CA cert...");
            {
                auto& sharedIns =
                    AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();
                const HINSTANCE hIns = sharedIns.getExactVal(
                    &AikariPLS::Types::Lifecycle::PLSSharedIns::hModuleIns
                );
                auto getCrtPtrRet =
                    AikariShared::Utils::Windows::RC::loadStringResource<char>(
                        hIns, IDR_SEEWO_BROKER_CERT
                    );

                if (!getCrtPtrRet.success)
                {
                    CUSTOM_LOG_ERROR(
                        "Failed to load Seewo broker CA resource, exiting "
                        "thread..."
                    );
                    this->cleanUp(true);
                    return;
                }
#ifdef _DEBUG
                LOG_TRACE(
                    "Broker CA data {}",
                    std::string(getCrtPtrRet.retPtr, getCrtPtrRet.size)
                );
#endif

                std::vector<unsigned char> crtBufferProcessed;
                crtBufferProcessed.reserve(getCrtPtrRet.size + 1);
                crtBufferProcessed.insert(
                    crtBufferProcessed.end(),
                    getCrtPtrRet.retPtr,
                    getCrtPtrRet.retPtr + getCrtPtrRet.size
                );

                crtBufferProcessed.emplace_back('\0');

                taskTempRet = mbedtls_x509_crt_parse(
                    &this->mbedCtx.caCrt,
                    crtBufferProcessed.data(),
                    crtBufferProcessed.size()
                );

                if (taskTempRet != 0)
                {
                    CUSTOM_LOG_ERROR(
                        "Failed to parse Seewo broker CA cert, exiting "
                        "thread..."
                    );
                    this->cleanUp(true);
                    return;
                }
            }

            // Connect later (in connLoop)

            CUSTOM_LOG_DEBUG("Setting up SSL config...");
            taskTempRet = mbedtls_ssl_config_defaults(
                &this->mbedCtx.sslConfig,
                MBEDTLS_SSL_IS_CLIENT,
                MBEDTLS_SSL_TRANSPORT_STREAM,
                MBEDTLS_SSL_PRESET_DEFAULT
            );
            if (taskTempRet != 0)
            {
                CUSTOM_LOG_ERROR(
                    "Failed to init SSL default config, error code: {}",
                    taskTempRet
                );
                this->cleanUp(true);
                return;
            }

            mbedtls_ssl_conf_authmode(
                &this->mbedCtx.sslConfig, MBEDTLS_SSL_VERIFY_REQUIRED
            );
            mbedtls_ssl_conf_ca_chain(
                &this->mbedCtx.sslConfig, &this->mbedCtx.caCrt, nullptr
            );
            mbedtls_ssl_conf_rng(
                &this->mbedCtx.sslConfig,
                mbedtls_ctr_drbg_random,
                &this->mbedCtx.ctrDrbg
            );

            taskTempRet =
                mbedtls_ssl_setup(&this->sslCtx, &this->mbedCtx.sslConfig);
            if (taskTempRet != 0)
            {
                CUSTOM_LOG_ERROR(
                    "Failed to apply SSL config, error code: {}", taskTempRet
                );
                this->cleanUp(true);
                return;
            }

            taskTempRet = mbedtls_ssl_set_hostname(
                &this->sslCtx, this->launchArg.targetHost.c_str()
            );
            if (taskTempRet != 0)
            {
                CUSTOM_LOG_ERROR(
                    "Failed to set up SSL verify hostname, error code: {}",
                    taskTempRet
                );
                this->cleanUp(true);
                return;
            }

            CUSTOM_LOG_INFO("Moving to next step...");
            this->runConnLoop();
        }
        catch (const std::exception& err)
        {
            CUSTOM_LOG_ERROR(
                "An unexpected error occurred in MQTT Client loop: {}",
                err.what()
            );
            Telemetry::sendEventStr(
                SENTRY_LEVEL_ERROR,
                TELEMETRY_MODULE_NAME,
                std::format("Error running MQTT client loop: {}", err.what())
            );
            return;
        }
    };

    bool Client::refreshHostRealIP()
    {
        try
        {
            auto dnsQueryResult =
                AikariShared::Utils::Network::DNS::getDNSARecordResult(
                    this->launchArg.targetHost
                );
            if (dnsQueryResult.empty())
            {
                throw std::runtime_error(
                    "Failed to get broker real IP through DNS query."
                );
            }

            this->realBrokerDest = dnsQueryResult;
            return true;
        }
        catch (const std::exception& err)
        {
            this->realBrokerDest.emplace_back(
                AikariPLS::Types::Constants::MQTT::Client::
                    FALLBACK_IOT_BROKER_ADDR
            );
            this->realBrokerDest.emplace_back(
                AikariPLS::Types::Constants::MQTT::Client::
                    FALLBACK_IOT_BROKER_IP
            );
            CUSTOM_LOG_WARN("Error getting broker real IP: {}", err.what());
            Telemetry::addBreadcrumb(
                "error",
                std::format("Failed to get broker real IP: {}", err.what()),
                TELEMETRY_ACTION_CATEGORY,
                "warning"
            );
            return true;
        }
    }

    void Client::startSendQueueWorker()
    {
        try
        {
            auto& mqttSharedQueues =
                AikariPLS::Lifecycle::MQTT::PLSMQTTMsgQueues::getInstance();
            auto* sendQueuePtr = mqttSharedQueues.getPtr(
                &AikariPLS::Types::Lifecycle::MQTT::PLSMQTTMsgQueues::
                    brokerToClientQueue
            );

            while (!this->shouldExit)
            {
                auto task = sendQueuePtr->pop();

                if (task.type == AikariPLS::Types::MQTTMsgQueue::
                                     PACKET_OPERATION_TYPE::CTRL_THREAD_END)
                {
                    CUSTOM_LOG_DEBUG(
                        "Exiting Fake Broker -> Fake Client msg queue "
                        "listening thread..."
                    );
                    return;
                }

                this->sendThreadPool->insertTask(std::move(task));
            }
        }
        catch (const std::exception& err)
        {
            CUSTOM_LOG_ERROR(
                "Critical error occurred in Fake Broker -> Fake Client -> Real "
                "Broker msg queue worker, error: {}",
                err.what()
            );
        }
    }

    void Client::initSendThreadPool()
    {
        this->sendThreadPool.reset();
        this->sendThreadPool = std::make_unique<
            AikariShared::Infrastructure::MessageQueue::PoolQueue<
                AikariPLS::Types::MQTTMsgQueue::FlaggedPacket>>(
            this->sendThreadCount,
            [this](AikariPLS::Types::MQTTMsgQueue::FlaggedPacket packet)
            {
                if (this->isConnectionActive)
                {
                    try
                    {
                        std::function<async_mqtt::packet_id_type()>
                            genNewPacketId = [this]()
                        {
                            std::lock_guard<std::recursive_mutex>
                                connectionIOLockGuard(this->connectionIOLock);

                            auto packetId =
                                this->connection->acquire_unique_packet_id();
                            if (packetId == std::nullopt)
                            {
                                throw std::runtime_error(
                                    "Packet ID acquire failed."
                                );
                            }
                            return packetId.value();
                        };

                        std::optional<std::string> newTopicName = std::nullopt;
                        std::optional<std::string> newGetMessageId =
                            std::nullopt;
                        if (packet.packet
                                ->get_if<async_mqtt::v3_1_1::publish_packet>())
                        {
                            if (packet.props.endpointType ==
                                AikariPLS::Types::MQTTMsgQueue::
                                    PACKET_ENDPOINT_TYPE::GET)
                            {
                                // clang-format off
                                // if pending pkt is a GET publish pkt, then
                                // topicName replace is required (/up/request/1 <-THIS)
                                // clang-format on
                                std::string originalMsgId =
                                    packet.props.msgId.value_or("1");
                                auto thisMsgId = std::to_string(
                                    this->connection->endpointGetMsgIdCounter
                                        .fetch_add(1, std::memory_order_relaxed)
                                );
                                CUSTOM_LOG_TRACE(
                                    "This packet msgId info: ori: {} | "
                                    "replaced: "
                                    "{}",
                                    originalMsgId,
                                    thisMsgId
                                );
                                if (originalMsgId !=
                                    thisMsgId)  // offset exists
                                {
                                    AikariPLS::Types::MQTTMsgQueue::
                                        PacketTopicProps newTopicProps(
                                            packet.props
                                        );  // copy in order to prevent
                                            // msgId disorder when pktId
                                            // acquire fail
                                    newTopicProps.msgId = thisMsgId;
                                    newGetMessageId = thisMsgId;
                                    if (packet.type !=
                                        AikariPLS::Types::MQTTMsgQueue::
                                            PACKET_OPERATION_TYPE::PKT_VIRTUAL)
                                    {
                                        // ↑ rep for VIRTUAL pkt won't be
                                        // forwarded to fake broker, so no
                                        // need for topicName map
                                        this->connection->endpointGetIdsMap
                                            .emplace(thisMsgId, originalMsgId);
                                    }
                                    newTopicName =
                                        AikariPLS::Utils::MQTTPacketUtils::
                                            mergeTopic(newTopicProps);
                                }
                            }
                        }

                        switch (packet.type)
                        {
                            case AikariPLS::Types::MQTTMsgQueue::
                                PACKET_OPERATION_TYPE::PKT_TRANSPARENT:
                            case AikariPLS::Types::MQTTMsgQueue::
                                PACKET_OPERATION_TYPE::PKT_MODIFIED:
                            {
                                try
                                {
                                    auto newPacket = AikariPLS::Utils::
                                        MQTTPacketUtils::reconstructPacket(
                                            packet.packet.value(),
                                            genNewPacketId,
                                            std::move(newTopicName),
                                            std::move(packet.newPayload)
                                        );
                                    this->connection->send(
                                        std::move(newPacket)
                                    );
                                }
                                catch (const std::exception& err)
                                {
                                    this->sendThreadPool->insertTask(
                                        std::move(packet)
                                    );
#ifdef _DEBUG
                                    CUSTOM_LOG_WARN(
                                        "Packet ID acquire failed, "
                                        "deferring packet send. Error: {}",
                                        err.what()
                                    );
#endif
                                }
                            }
                            break;
                            case AikariPLS::Types::MQTTMsgQueue::
                                PACKET_OPERATION_TYPE::PKT_VIRTUAL:
                            {
                                if (packet.props.endpointType ==
                                    AikariPLS::Types::MQTTMsgQueue::
                                        PACKET_ENDPOINT_TYPE::RPC)
                                {
                                    // rpc no need for replace, because
                                    // every rpcId is unique
                                    this->connection->endpointRpcIgnoredIds
                                        .emplace(packet.props.msgId.value());
                                }
                                else if (packet.props.endpointType ==
                                         AikariPLS::Types::MQTTMsgQueue::
                                             PACKET_ENDPOINT_TYPE::GET)
                                {
                                    this->connection->endpointGetIgnoredIds
                                        .emplace(newGetMessageId.value());
                                }

                                try
                                {
                                    async_mqtt::v3_1_1::publish_packet
                                        virtualPacket(
                                            genNewPacketId(),
                                            newTopicName.value_or(
                                                AikariPLS::Utils::
                                                    MQTTPacketUtils::mergeTopic(
                                                        packet.props
                                                    )
                                            ),
                                            packet.newPayload.value(),
                                            async_mqtt::qos::at_least_once
                                        );

                                    this->connection->send(
                                        std::move(virtualPacket)
                                    );
                                }
                                catch (const std::exception& err)
                                {
                                    this->sendThreadPool->insertTask(
                                        std::move(packet)
                                    );
#ifdef _DEBUG
                                    CUSTOM_LOG_WARN(
                                        "Packet ID acquire failed, "
                                        "deferring packet send. Error: {}",
                                        err.what()
                                    );
#endif
                                }
                            }
                            break;

                            default:
                                break;
                        }
                    }
                    catch (const std::exception& err)
                    {
                        CUSTOM_LOG_ERROR(
                            "Unexpected error occurred processing pending "
                            "packets, error: {}",
                            err.what()
                        );
                    }
                }
            }
        );
    }

    void Client::resetConnection()
    {
        this->sslCtxLock.lock();
        mbedtls_ssl_close_notify(&this->sslCtx);
        mbedtls_net_free(&this->netCtxs.serverFd);
        mbedtls_ssl_session_reset(&this->sslCtx);
        this->sslCtxLock.unlock();
        this->isConnectionActive = false;
        this->connection.reset();
    };

    void Client::runConnLoop()
    {
        CUSTOM_LOG_DEBUG("Ready to connect to the server.");

        this->isConnectionActive = false;

        while (!this->shouldExit)
        {
            int taskTempRet = 0;

            if (this->pendingExit)
            {
                CUSTOM_LOG_INFO("Flag detected, MQTT Client is exiting.");
                this->resetConnection();
                this->pendingExit.store(false);
                continue;
            }

            {
                if (!this->isConnectionActive)
                {
                    if (this->isConnectRetryDisabled ||
                        retryTimes >= maxRetry ||
                        this->curTryingDest >= this->realBrokerDest.size())
                    {
                        CUSTOM_LOG_ERROR(
                            "Max connect tries reached, forcibly exiting MQTT "
                            "Client..."
                        );
                        Telemetry::addBreadcrumb(
                            "default",
                            "Max connect retries reached, client is exiting",
                            TELEMETRY_ACTION_CATEGORY,
                            "error"
                        );
                        this->cleanUp(true);
                        return;
                    }

                    // handshake using config
                    CUSTOM_LOG_INFO(
                        "Connecting to ssl://{}:{}",
                        this->launchArg.targetHost,
                        this->launchArg.targetPort
                    );

                    auto thisHost =
                        this->realBrokerDest.at(this->curTryingDest).c_str();
                    Telemetry::addBreadcrumb(
                        "default",
                        std::format(
                            "MQTT Client trying connect to ssl://{}:{} (Phase "
                            "mbedtls_net_connect)",
                            thisHost,
                            this->launchArg.targetPort
                        ),
                        TELEMETRY_ACTION_CATEGORY,
                        "info"
                    );
                    std::unique_lock<std::mutex> lock(this->sslCtxLock
                    );  // lock until handshake done
                    taskTempRet = mbedtls_net_connect(
                        &this->netCtxs.serverFd,
                        thisHost,
                        std::to_string(this->launchArg.targetPort).c_str(),
                        MBEDTLS_NET_PROTO_TCP
                    );

                    if (taskTempRet != 0)
                    {
                        CUSTOM_LOG_ERROR(
                            "Failed connecting to broker host {}, error code: "
                            "{}. Trying another...",
                            thisHost,
                            taskTempRet
                        );
                        Telemetry::sendEventStr(
                            SENTRY_LEVEL_WARNING,
                            TELEMETRY_MODULE_NAME,
                            std::format(
                                "MQTT Client failed connecting to real broker "
                                "ssl://{}:{}",
                                thisHost,
                                this->launchArg.targetPort
                            )
                        );
                        this->curTryingDest++;
                        continue;
                    };

                    CUSTOM_LOG_DEBUG("Setting bio...");
                    mbedtls_ssl_set_bio(
                        &this->sslCtx,
                        &this->netCtxs.serverFd,
                        mbedtls_net_send,
                        mbedtls_net_recv,
                        nullptr
                    );

                    CUSTOM_LOG_INFO("Handshaking with server...");
                    bool handshakeSuccess = true;
                    while ((taskTempRet =
                                mbedtls_ssl_handshake(&this->sslCtx)) != 0)
                    {
                        if (taskTempRet != MBEDTLS_ERR_SSL_WANT_READ &&
                            taskTempRet != MBEDTLS_ERR_SSL_WANT_WRITE)
                        {
                            CUSTOM_LOG_ERROR(
                                "Unexpected error occurred handshaking with "
                                "server, error code: {}",
                                taskTempRet
                            );
                            Telemetry::addBreadcrumb(
                                "default",
                                std::format(
                                    "SSL Handshake with {} failed, ec: {}",
                                    thisHost,
                                    taskTempRet
                                ),
                                TELEMETRY_ACTION_CATEGORY,
                                "warning"
                            );
                            mbedtls_ssl_session_reset(&this->sslCtx);
                            mbedtls_net_free(&this->netCtxs.serverFd);
                            handshakeSuccess = false;
                            break;
                        }
                    }

                    if (!handshakeSuccess)
                    {
                        break;
                    }

                    CUSTOM_LOG_DEBUG("Handshake completed.");

                    CUSTOM_LOG_DEBUG("Verifying certificate chain...");
                    taskTempRet = static_cast<int>(
                        mbedtls_ssl_get_verify_result(&this->sslCtx)
                    );
                    if (taskTempRet != 0)
                    {
                        char verifyResultBuffer[512] = {};
                        mbedtls_x509_crt_verify_info(
                            verifyResultBuffer,
                            sizeof(verifyResultBuffer),
                            "",
                            taskTempRet
                        );

                        CUSTOM_LOG_ERROR(
                            "Failed to verify server certificate, error: {}",
                            verifyResultBuffer
                        );
                        Telemetry::sendEventStr(
                            SENTRY_LEVEL_ERROR,
                            TELEMETRY_MODULE_NAME,
                            "[HEADS UP] Client-side real broker cert chain "
                            "verification failed, broker's cert might be "
                            "changed, please take action!"
                        );
                        lock.unlock();
                        this->resetConnection();
                        break;
                    }
                    CUSTOM_LOG_DEBUG("Certificate verified.");

                    lock.unlock();
                    Telemetry::addBreadcrumb(
                        "default",
                        "MQTT Client's connection is established",
                        TELEMETRY_ACTION_CATEGORY,
                        "debug"
                    );
                    CUSTOM_LOG_INFO("Connection established.");
                    this->isConnectionActive = true;
                    this->retryTimes = 0;

                    this->connection = std::make_unique<
                        AikariPLS::Components::MQTTClient::Class::
                            MQTTClientConnection>(
                        [this](const async_mqtt::packet_variant& packet)
                        {
                            if (!this->isConnectionActive)
                                return;

                            std::vector<unsigned char> pendingBuf;
                            auto pktBufSeq = packet.const_buffer_sequence();
                            for (auto& pktBuf : pktBufSeq)
                            {
                                pendingBuf.insert(
                                    pendingBuf.end(),
                                    static_cast<const unsigned char*>(
                                        pktBuf.data()
                                    ),
                                    static_cast<const unsigned char*>(
                                        pktBuf.data()
                                    ) + pktBuf.size()
                                );
                            }

                            std::lock_guard<std::mutex> lock(this->sslCtxLock);
                            const unsigned char* curBufPtr = pendingBuf.data();
                            auto totalBufRemainLen = pendingBuf.size();

                            while (totalBufRemainLen > 0)
                            {
                                auto writeResult = mbedtls_ssl_write(
                                    &this->sslCtx, curBufPtr, totalBufRemainLen
                                );

                                if (writeResult > 0)
                                {
                                    totalBufRemainLen -= writeResult;
                                    curBufPtr += writeResult;
                                }
                                else if (writeResult ==
                                             MBEDTLS_ERR_SSL_WANT_READ ||
                                         writeResult ==
                                             MBEDTLS_ERR_SSL_WANT_WRITE)
                                {
                                    std::this_thread::sleep_for(
                                        std::chrono::milliseconds(1)
                                    );
                                    continue;  // TODO: Same as broker
                                }
                                else
                                {
                                    CUSTOM_LOG_ERROR(
                                        "Error calling mbedtls_ssl_write, "
                                        "ec: {}",
                                        writeResult
                                    );
                                    Telemetry::addBreadcrumb(
                                        "default",
                                        std::format(
                                            "Error calling "
                                            "mbedtls_ssl_write | EC: {}",
                                            writeResult
                                        ),
                                        TELEMETRY_ACTION_CATEGORY,
                                        "error"
                                    );
                                    break;
                                }
                            }
                        },
                        [this]()
                        {
                            CUSTOM_LOG_DEBUG(
                                "Client conn close triggered by handler side."
                            );
                            Telemetry::addBreadcrumb(
                                "default",
                                "Client connection closed by handler",
                                TELEMETRY_ACTION_CATEGORY,
                                "debug"
                            );
                            this->pendingExit.store(true);
                            // this->isConnectionActive = false;
                        },
                        [](async_mqtt::error_code errCode)
                        {
                            // TODO: Error handling (same as broker)
                        },
                        [this]()
                        {
                            this->retryTimes = 0;
                            this->isConnectRetryDisabled = false;
                        }
                    );

                    async_mqtt::v3_1_1::connect_packet connPacket(
                        true,
                        this->launchArg.keepAliveSec,
                        this->launchArg.clientId,
                        this->launchArg.username,
                        this->launchArg.password
                    );
                    this->connection->send(std::move(connPacket));
                }
            }

            bool curHasPendingData = false;
            if (this->isConnectionActive)
            {
                if (mbedtls_ssl_get_bytes_avail(&this->sslCtx) > 0)
                {
                    curHasPendingData = true;
                }
            }

            fd_set readFds;
            struct timeval timeVal = { .tv_sec = curHasPendingData ? 0 : 1,
                                       .tv_usec = 0 };

            FD_ZERO(&readFds);
            FD_SET(this->netCtxs.serverFd.fd, &readFds);

            taskTempRet = select(0, &readFds, nullptr, nullptr, &timeVal);

            if (taskTempRet < 0)
            {
                int lastError = WSAGetLastError();
                CUSTOM_LOG_ERROR(
                    "Unexpected error running select() | WSAErrorCode: {}",
                    lastError
                );
                Telemetry::sendEventStr(
                    SENTRY_LEVEL_ERROR,
                    TELEMETRY_MODULE_NAME,
                    std::format(
                        "Unexpected error occurred running select, WSA ec: {}",
                        lastError
                    )
                );
                break;
            }

            {
                if (this->isConnectionActive &&
                    ((taskTempRet > 0 &&
                      FD_ISSET(this->netCtxs.serverFd.fd, &readFds)) ||
                     curHasPendingData))
                {
                    unsigned char buffer[4096] = {};

                    this->sslCtxLock.lock();
                    taskTempRet = mbedtls_ssl_read(
                        &this->sslCtx, buffer, sizeof(buffer) - 1
                    );
                    this->sslCtxLock.unlock();

                    // Begin ssl_read ret switch
                    if (taskTempRet > 0)
                    {
                        std::string strIncomingData(
                            reinterpret_cast<char*>(buffer), taskTempRet
                        );

                        // data from broker

                        std::stringstream strStream(strIncomingData);

                        try
                        {
                            std::lock_guard<std::recursive_mutex>
                                connectionIOLockGuard(this->connectionIOLock);

                            this->connection->recv(strStream);
                        }
                        catch (std::exception& e)
                        {
                            CUSTOM_LOG_WARN(
                                "Error running connection.recv, closing "
                                "connection... | Error detail: ",
                                e.what()
                            );
                            this->pendingExit.store(true);
                        }
                    }
                    else if (taskTempRet == MBEDTLS_ERR_SSL_WANT_READ ||
                             taskTempRet == MBEDTLS_ERR_SSL_WANT_WRITE)
                    {
                        // silently ignore
                    }
                    else if (taskTempRet <= 0)
                    {
                        switch (taskTempRet)
                        {
                            case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
                                CUSTOM_LOG_DEBUG("Server connection closed.");
                                break;
                            case MBEDTLS_ERR_NET_CONN_RESET:
                                CUSTOM_LOG_ERROR("Server connection reset.");
                                break;
                            case MBEDTLS_ERR_NET_RECV_FAILED:
                                CUSTOM_LOG_ERROR(
                                    "Failed to run mbedtls_net_recv."
                                );
                                break;
                            default:
                                CUSTOM_LOG_DEBUG(
                                    "mbedtls_ssl_read returned {}", taskTempRet
                                );
                                break;
                        }

                        CUSTOM_LOG_DEBUG("Server disconnected, cleaning up...");
                        this->pendingExit.store(true);

                        this->isConnectionActive = false;
                        this->retryTimes++;
                    }
                    // End ssl_read ret switch
                }  // LEVEL = taskRet > 0 && FDISSET

                if (taskTempRet == 0 && this->isConnectionActive &&
                    !curHasPendingData)
                {
                    this->connection->checkTimerTimeout();
                }
            }
        }

        this->sslCtxLock.lock();
        mbedtls_ssl_close_notify(&this->sslCtx);
        mbedtls_ssl_session_reset(&this->sslCtx);
        this->sslCtxLock.unlock();
        Telemetry::addBreadcrumb(
            "default",
            "MQTT Client stopped.",
            TELEMETRY_ACTION_CATEGORY,
            "debug"
        );
        CUSTOM_LOG_INFO("Client stopped.");
    }

    namespace ClientLifecycleController
    {
        static std::mutex clientResetMutex;

        void initAndMountClientIns(const ClientLaunchArg arg)
        {
            CUSTOM_LOG_INFO("<Controller> Creating MQTT Client instance...");

            Telemetry::addBreadcrumb(
                "default",
                "MQTT Client Lifecycle Controller is creating client ins",
                "pls.mqtt.client.lifecycle",
                "debug"
            );

            auto& sharedIns =
                AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();

            auto clientInsOldPtr = sharedIns.getPtr(
                &AikariPLS::Types::Lifecycle::PLSSharedIns::mqttClient
            );
            if (clientInsOldPtr != nullptr)
            {
                Telemetry::addBreadcrumb(
                    "default",
                    "Prev client ins ptr detected, triggering reset...",
                    "pls.mqtt.client.lifecycle",
                    "debug"
                );
                resetClientIns(true);
            }

            auto clientInsPtr = std::make_unique<Client>(arg);
            sharedIns.setPtr(
                &AikariPLS::Types::Lifecycle::PLSSharedIns::mqttClient,
                std::move(clientInsPtr)
            );
        };

        void resetClientIns(bool noDisconnPkt)
        {
            CUSTOM_LOG_INFO("<Controller> Resetting MQTT Client instance...");

            Telemetry::addBreadcrumb(
                "default",
                "MQTT Client Lifecycle Controller is trying to reset client "
                "ins",
                "pls.mqtt.client.lifecycle",
                "debug"
            );

            auto lockRes = clientResetMutex.try_lock();
            if (!lockRes)
                return;

            Telemetry::addBreadcrumb(
                "default",
                "Lock gained, resetting instance",
                "pls.mqtt.client.lifecycle",
                "debug"
            );

            auto& sharedIns =
                AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();
            auto clientInsPtr = sharedIns.getPtr(
                &AikariPLS::Types::Lifecycle::PLSSharedIns::mqttClient
            );
            if (clientInsPtr != nullptr)
            {
                if (!noDisconnPkt)
                {
                    // TODO: Send MQTT DISCONNECT pkt
                }
            }
            sharedIns.resetPtr(
                &AikariPLS::Types::Lifecycle::PLSSharedIns::mqttClient
            );

            clientResetMutex.unlock();
        };
    }  // namespace ClientLifecycleController

}  // namespace AikariPLS::Components::MQTTClient
