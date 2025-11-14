#include "./dispatcher.h"

#include <Aikari-Launcher-Public/constants/ws/errors.h>
#include <Aikari-PLS/types/constants/ws/config.h>
#include <Aikari-Shared/utils/string.h>

#include "../../routes/ws/configRoute.h"

namespace launcherWsConstants = AikariLauncherPublic::Constants::WebSocket;

namespace AikariPLS::Components::WebSocket::Dispatcher
{
    AikariShared::Types::InterThread::SubToMainWebSocketMessageCore
    dispatchWsMsg(
        const AikariShared::Types::InterThread::MainToSubWebSocketMessageCore&
            srcMsg
    )
    {
        auto methodVec = AikariShared::Utils::String::split(srcMsg.method, '.');
        auto& rootMethod = methodVec.at(0);

        if (rootMethod ==
            AikariPLS::Types::Constants::WebSocket::Config::Rules::_PREFIX)
        {
            return AikariPLS::Routes::WebSocket::Config::handleConfigMsg(
                srcMsg, methodVec
            );
        }

        return { .code = AikariLauncherPublic::Constants::WebSocket::Errors::
                     Codes::METHOD_NOT_FOUND,
                 .success = false,
                 .data = { { "message", "Method not found" },
                           { "layer", "plsDispatcher" } } };
    };
}  // namespace AikariPLS::Components::WebSocket::Dispatcher
