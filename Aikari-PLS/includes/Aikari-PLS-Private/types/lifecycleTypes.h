#pragma once

#include <Aikari-Launcher-Public/constants/lifecycle.h>
#include <Aikari-PLS-Private/types/components/mqtt.h>
#include <memory>
#include <minwindef.h>

#include "components/ruleSystem/rulesManager.h"

namespace AikariShared::Infrastructure::MessageQueue
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

namespace AikariPLS::Types::Lifecycle
{
    struct PLSSharedStates
    {
        AikariLauncherPublic::Constants::Lifecycle::APPLICATION_RUNTIME_MODES
            runtimeMode;

        static PLSSharedStates createDefault()
        {
            return { .runtimeMode = AikariLauncherPublic::Constants::Lifecycle::
                         APPLICATION_RUNTIME_MODES::NORMAL };
        };
    };

    struct PLSSharedIns
    {
        std::unique_ptr<
            AikariPLS::Infrastructure::MsgQueue::PLSThreadMsgQueueHandler>
            threadMsgQueueHandler;
        std::unique_ptr<AikariPLS::Components::MQTTBroker::Broker> mqttBroker;
        std::unique_ptr<AikariPLS::Components::MQTTClient::Client> mqttClient;
        std::unique_ptr<AikariPLS::Components::Rules::Manager> ruleMgr;
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
            AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::MainToSubMessageInstance>>
            inputMsgQueue;

        std::shared_ptr<
            AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::SubToMainMessageInstance>>
            retMsgQueue;

        static PLSSharedMsgQueues createDefault()
        {
            return {};
        }
    };
}  // namespace AikariPLS::Types::Lifecycle
