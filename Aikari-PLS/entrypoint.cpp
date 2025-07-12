#include <Aikari-PLS/Aikari-PLS-Exports.h>
#include <Aikari-PLS/types/entrypoint.h>
#include <Aikari-Shared/infrastructure/logger.h>
#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>
#include <Aikari-Shared/types/itc/shared.h>

#include "components/mqttBroker.h"
#include "components/mqttClient.h"
#include "infrastructure/threadMsgHandler.h"
#include "init.h"
#include "lifecycle.h"

namespace LifecycleTypes = AikariPLS::Types::lifecycle;

namespace AikariPLS::Exports
{
    extern AIKARIPLS_API AikariPLS::Types::entrypoint::EntrypointRet main(
        const std::filesystem::path& aikariRootPath,
        const std::filesystem::path& certDirPath,
        std::shared_ptr<
            AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::MainToSubMessageInstance>>
            inputMessageQueue
    )
    {
        auto& sharedInsManager =
            AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();
        auto& sharedQueuesManager =
            AikariPLS::Lifecycle::PLSSharedQueuesManager::getInstance();

        AikariShared::LoggerSystem::initLogger(
            "PLS", 37, 45
        );  // 37 = White text; 45 = Purple background
        LOG_INFO("[MODULE_INIT] Aikari Submodule PLS is launching...");
        auto retMessageQueue = std::make_shared<
            AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::SubToMainMessageInstance>>();

        sharedQueuesManager.setVal(
            &AikariPLS::Types::lifecycle::PLSSharedMsgQueues::inputMsgQueue,
            inputMessageQueue
        );

        sharedQueuesManager.setVal(
            &AikariPLS::Types::lifecycle::PLSSharedMsgQueues::retMsgQueue,
            retMessageQueue
        );  // not using setPtr for shared_ptr

        {
            auto threadMsgHandlerIns = std::make_unique<
                AikariPLS::Infrastructure::MsgQueue::PLSThreadMsgQueueHandler>(
                inputMessageQueue.get(), retMessageQueue.get(), "PLS"
            );

            sharedInsManager.setPtr(
                &LifecycleTypes::PLSSharedIns::threadMsgQueueHandler,
                std::move(threadMsgHandlerIns)
            );
        }

        auto plsInitThread =
            std::make_unique<std::jthread>(AikariPLS::Init::runPlsInit);

        AikariPLS::Types::entrypoint::EntrypointRet launchResult = {
            .success = true,
            .retMessageQueue = std::move(retMessageQueue),
            .plsRuntimeThread = std::move(plsInitThread)
        };

        return launchResult;
    };

    extern AIKARIPLS_API void onExit()
    {
        LOG_INFO("Cleaning up msg queue handlers...");

        auto& sharedInsMgr =
            AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();

        auto threadMsgHandlerIns = sharedInsMgr.getPtr(
            &LifecycleTypes::PLSSharedIns::threadMsgQueueHandler
        );
        threadMsgHandlerIns->manualDestroy();

        auto mqttBrokerIns =
            sharedInsMgr.getPtr(&LifecycleTypes::PLSSharedIns::mqttBroker);
        if (mqttBrokerIns != nullptr)
        {
            mqttBrokerIns->cleanUp(false);
        }

        auto mqttClientIns =
            sharedInsMgr.getPtr(&LifecycleTypes::PLSSharedIns::mqttClient);
        if (mqttClientIns != nullptr)
        {
            mqttClientIns->cleanUp(false);
        }

        LOG_INFO("Clean up finished, module PLS is unloaded.");
        return;
    };
}  // namespace AikariPLS::Exports
