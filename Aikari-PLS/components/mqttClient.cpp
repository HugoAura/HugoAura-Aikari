#include "mqttClient.h"

#define CUSTOM_LOG_HEADER "[MQTT Client]"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/utils/network.h>
#include <Aikari-Shared/utils/windows.h>
#include <WinSock2.h>
#include <exception>

#include "../lifecycle.h"
#include "../resource.h"

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

        CUSTOM_LOG_DEBUG("Cleaning up MQTT Client context...");
        this->cleaned = true;
        this->shouldExit = true;
        if (this->clientLoop->joinable() && !ignoreThreadJoin)
        {
            this->clientLoop->join();
        }
        mbedtls_net_free(&this->netCtxs.serverFd);
        mbedtls_x509_crt_free(&this->mbedCtx.caCrt);
        mbedtls_ssl_free(&this->sslCtx);
        mbedtls_ssl_config_free(&this->mbedCtx.sslConfig);
        mbedtls_ctr_drbg_free(&this->mbedCtx.ctrDrbg);
        mbedtls_entropy_free(&this->mbedCtx.entropyCtx);
    };

    void Client::startClientLoop()
    {
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
                (const unsigned char*)pers,
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
                HINSTANCE hIns = sharedIns.getVal(
                    &AikariPLS::Types::lifecycle::PLSSharedIns::hModuleIns
                );
                auto getCrtPtrRet =
                    AikariShared::utils::windows::rc::loadStringResource<char>(
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
                &this->mbedCtx.sslConfig, &this->mbedCtx.caCrt, NULL
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
            return;
        }
    };

    bool Client::refreshHostRealIP()
    {
        auto dnsQueryResult =
            AikariShared::utils::network::DNS::getDNSARecordResult(
                this->launchArg.targetHost
            );
        if (dnsQueryResult.empty())
        {
            return false;
        }

        this->hostRealIP = dnsQueryResult.at(0);
        return true;
    }

    void Client::resetConnection()
    {
        mbedtls_ssl_close_notify(&this->sslCtx);
        mbedtls_net_free(&this->netCtxs.serverFd);
        mbedtls_ssl_session_reset(&this->sslCtx);
        // this->connection.reset...
    };

    void Client::runConnLoop()
    {
        CUSTOM_LOG_DEBUG("Ready to connect to the server.");

        bool isConnectionActive = false;

        while (!this->shouldExit)
        {
            int taskTempRet = 0;

            do
            {
                if (!isConnectionActive)
                {
                    if (this->isConnectRetryDisabled)
                    {
                        break;  // jump out from do while (false)
                    }

                    // handshake using config
                    CUSTOM_LOG_INFO(
                        "Connecting to ssl://{}:{}",
                        this->launchArg.targetHost,
                        this->launchArg.targetPort
                    );
                    taskTempRet = mbedtls_net_connect(
                        &this->netCtxs.serverFd,
                        this->hostRealIP.c_str(),
                        std::to_string(this->launchArg.targetPort).c_str(),
                        MBEDTLS_NET_PROTO_TCP
                    );
                    if (taskTempRet != 0)
                    {
                        CUSTOM_LOG_ERROR(
                            "Failed connecting to server, error code: {}",
                            taskTempRet
                        );
                        break;
                    };

                    CUSTOM_LOG_DEBUG("Setting bio...");
                    mbedtls_ssl_set_bio(
                        &this->sslCtx,
                        &this->netCtxs.serverFd,
                        mbedtls_net_send,
                        mbedtls_net_recv,
                        NULL
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
                    taskTempRet = mbedtls_ssl_get_verify_result(&this->sslCtx);
                    if (taskTempRet != 0)
                    {
                        char verifyResultBuffer[512] = {};
                        mbedtls_x509_crt_verify_info(
                            verifyResultBuffer,
                            sizeof(verifyResultBuffer),
                            "",
                            taskTempRet
                        );

                        LOG_ERROR(
                            "Failed to verify server certificate, error: {}",
                            verifyResultBuffer
                        );
                        this->resetConnection();
                        break;
                    }
                    CUSTOM_LOG_DEBUG("Certificate verified.");

                    CUSTOM_LOG_INFO("Connection established.");
                    isConnectionActive = true;
                }
            } while (false);

            fd_set readFds;
            struct timeval timeVal;

            FD_ZERO(&readFds);
            FD_SET(this->netCtxs.serverFd.fd, &readFds);

            timeVal.tv_sec = 1;
            timeVal.tv_usec = 0;

            taskTempRet = select(0, &readFds, nullptr, nullptr, &timeVal);

            if (taskTempRet < 0)
            {
                CUSTOM_LOG_ERROR("Unexpected error running select()");
                break;
            }

            do
            {
                if (taskTempRet > 0 &&
                    FD_ISSET(this->netCtxs.serverFd.fd, &readFds) &&
                    isConnectionActive)
                {
                    unsigned char buffer[4096] = {};
                    taskTempRet = mbedtls_ssl_read(
                        &this->sslCtx, buffer, sizeof(buffer) - 1
                    );

                    // Begin ssl_read ret switch
                    if (taskTempRet > 0)
                    {
                        std::string strIncomingData(
                            reinterpret_cast<char*>(buffer), taskTempRet
                        );
#ifdef _DEBUG
                        CUSTOM_LOG_TRACE(
                            "New data from server: {}", strIncomingData
                        );
#endif
                        // data from server, to be processed
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
                        this->resetConnection();

                        isConnectionActive = false;
                    }
                    // End ssl_read ret switch
                }  // LEVEL = taskRet > 0 && FDISSET
            } while (false);
        }

        mbedtls_ssl_close_notify(&this->sslCtx);
        mbedtls_ssl_session_reset(&this->sslCtx);
        CUSTOM_LOG_INFO("Client stopped.");
    }

    namespace ClientLifecycleController
    {
        void initAndMountClientIns(const ClientLaunchArg arg)
        {
            CUSTOM_LOG_INFO("<Controller> Creating MQTT Client instance...");

            auto& sharedIns =
                AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();

            auto clientInsOldPtr = sharedIns.getPtr(
                &AikariPLS::Types::lifecycle::PLSSharedIns::mqttClient
            );
            if (clientInsOldPtr != nullptr)
            {
                resetClientIns(true);
            }

            auto clientInsPtr = std::make_unique<Client>(arg);
            sharedIns.setPtr(
                &AikariPLS::Types::lifecycle::PLSSharedIns::mqttClient,
                std::move(clientInsPtr)
            );
        };

        void resetClientIns(bool noDisconnPkt)
        {
            CUSTOM_LOG_INFO("<Controller> Resetting MQTT Client instance...");

            auto& sharedIns =
                AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();
            auto clientInsPtr = sharedIns.getPtr(
                &AikariPLS::Types::lifecycle::PLSSharedIns::mqttClient
            );
            if (clientInsPtr != nullptr)
            {
                if (!noDisconnPkt)
                {
                    // TODO: Send MQTT DISCONNECT pkt
                }
            }
            sharedIns.resetPtr(
                &AikariPLS::Types::lifecycle::PLSSharedIns::mqttClient
            );
        };
    }  // namespace ClientLifecycleController

}  // namespace AikariPLS::Components::MQTTClient
