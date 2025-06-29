#include "pch.h"

#include <Aikari-PLS/Aikari-PLS-Exports.h>
#include <Aikari-PLS/types/entrypoint.h>
#include <Aikari-PLS/types/infrastructure/messageQueue.h>
#include <Aikari-Shared/infrastructure/SinglePointMessageQueue.hpp>
#include <Aikari-Shared/infrastructure/logger.h>
#include <Aikari-Shared/infrastructure/loggerMacro.h>

#include "infrastructure/threadMsgHandler.h"
#include "lifecycle.h"

namespace LifecycleTypes = AikariPLS::Types::lifecycle;

namespace AikariPLS::Exports
{
extern AIKARIPLS_API AikariPLS::Types::entrypoint::EntrypointRet main(
    const std::filesystem::path& aikariRootPath,
    const std::filesystem::path& certDirPath,
    std::shared_ptr<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariPLS::Types::infrastructure::InputMessageStruct>>
        inputMessageQueue
)
{
    auto& sharedInsManager =
        AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();

    AikariShared::LoggerSystem::initLogger(
        "PLS", 37, 45
    );  // 37 = White text; 45 = Purple background
    LOG_INFO("[MODULE_INIT] Aikari Submodule PLS is launching...");
    auto retMessageQueue = std::make_shared<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariPLS::Types::infrastructure::RetMessageStruct>>();

    auto threadMsgHandlerIns = std::make_shared<
        AikariPLS::Infrastructure::MsgQueue::PLSThreadMsgQueueHandler>(
        inputMessageQueue, retMessageQueue
    );

    sharedInsManager.setVal(
        &LifecycleTypes::PLSSharedIns::threadMsgQueueHandler,
        threadMsgHandlerIns
    );

    AikariPLS::Types::entrypoint::EntrypointRet launchResult = {
        .success = true, .retMessageQueue = retMessageQueue
    };

    return launchResult;
};

extern AIKARIPLS_API void onExit()
{
    LOG_INFO("Cleaning up msg queue handlers...");

    auto& sharedInsMgr =
        AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();

    auto threadMsgHandlerIns =
        sharedInsMgr.getVal(&LifecycleTypes::PLSSharedIns::threadMsgQueueHandler
        );
    threadMsgHandlerIns->manualDestroy();

    LOG_INFO("Clean up finished, module PLS is unloaded.");
    return;
};
}  // namespace AikariPLS::Exports
