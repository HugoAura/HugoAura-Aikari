#pragma once

#include <Aikari-Shared/types/itc/shared.h>

namespace AikariPLS::Components::WebSocket::Dispatcher
{
    AikariShared::Types::InterThread::SubToMainWebSocketMessageCore
    dispatchWsMsg(
        const AikariShared::Types::InterThread::MainToSubWebSocketMessageCore&
            srcMsg
    );
}
