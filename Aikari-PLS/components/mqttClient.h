#pragma once

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <string>
#include <thread>

/*
 * MQTT Client will be initialized in MQTTBrokerHandler.cpp, when CONNECT pkt is
 * triggered Mounted on sharedIns
 */
namespace AikariPLS::Components::MQTTClient
{
    constexpr const char* pers = "Across the galaxy, we see the twilight";

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
        Client(const ClientLaunchArg& arg);

        ~Client();

        void startClientLoop();
        void cleanUp(bool ignoreThreadJoin);

        bool isConnectRetryDisabled = false;

        bool refreshHostRealIP();

       private:
        bool cleaned = false;

        std::atomic<bool> shouldExit = false;

        mbedtls_ssl_context sslCtx;
        NetContexts netCtxs;
        OtherMBedTLSInstances mbedCtx;

        ClientLaunchArg launchArg;
        std::string hostRealIP;

        std::unique_ptr<std::jthread> clientLoop;

        void resetConnection();

        void runConnLoop();
    };

    namespace ClientLifecycleController
    {
        void initAndMountClientIns(const ClientLaunchArg arg);

        void resetClientIns(bool noDisconnPkt);
    }  // namespace ClientLifecycleController
}  // namespace AikariPLS::Components::MQTTClient
