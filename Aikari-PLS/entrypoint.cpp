#include "pch.h"

#include <Aikari-PLS/Aikari-PLS-Exports.h>
#include <Aikari-PLS/types/entrypoint.h>
#include <Aikari-PLS/types/infrastructure/messageQueue.h>
#include <Aikari-Shared/infrastructure/MessageQueue.hpp>
#include <Aikari-Shared/infrastructure/logger.h>
#include <Aikari-Shared/infrastructure/loggerMacro.h>

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
    AikariShared::LoggerSystem::initLogger("PLS", 37, 45);
    LOG_INFO("[MODULE_INIT] Aikari Submodule PLS is launching...");
    auto retMessageQueue = std::make_shared<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariPLS::Types::infrastructure::RetMessageStruct>>();

    AikariPLS::Types::entrypoint::EntrypointRet launchResult = {
        .success = true, .retMessageQueue = retMessageQueue
    };

    return launchResult;
};
}  // namespace AikariPLS::Exports
