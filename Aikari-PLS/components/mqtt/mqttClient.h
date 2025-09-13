#pragma once

#include <Aikari-Shared/infrastructure/queue/PoolQueue.hpp>
#include <atomic>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mutex>
#include <string>
#include <thread>

#include "mqttClientHandler.h"

/*
 * MQTT Client will be initialized in MQTTBrokerHandler.cpp, when CONNECT pkt is
 * triggered
 * Mounted on sharedIns
 */
namespace AikariPLS::Components::MQTTClient
{
    constexpr const char* pers =
        "Across every possibility, we see the infinity";

    struct ClientLaunchArg
    {
        std::string targetHost;
        int targetPort;

        std::string clientId;
        std::string username;
        std::string password;
        std::uint32_t keepAliveSec;
    };

    struct NetContexts
    {
        mbedtls_net_context serverFd;
    };

    struct OtherMBedTLSInstances
    {
        mbedtls_entropy_context entropyCtx;
        mbedtls_ctr_drbg_context ctrDrbg;
        mbedtls_ssl_config sslConfig;
        mbedtls_x509_crt caCrt;
    };

    class Client
    {
       public:
        explicit Client(const ClientLaunchArg& arg);

        ~Client();

        void startClientLoop();
        void cleanUp(bool ignoreThreadJoin);

        bool isConnectRetryDisabled = false;

        bool refreshHostRealIP();

        void startSendQueueWorker();

       private:
        bool cleaned = false;

        std::atomic<bool> shouldExit = false;
        std::atomic<bool> pendingExit = false;
        std::mutex sslCtxLock;

        mbedtls_ssl_context sslCtx;
        NetContexts netCtxs;
        OtherMBedTLSInstances mbedCtx;

        std::unique_ptr<
            AikariPLS::Components::MQTTClient::Class::MQTTClientConnection>
            connection;
        std::unique_ptr<AikariShared::infrastructure::MessageQueue::PoolQueue<
            AikariPLS::Types::mqttMsgQueue::FlaggedPacket>>
            sendThreadPool;

        void initSendThreadPool();

        const size_t sendThreadCount = 4;

        uint8_t retryTimes = 0;
        const uint8_t maxRetry = 3;

        std::unique_ptr<std::jthread> sendQueueWorker;

        ClientLaunchArg launchArg;
        std::string hostRealIP;

        std::unique_ptr<std::jthread> clientLoop;

        bool isConnectionActive = false;

        void resetConnection();

        void runConnLoop();
    };

    namespace ClientLifecycleController
    {
        void initAndMountClientIns(const ClientLaunchArg arg);

        void resetClientIns(bool noDisconnPkt);
    }  // namespace ClientLifecycleController
}  // namespace AikariPLS::Components::MQTTClient
