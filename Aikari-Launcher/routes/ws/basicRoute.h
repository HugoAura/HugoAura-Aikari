#pragma once

#include <Aikari-Launcher-Public/types/components/wsTypes.h>

namespace AikariLauncherRoutes::WebSocket::Basic
{
    AikariLauncherPublic::Types::Components::WebSocket::ServerWSRep
    handleBasicMethods(
        const AikariLauncherPublic::Types::Components::WebSocket::ClientWSMsg&
            clientDataIncoming,
        const std::vector<std::string>& methods
    );
}
