#include "pch.h"

#include "wsMsgHandler.h"

#include <Aikari-Shared/infrastructure/MessageQueue.hpp>
#include <nlohmann/json.hpp>

namespace wsTypes = AikariTypes::components::websocket;
namespace messageQueue = AikariShared::infrastructure::MessageQueue;

namespace AikariLauncherComponents::AikariWebSocketHandler
{
wsTypes::MODULES getMsgModule(std::string moduleStr)
{
    if (moduleStr == "launcher")
    {
        return wsTypes::MODULES::LAUNCHER;
    }
    else if (moduleStr == "pls")
    {
        return wsTypes::MODULES::PLS;
    }
    else if (moduleStr == "auraUpdate")
    {
        return wsTypes::MODULES::AUA;
    }
    else
    {
        return wsTypes::MODULES::UNKNOWN;
    }
}

namespace RepTemplates
{
static const nlohmann::json moduleNotFoundJSON = {
    { "message", "Requested module not found" }
};

static const wsTypes::ServerWSRep moduleNotFoundRep = {
    .code = 9000, .success = false, .data = moduleNotFoundJSON
};  // namespace RepTemplates
}  // namespace RepTemplates

void handleTask(
    wsTypes::ClientWSTask task,
    std::shared_ptr<
        messageQueue::SinglePointMessageQueue<wsTypes::ServerWSTaskRet>>
        retMsgQueue
)
{
    wsTypes::MODULES msgModule = getMsgModule(task.content.module);
    switch (msgModule)
    {
        case wsTypes::MODULES::LAUNCHER:
        {
            // TO DO: Handle msg for launcher
            break;
        }
        case wsTypes::MODULES::UNKNOWN:
        {
            wsTypes::ServerWSTaskRet retVal;
            wsTypes::ServerWSRep repContent;
            repContent = RepTemplates::moduleNotFoundRep;
            repContent.eventId = task.content.eventId;
            retVal.clientId = task.clientId;
            retVal.result = repContent;
            retMsgQueue->push(std::move(retVal));
            break;
        }
    }
}
}  // namespace AikariLauncherComponents::AikariWebSocketHandler
