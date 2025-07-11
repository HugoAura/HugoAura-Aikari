#pragma once

#include <Aikari-Shared/infrastructure/queue/PoolQueue.hpp>
#include <atomic>
#include <filesystem>
#include <functional>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/pk.h>
#include <mbedtls/ssl.h>
#include <memory>
#include <string>
#include <thread>

#include "mqttBrokerHandler.h"

namespace AikariPLS::Components::MQTTBroker
{
    constexpr const char* pers = "Caused by playing Gensh1n 1mpact";

    struct BrokerLaunchArg
    {
        std::filesystem::path certPath;
        std::filesystem::path keyPath;
        std::string hostname;
        int port;
    };

    struct NetContexts
    {
        mbedtls_net_context listenFd;
        mbedtls_net_context clientFd;
    };

    struct OtherMBedTLSInstances
    {
        mbedtls_entropy_context entropyCtx;
        mbedtls_ctr_drbg_context ctrDrbg;
        mbedtls_ssl_config sslConfig;
        mbedtls_x509_crt srvCert;
        mbedtls_pk_context privKey;
    };

    class Broker
    {
       public:
        Broker(BrokerLaunchArg arg);

        ~Broker();

        void startTlsServerLoop(BrokerLaunchArg arg);
        void cleanUp();

       private:
        bool cleaned = false;

        std::atomic<bool> shouldExit = false;

        mbedtls_ssl_context sslCtx;
        NetContexts netCtx;
        OtherMBedTLSInstances mbedCtx;

        std::vector<async_mqtt::packet_variant> pendingPkts;

        std::unique_ptr<
            AikariPLS::Components::MQTTBroker::Class::MQTTBrokerConnection>
            connection;
        std::unique_ptr<AikariShared::infrastructure::MessageQueue::PoolQueue<
            std::stringstream>>
            threadPool;

        size_t threadCount = 8;

        std::string hostname;
        int port;
        std::string certPath;
        std::string keyPath;

        std::unique_ptr<std::jthread> serverLoop;

        std::function<void(std::stringstream)> handleRecv;

        void resetCurConnection();

        void processConnectPackage(std::string& package);

        void listenLoop();
    };
}  // namespace AikariPLS::Components::MQTTBroker
