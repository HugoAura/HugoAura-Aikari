#include <Aikari-PLS/Aikari-PLS-Exports.h>
#include <Aikari-PLS/types/entrypoint.h>
#include <Aikari-Shared/infrastructure/logger.h>
#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>
#include <Aikari-Shared/infrastructure/telemetryShortFn.h>
#include <Aikari-Shared/types/itc/shared.h>

#include "components/infrastructure/config.h"
#include "components/mqtt/mqttBroker.h"
#include "components/mqtt/mqttClient.h"
#include "components/mqtt/mqttLifecycle.h"
#include "infrastructure/threadMsgHandler.h"
#include "init.h"
#include "lifecycle.h"

#define TELEMETRY_ACTION_CATEGORY "pls.entrypoint"

namespace LifecycleTypes = AikariPLS::Types::Lifecycle;

namespace AikariPLS::Exports
{
    extern AIKARIPLS_API AikariPLS::Types::Entrypoint::EntrypointRet main(
        const std::filesystem::path& aikariRootPath,
        const std::filesystem::path& certDirPath,
        std::shared_ptr<
            AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::MainToSubMessageInstance>>
            inputMessageQueue,
        const std::unordered_set<AikariShared::LoggerSystem::LOGGER_SINK>*
            loggerSinks
    )
    {
        auto& sharedInsManager =
            AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();
        auto& sharedQueuesManager =
            AikariPLS::Lifecycle::PLSSharedQueuesManager::getInstance();

        AikariShared::LoggerSystem::defaultLoggerSink = *loggerSinks;
        AikariShared::LoggerSystem::initLogger(
            "PLS", 37, 45
        );  // 37 = White text; 45 = Purple background
        LOG_INFO("[MODULE_INIT] Aikari Submodule PLS is launching...");
        Telemetry::addBreadcrumb(
            "default", "PLS is mounted", TELEMETRY_ACTION_CATEGORY, "info"
        );

        auto retMessageQueue = std::make_shared<
            AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::SubToMainMessageInstance>>();

        sharedQueuesManager.setVal(
            &AikariPLS::Types::Lifecycle::PLSSharedMsgQueues::inputMsgQueue,
            inputMessageQueue
        );

        sharedQueuesManager.setVal(
            &AikariPLS::Types::Lifecycle::PLSSharedMsgQueues::retMsgQueue,
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

        std::packaged_task<
            AikariPLS::Types::Infrastructure::Init::PLSInitResult()>
            plsInitTask(AikariPLS::Init::runPlsInit);

        auto plsInitFuture = plsInitTask.get_future();

        auto plsInitThread =
            std::make_unique<std::jthread>(std::move(plsInitTask));

        AikariPLS::Types::Entrypoint::EntrypointRet launchResult = {
            .success = true,
            .retMessageQueue = std::move(retMessageQueue),
            .plsInitThread = std::move(plsInitThread),
            .plsInitResultFuture = std::move(plsInitFuture)
        };

        return launchResult;
    };

    extern AIKARIPLS_API void onExit()
    {
        LOG_INFO("Cleaning up msg queue handlers...");
        Telemetry::addBreadcrumb(
            "default", "PLS is unmounting", TELEMETRY_ACTION_CATEGORY, "info"
        );

        auto& sharedInsMgr =
            AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();

        auto threadMsgHandlerIns = sharedInsMgr.getPtr(
            &LifecycleTypes::PLSSharedIns::threadMsgQueueHandler
        );
        if (threadMsgHandlerIns != nullptr)
        {
            Telemetry::addBreadcrumb(
                "default",
                "Destroying threadMsgHandlerIns",
                TELEMETRY_ACTION_CATEGORY,
                "debug"
            );
            threadMsgHandlerIns->manualDestroy();
        }

        auto mqttBrokerIns =
            sharedInsMgr.getPtr(&LifecycleTypes::PLSSharedIns::mqttBroker);
        if (mqttBrokerIns != nullptr)
        {
            Telemetry::addBreadcrumb(
                "default",
                "Destroying mqttBrokerIns",
                TELEMETRY_ACTION_CATEGORY,
                "debug"
            );
            mqttBrokerIns->cleanUp(false);
        }

        auto mqttClientIns =
            sharedInsMgr.getPtr(&LifecycleTypes::PLSSharedIns::mqttClient);
        if (mqttClientIns != nullptr)
        {
            Telemetry::addBreadcrumb(
                "default",
                "Destroying mqttClientIns",
                TELEMETRY_ACTION_CATEGORY,
                "debug"
            );
            mqttClientIns->cleanUp(false);
        }

        Telemetry::addBreadcrumb(
            "default", "PLS is unloaded", TELEMETRY_ACTION_CATEGORY, "info"
        );
        LOG_INFO("Clean up finished, module PLS is unloaded.");
        return;
    };
}  // namespace AikariPLS::Exports
