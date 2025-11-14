#pragma once

#include <Aikari-Shared/types/itc/shared.h>

namespace AikariPLS::Routes::WebSocket::Config
{
    AikariShared::Types::InterThread::SubToMainWebSocketMessageCore
    handleConfigMsg(
        const AikariShared::Types::InterThread::MainToSubWebSocketMessageCore&
            clientDataIncoming,
        const std::vector<std::string>& methodVec
    );
}
