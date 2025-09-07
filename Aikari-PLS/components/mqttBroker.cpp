#include "mqttBroker.h"

#define CUSTOM_LOG_HEADER "[MQTT Broker]"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <WinSock2.h>

#include "Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp"
#include "mqttLifecycle.h"
#include "utils/mqttPacketUtils.h"

#pragma comment(lib, "ws2_32")

namespace AikariPLS::Components::MQTTBroker
{
    Broker::Broker(const BrokerLaunchArg& arg)
    {
        this->hostname = arg.hostname;
        this->port = arg.port;
        this->certPath = arg.certPath.string();
        this->keyPath = arg.keyPath.string();

        this->serverLoop = std::make_unique<std::jthread>(
            &Broker::startTlsServerLoop, this, arg
        );

        this->initSendThreadPool();
        this->sendQueueWorker =
            std::make_unique<std::jthread>(&Broker::startSendQueueWorker, this);
    };

    Broker::~Broker()
    {
        this->cleanUp(false);
    };

    void Broker::cleanUp(bool ignoreThreadJoin)
    {
        if (this->cleaned)
            return;

        CUSTOM_LOG_DEBUG("Cleaning up MQTT Broker server context...");
        this->cleaned = true;
        this->shouldExit = true;

        auto& mqttSharedQueues =
            AikariPLS::Lifecycle::MQTT::PLSMQTTMsgQueues::getInstance();
        auto* sendQueuePtr =
            mqttSharedQueues.getPtr(&AikariPLS::Types::lifecycle::MQTT::
                                        PLSMQTTMsgQueues::clientToBrokerQueue);

        sendQueuePtr->push(
            {
                .type =
                    Types::mqttMsgQueue::PACKET_OPERATION_TYPE::CTRL_THREAD_END,
            }
        );

        if (this->serverLoop->joinable() && !ignoreThreadJoin)
        {
            this->serverLoop->join();
        }
        if (this->sendQueueWorker->joinable() && !ignoreThreadJoin)
        {
            this->sendQueueWorker->join();
        }

        std::lock_guard<std::mutex> lock(this->sslCtxLock);
        mbedtls_net_free(&this->netCtx.listenFd);
        mbedtls_net_free(&this->netCtx.clientFd);
        mbedtls_x509_crt_free(&this->mbedCtx.srvCert);
        mbedtls_ssl_free(&this->sslCtx);
        mbedtls_pk_free(&this->mbedCtx.privKey);
        mbedtls_ssl_config_free(&this->mbedCtx.sslConfig);
        mbedtls_ctr_drbg_free(&this->mbedCtx.ctrDrbg);
        mbedtls_entropy_free(&this->mbedCtx.entropyCtx);
    };

    void Broker::startTlsServerLoop(const BrokerLaunchArg& arg)
    {
        try
        {
            int taskTempRet = 0;

            mbedtls_net_init(&this->netCtx.listenFd);
            mbedtls_net_init(&this->netCtx.clientFd);
            mbedtls_ssl_init(&this->sslCtx);
            mbedtls_ssl_config_init(&this->mbedCtx.sslConfig);
            mbedtls_x509_crt_init(&this->mbedCtx.srvCert);
            mbedtls_pk_init(&this->mbedCtx.privKey);
            mbedtls_entropy_init(&this->mbedCtx.entropyCtx);
            mbedtls_ctr_drbg_init(&this->mbedCtx.ctrDrbg);

            CUSTOM_LOG_INFO("MQTT Broker TLS server context init success.");

            CUSTOM_LOG_DEBUG("Setting up random num generator...");

            taskTempRet = mbedtls_ctr_drbg_seed(
                &this->mbedCtx.ctrDrbg,
                mbedtls_entropy_func,
                &this->mbedCtx.entropyCtx,
                (const unsigned char*)pers,
                strlen(pers)
            );

            if (taskTempRet != 0)
            {
                CUSTOM_LOG_ERROR(
                    "Failed to set up ctrDrbg, error code: {}", taskTempRet
                );
                cleanUp(true);
                return;
            }

            CUSTOM_LOG_DEBUG("Random num generator init done.");

            CUSTOM_LOG_DEBUG("Loading server cert...");
            taskTempRet = mbedtls_x509_crt_parse_file(
                &this->mbedCtx.srvCert, this->certPath.c_str()
            );
            if (taskTempRet != 0)
            {
                CUSTOM_LOG_ERROR(
                    "Failed to init server cert, error code: {}", taskTempRet
                );
                cleanUp(true);
                return;
            }

            taskTempRet = mbedtls_pk_parse_keyfile(
                &this->mbedCtx.privKey,
                this->keyPath.c_str(),
                nullptr,
                mbedtls_ctr_drbg_random,
                &this->mbedCtx.ctrDrbg
            );
            if (taskTempRet != 0)
            {
                CUSTOM_LOG_ERROR(
                    "Failed to init server pKey, error code: {}", taskTempRet
                );
                cleanUp(true);
                return;
            }

            CUSTOM_LOG_DEBUG("TLS cert & key init done.");

            CUSTOM_LOG_INFO(
                "MQTT Broker binding to ssl://{}:{}", this->hostname, this->port
            );
            taskTempRet = mbedtls_net_bind(
                &this->netCtx.listenFd,
                this->hostname.c_str(),
                std::to_string(this->port).c_str(),
                MBEDTLS_NET_PROTO_TCP
            );
            if (taskTempRet != 0)
            {
                CUSTOM_LOG_ERROR(
                    "Failed while binding to addr, error code: {}", taskTempRet
                );
                cleanUp(true);
                return;
            }

            CUSTOM_LOG_INFO("Bind to ssl://{}:{}", this->hostname, this->port);

            CUSTOM_LOG_DEBUG("Setting up default TLS config...");
            taskTempRet = mbedtls_ssl_config_defaults(
                &this->mbedCtx.sslConfig,
                MBEDTLS_SSL_IS_SERVER,
                MBEDTLS_SSL_TRANSPORT_STREAM,
                MBEDTLS_SSL_PRESET_DEFAULT
            );
            if (taskTempRet != 0)
            {
                CUSTOM_LOG_ERROR(
                    "Failed to set default TLS config, error code: {}",
                    taskTempRet
                );
                cleanUp(true);
                return;
            }

            CUSTOM_LOG_DEBUG("Setting RNG and cert config...");
            mbedtls_ssl_conf_rng(
                &this->mbedCtx.sslConfig,
                mbedtls_ctr_drbg_random,
                &this->mbedCtx.ctrDrbg
            );
            mbedtls_ssl_conf_own_cert(
                &this->mbedCtx.sslConfig,
                &this->mbedCtx.srvCert,
                &this->mbedCtx.privKey
            );

            CUSTOM_LOG_DEBUG("Applying TLS config to sslCtx...");
            taskTempRet =
                mbedtls_ssl_setup(&this->sslCtx, &this->mbedCtx.sslConfig);
            if (taskTempRet != 0)
            {
                CUSTOM_LOG_ERROR(
                    "Failed to apply TLS config, error code: {}", taskTempRet
                );
                cleanUp(true);
                return;
            }

            CUSTOM_LOG_INFO("MQTT Broker TLS config applied.");
            CUSTOM_LOG_INFO(
                "MQTT Broker TLS server initialized. Listening on ssl://{}:{}",
                this->hostname,
                this->port
            );

            this->listenLoop();
        }
        catch (const std::exception& err)
        {
            CUSTOM_LOG_ERROR(
                "An unexpected error occurred in MQTT Broker loop: {}",
                err.what()
            );
            return;
        }
    }

    void Broker::startSendQueueWorker()
    {
        try
        {
            auto& mqttSharedQueues =
                AikariPLS::Lifecycle::MQTT::PLSMQTTMsgQueues::getInstance();
            auto* sendQueuePtr = mqttSharedQueues.getPtr(
                &AikariPLS::Types::lifecycle::MQTT::PLSMQTTMsgQueues::
                    clientToBrokerQueue
            );

            while (!this->shouldExit)
            {
                auto task = sendQueuePtr->pop();

                if (task.type == AikariPLS::Types::mqttMsgQueue::
                                     PACKET_OPERATION_TYPE::CTRL_THREAD_END)
                {
                    CUSTOM_LOG_DEBUG(
                        "Exiting Fake Client -> Fake Broker msg queue "
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
                "Critical error occurred in Fake Client -> Fake Broker -> Real "
                "Client msg queue worker, error: {}",
                err.what()
            );
        }
    }

    void Broker::initSendThreadPool()
    {
        this->sendThreadPool.reset();
        this->sendThreadPool = std::make_unique<
            AikariShared::infrastructure::MessageQueue::PoolQueue<
                AikariPLS::Types::mqttMsgQueue::FlaggedPacket>>(
            this->sendThreadCount,
            [this](AikariPLS::Types::mqttMsgQueue::FlaggedPacket packet)
            {
                if (this->netCtx.curClientFd != nullptr)
                {
                    switch (packet.type)
                    {
                        case AikariPLS::Types::mqttMsgQueue::
                            PACKET_OPERATION_TYPE::PKT_TRANSPARENT:
                        case AikariPLS::Types::mqttMsgQueue::
                            PACKET_OPERATION_TYPE::PKT_MODIFIED:
                        {
                            // TODO: error handling (same as mqttClient.cpp)
                            std::function<async_mqtt::packet_id_type()>
                                genNewPacketId = [this]()
                            {
                                return this->connection
                                    ->acquire_unique_packet_id()
                                    .value_or(0);
                            };
                            auto newPacket = AikariPLS::Utils::MQTTPacketUtils::
                                reconstructPacketWithPktId(
                                    packet.packet.value(), genNewPacketId
                                );

                            this->connection->send(std::move(newPacket));
                        }
                        break;

                        case AikariPLS::Types::mqttMsgQueue::
                            PACKET_OPERATION_TYPE::PKT_DROP:
                        default:
                        {
                            // drop, do nothing
                        }
                        break;
                    }
                }
            }
        );
    }

    void Broker::resetCurConnection()
    {
        this->sslCtxLock.lock();
        mbedtls_ssl_close_notify(&this->sslCtx);
        mbedtls_net_free(&this->netCtx.clientFd);
        mbedtls_ssl_session_reset(&this->sslCtx);
        this->sslCtxLock.unlock();
        this->connection.reset();
    };

    void Broker::processConnectPackage(std::string& package)
    {
        std::string original("MQIsdp");
        auto findResult = package.find(original);
        if (findResult == std::string::npos)
        {
            return;
        }

        CUSTOM_LOG_DEBUG("CONNECT packet found, replacing headers...");
        package[findResult - 2] = 0x00;
        package[findResult - 1] = 0x04;
        package.replace(findResult, original.length(), "MQTT");
        package[findResult + 4] = 0x04;

        auto originalLastLength = package[findResult - 3];
        package[findResult - 3] = originalLastLength - 0x02;

#ifdef _DEBUG

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (unsigned char c : package)
        {
            ss << std::setw(2) << static_cast<int>(c);
        }
        CUSTOM_LOG_TRACE("Replace result: {}", ss.str());

#endif
    };

    void Broker::listenLoop()
    {
        this->netCtx.curClientFd = nullptr;

        CUSTOM_LOG_DEBUG("Waiting for client connection...");

        while (!this->shouldExit)
        {
            // 加个 writeFd 可能更好, 但目前应该没必要

            fd_set readFds;
            struct timeval timeVal;

            FD_ZERO(&readFds);
            FD_SET(this->netCtx.listenFd.fd, &readFds);

            timeVal.tv_sec = 1;
            timeVal.tv_usec = 0;

            if (this->netCtx.curClientFd != nullptr)
            {
                FD_SET(this->netCtx.curClientFd->fd, &readFds);
            }

            int taskTempRet;

            taskTempRet = select(0, &readFds, nullptr, nullptr, &timeVal);

            if (taskTempRet < 0)
            {
                CUSTOM_LOG_ERROR("Unexpected error running select()");
                break;
            }

            do  // create a scope
            {
                if (FD_ISSET(this->netCtx.listenFd.fd, &readFds))
                {
                    CUSTOM_LOG_DEBUG("New activity on listenFd detected.");
                    // Incoming connection available
                    if (this->netCtx.curClientFd != nullptr)
                    {
                        mbedtls_net_context tempCtx;
                        mbedtls_net_init(&tempCtx);
                        mbedtls_net_accept(
                            &this->netCtx.listenFd,
                            &tempCtx,
                            nullptr,
                            0,
                            nullptr
                        );
                        mbedtls_net_free(&tempCtx);
                        CUSTOM_LOG_DEBUG(
                            "A client has already connected, closing new "
                            "connection..."
                        );
                    }
                    else
                    {
                        CUSTOM_LOG_DEBUG("Handshaking with new client...");

                        std::unique_lock<std::mutex> lock(this->sslCtxLock);
                        // so cur no client, new client connecting
                        taskTempRet = mbedtls_net_accept(
                            &this->netCtx.listenFd,
                            &this->netCtx.clientFd,
                            nullptr,
                            0,
                            nullptr
                        );

                        if (taskTempRet != 0)
                        {
                            CUSTOM_LOG_ERROR(
                                "Unexpected error occurred in "
                                "mbedtls_net_accept, "
                                "error "
                                "code: "
                                "{}",
                                taskTempRet
                            );
                            lock.unlock();
                            this->resetCurConnection();
                            break;  // jump out from this do-while scope
                        }
                        CUSTOM_LOG_DEBUG("New connection accepted.");

                        mbedtls_ssl_set_bio(
                            &this->sslCtx,
                            &this->netCtx.clientFd,
                            mbedtls_net_send,
                            mbedtls_net_recv,
                            nullptr
                        );

                        CUSTOM_LOG_DEBUG("Performing handshake...");
                        bool handshakeResult = true;
                        while ((taskTempRet = mbedtls_ssl_handshake(&sslCtx)) !=
                               0)
                        {
                            if (taskTempRet != MBEDTLS_ERR_SSL_WANT_READ &&
                                taskTempRet != MBEDTLS_ERR_SSL_WANT_WRITE)
                            {
                                CUSTOM_LOG_ERROR(
                                    "An error occurred while handshaking with "
                                    "client, "
                                    "error "
                                    "code: ",
                                    taskTempRet
                                );
                                handshakeResult = false;
                                break;
                            }
                        }

                        if (handshakeResult == false)
                        {
                            break;
                        }

                        CUSTOM_LOG_DEBUG("SSL handshake completed.");

                        lock.unlock();

                        this->netCtx.curClientFd = &this->netCtx.clientFd;

                        this->connection =
                            std::make_unique<AikariPLS::Components::MQTTBroker::
                                                 Class::MQTTBrokerConnection>(
                                [this](async_mqtt::packet_variant packet)
                                {
                                    if (this->netCtx.curClientFd == nullptr)
                                    {
                                        return;
                                    }

                                    std::vector<unsigned char> pendingBuf;
                                    auto pktBufSeq =
                                        packet.const_buffer_sequence();
                                    for (auto& pktBuf : pktBufSeq)
                                    {
                                        pendingBuf.insert(
                                            pendingBuf.end(),
                                            (const unsigned char*)pktBuf.data(),
                                            (const unsigned char*)pktBuf.data(
                                            ) + pktBuf.size()
                                        );
                                    }

                                    std::lock_guard<std::mutex> lock(
                                        this->sslCtxLock
                                    );
                                    mbedtls_ssl_write(
                                        &this->sslCtx,
                                        pendingBuf.data(),
                                        pendingBuf.size()
                                    );
                                },
                                [this]()
                                {
                                    std::lock_guard<std::mutex> lock(
                                        this->sslCtxLock
                                    );
                                    this->resetCurConnection();
                                },
                                [this](async_mqtt::error_code errCode)
                                {
                                }
                            );
                    }
                }
            } while (false);

            do
            {
                if (this->netCtx.curClientFd != nullptr &&
                    FD_ISSET(this->netCtx.clientFd.fd, &readFds))
                {
                    unsigned char clientMsgBuffer[4096] = {};

                    this->sslCtxLock.lock();
                    taskTempRet = mbedtls_ssl_read(
                        &sslCtx, clientMsgBuffer, sizeof(clientMsgBuffer) - 1
                    );
                    this->sslCtxLock.unlock();

                    if (taskTempRet > 0)
                    {
                        std::string strReadData(
                            reinterpret_cast<char*>(clientMsgBuffer),
                            taskTempRet
                        );

                        if (clientMsgBuffer[0] == 0x10)
                        {  // Maybe CONNECT pkt
                            this->processConnectPackage(strReadData);
                        }
                        /*
                        #ifdef _DEBUG
                                            LOG_TRACE(
                                                "New data received, bytes: {},
                        data:\n{}", taskTempRet, strReadData
                                            );
                        #endif
                        */
                        std::stringstream strStream(strReadData);

                        if (this->connection != nullptr)
                        {
                            this->connection->recv(strStream);
                        }
                    }
                    else if (taskTempRet == MBEDTLS_ERR_SSL_WANT_READ ||
                             taskTempRet == MBEDTLS_ERR_SSL_WANT_WRITE)
                    {
                        // ignore
                    }
                    else if (taskTempRet <= 0)
                    {
                        switch (taskTempRet)
                        {
                            case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
                                CUSTOM_LOG_DEBUG("Client connection closed.");
                                break;
                            case MBEDTLS_ERR_NET_CONN_RESET:
                                CUSTOM_LOG_ERROR("Client connection reset.");
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

                        CUSTOM_LOG_DEBUG("Client disconnected, cleaning up...");
                        this->resetCurConnection();

                        this->netCtx.curClientFd = nullptr;
                    }
                }
            } while (false);
        }

        this->sslCtxLock.lock();
        mbedtls_ssl_close_notify(&this->sslCtx);
        mbedtls_ssl_session_reset(&this->sslCtx);
        this->sslCtxLock.unlock();
        CUSTOM_LOG_INFO("Broker stopped.");
    }
}  // namespace AikariPLS::Components::MQTTBroker
