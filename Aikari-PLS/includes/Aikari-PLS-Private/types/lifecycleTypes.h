#pragma once

#include <memory>
#include <minwindef.h>

namespace AikariShared::infrastructure::MessageQueue
{
    template <typename T>
    class SinglePointMessageQueue;
}

namespace AikariShared::Types::InterThread
{
    struct MainToSubMessageInstance;
    struct SubToMainMessageInstance;
}  // namespace AikariShared::Types::InterThread

namespace AikariPLS::Infrastructure::MsgQueue
{
    class PLSThreadMsgQueueHandler;
}

namespace AikariPLS::Components
{
    namespace MQTTBroker
    {
        class Broker;
    }

    namespace MQTTClient
    {
        class Client;
    }
}  // namespace AikariPLS::Components

namespace AikariPLS::Types::lifecycle
{
    struct PLSSharedIns
    {
        std::unique_ptr<
            AikariPLS::Infrastructure::MsgQueue::PLSThreadMsgQueueHandler>
            threadMsgQueueHandler;
        std::unique_ptr<AikariPLS::Components::MQTTBroker::Broker> mqttBroker;
        std::unique_ptr<AikariPLS::Components::MQTTClient::Client> mqttClient;
        HMODULE hModuleIns;

        PLSSharedIns();
        static PLSSharedIns createDefault();
        ~PLSSharedIns();

        PLSSharedIns(PLSSharedIns&&) noexcept;
        PLSSharedIns& operator=(PLSSharedIns&&) noexcept;
    };

    struct PLSSharedMsgQueues
    {
        std::shared_ptr<
            AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::MainToSubMessageInstance>>
            inputMsgQueue;

        std::shared_ptr<
            AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::SubToMainMessageInstance>>
            retMsgQueue;

        static PLSSharedMsgQueues createDefault()
        {
            return {};
        }
    };
}  // namespace AikariPLS::Types::lifecycle
