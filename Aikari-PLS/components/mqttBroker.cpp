#include "mqttBroker.h"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32")

namespace AikariPLS::Components::MQTTBroker
{
    Broker::Broker(BrokerLaunchArg arg)
    {
        this->hostname = arg.hostname;
        this->port = arg.port;
        this->certPath = arg.certPath.string();
        this->keyPath = arg.keyPath.string();

        this->serverLoop = std::make_unique<std::jthread>(
            &Broker::startTlsServerLoop, this, arg
        );
    };

    Broker::~Broker()
    {
        this->cleanUp();
    };

    void Broker::cleanUp()
    {
        if (this->cleaned)
            return;

        LOG_DEBUG("Cleaning up MQTT Broker server contexts...");
        this->cleaned = true;
        this->shouldExit = true;
        if (this->serverLoop->joinable())
        {
            this->serverLoop->join();
        }
        mbedtls_net_free(&this->netCtx.listenFd);
        mbedtls_net_free(&this->netCtx.clientFd);
        mbedtls_x509_crt_free(&this->mbedCtx.srvCert);
        mbedtls_ssl_free(&this->sslCtx);
        mbedtls_pk_free(&this->mbedCtx.privKey);
        mbedtls_ssl_config_free(&this->mbedCtx.sslConfig);
        mbedtls_ctr_drbg_free(&this->mbedCtx.ctrDrbg);
        mbedtls_entropy_free(&this->mbedCtx.entropyCtx);
    };

    void Broker::startTlsServerLoop(BrokerLaunchArg arg)
    {
        try
        {
            int taskTempRet;

            mbedtls_net_init(&this->netCtx.listenFd);
            mbedtls_net_init(&this->netCtx.clientFd);
            mbedtls_ssl_init(&this->sslCtx);
            mbedtls_ssl_config_init(&this->mbedCtx.sslConfig);
            mbedtls_x509_crt_init(&this->mbedCtx.srvCert);
            mbedtls_pk_init(&this->mbedCtx.privKey);
            mbedtls_entropy_init(&this->mbedCtx.entropyCtx);
            mbedtls_ctr_drbg_init(&this->mbedCtx.ctrDrbg);

            LOG_INFO("MQTT Broker TLS server context init success.");

            LOG_DEBUG("Settings up random num generator...");

            taskTempRet = mbedtls_ctr_drbg_seed(
                &this->mbedCtx.ctrDrbg,
                mbedtls_entropy_func,
                &this->mbedCtx.entropyCtx,
                (const unsigned char*)pers,
                strlen(pers)
            );

            if (taskTempRet != 0)
            {
                LOG_ERROR(
                    "Failed to set up ctrDrbg, error code: {}", taskTempRet
                );
                cleanUp();
                return;
            }

            LOG_DEBUG("Random num generator init done.");

            LOG_DEBUG("Loading server cert...");
            taskTempRet = mbedtls_x509_crt_parse_file(
                &this->mbedCtx.srvCert, this->certPath.c_str()
            );
            if (taskTempRet != 0)
            {
                LOG_ERROR(
                    "Failed to init server cert, error code: {}", taskTempRet
                );
                cleanUp();
                return;
            }

            taskTempRet = mbedtls_pk_parse_keyfile(
                &this->mbedCtx.privKey,
                this->keyPath.c_str(),
                NULL,
                mbedtls_ctr_drbg_random,
                &this->mbedCtx.ctrDrbg
            );
            if (taskTempRet != 0)
            {
                LOG_ERROR(
                    "Failed to init server pKey, error code: {}", taskTempRet
                );
                cleanUp();
                return;
            }

            LOG_DEBUG("TLS cert & key init done.");

            LOG_INFO(
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
                LOG_ERROR(
                    "Failed while binding to addr, error code: {}", taskTempRet
                );
                cleanUp();
                return;
            }

            LOG_INFO("Bind to ssl://{}:{}", this->hostname, this->port);

            LOG_DEBUG("Setting up default TLS config...");
            taskTempRet = mbedtls_ssl_config_defaults(
                &this->mbedCtx.sslConfig,
                MBEDTLS_SSL_IS_SERVER,
                MBEDTLS_SSL_TRANSPORT_STREAM,
                MBEDTLS_SSL_PRESET_DEFAULT
            );
            if (taskTempRet != 0)
            {
                LOG_ERROR(
                    "Failed to set default TLS config, error code: {}",
                    taskTempRet
                );
                cleanUp();
                return;
            }

            LOG_DEBUG("Setting RNG and cert config...");
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

            LOG_DEBUG("Applying TLS config to sslCtx...");
            taskTempRet =
                mbedtls_ssl_setup(&this->sslCtx, &this->mbedCtx.sslConfig);
            if (taskTempRet != 0)
            {
                LOG_ERROR(
                    "Failed to apply TLS config, error code: {}", taskTempRet
                );
                cleanUp();
                return;
            }

            LOG_INFO("MQTT Broker TLS config applied.");
            LOG_INFO(
                "MQTT Broker TLS server initialized. Listening on ssl://{}:{}",
                this->hostname,
                this->port
            );

            this->listenLoop();
        }
        catch (const std::exception& err)
        {
            LOG_ERROR(
                "An unexpected error occurred in MQTT Broker loop: {}",
                err.what()
            );
            return;
        }
    }

    void Broker::listenLoop()
    {
        mbedtls_net_context* curClientFd = NULL;

        LOG_DEBUG("Waiting for client connection...");

        while (!this->shouldExit)
        {
            fd_set readFds;
            struct timeval timeVal;

            FD_ZERO(&readFds);
            FD_SET(this->netCtx.listenFd.fd, &readFds);

            timeVal.tv_sec = 1;
            timeVal.tv_usec = 0;

            if (curClientFd != NULL)
            {
                FD_SET(curClientFd->fd, &readFds);
            }

            int taskTempRet;

            taskTempRet = select(0, &readFds, NULL, NULL, &timeVal);

            if (taskTempRet < 0)
            {
                LOG_ERROR("Unexpected error running select()");
                break;
            }

            if (FD_ISSET(this->netCtx.listenFd.fd, &readFds))
            {
                LOG_DEBUG("New activity on listenFd detected.");
                // Incoming connection available
                if (curClientFd != NULL)
                {
                    mbedtls_net_context tempCtx;
                    mbedtls_net_init(&tempCtx);
                    mbedtls_net_accept(
                        &this->netCtx.listenFd, &tempCtx, NULL, 0, NULL
                    );
                    mbedtls_net_free(&tempCtx);
                    LOG_DEBUG(
                        "A client has already connected, closing new "
                        "connection..."
                    );
                }
                else
                {
                    LOG_DEBUG("Handshaking with new client...");
                    // so cur no client, new client connecting
                    taskTempRet = mbedtls_net_accept(
                        &this->netCtx.listenFd,
                        &this->netCtx.clientFd,
                        NULL,
                        0,
                        NULL
                    );

                    if (taskTempRet != 0)
                    {
                        LOG_ERROR(
                            "Unexpected error occurred in mbedtls_net_accept, "
                            "error "
                            "code: "
                            "{}",
                            taskTempRet
                        );
                        this->cleanUp();
                        return;
                    }
                    LOG_DEBUG("New connection accepted.");

                    mbedtls_ssl_set_bio(
                        &this->sslCtx,
                        &this->netCtx.clientFd,
                        mbedtls_net_send,
                        mbedtls_net_recv,
                        NULL
                    );

                    LOG_DEBUG("Performing handshake...");
                    bool handshakeResult = true;
                    while ((taskTempRet = mbedtls_ssl_handshake(&sslCtx)) != 0)
                    {
                        if (taskTempRet != MBEDTLS_ERR_SSL_WANT_READ &&
                            taskTempRet != MBEDTLS_ERR_SSL_WANT_WRITE)
                        {
                            LOG_ERROR(
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
                        continue;
                    }

                    LOG_DEBUG("SSL handshake completed.");

                    curClientFd = &this->netCtx.clientFd;
                }
            }

            if (curClientFd != NULL &&
                FD_ISSET(this->netCtx.clientFd.fd, &readFds))
            {
                unsigned char clientMsgBuffer[8192];
                memset(clientMsgBuffer, 0, sizeof(clientMsgBuffer));
                taskTempRet = mbedtls_ssl_read(
                    &sslCtx, clientMsgBuffer, sizeof(clientMsgBuffer) - 1
                );

                if (taskTempRet > 0)
                {
                    int packetLength = taskTempRet;
#ifdef _DEBUG
                    LOG_TRACE(
                        "New data received, bytes: {}, data:\n{}",
                        packetLength,
                        (char*)clientMsgBuffer
                    );
#endif
                    this->clientMsgBuffer.insert(this->clientMsgBuffer.end(), clientMsgBuffer, clientMsgBuffer + taskTempRet);
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
                            LOG_DEBUG("Client connection closed.");
                            break;
                        case MBEDTLS_ERR_NET_CONN_RESET:
                            LOG_ERROR("Client connection reset.");
                            break;
                        case MBEDTLS_ERR_NET_RECV_FAILED:
                            LOG_ERROR("Failed to run mbedtls_net_recv.");
                            break;
                        default:
                            LOG_DEBUG(
                                "mbedtls_ssl_read returned {}", taskTempRet
                            );
                            break;
                    }

                    LOG_DEBUG("Client disconnected, cleaning up...");
                    mbedtls_ssl_close_notify(&this->sslCtx);
                    mbedtls_net_free(&this->netCtx.clientFd);
                    mbedtls_ssl_session_reset(&this->sslCtx);

                    curClientFd = NULL;
                }
            }
        }

        mbedtls_ssl_close_notify(&this->sslCtx);
        mbedtls_ssl_session_reset(&this->sslCtx);
    }
}  // namespace AikariPLS::Components::MQTTBroker
