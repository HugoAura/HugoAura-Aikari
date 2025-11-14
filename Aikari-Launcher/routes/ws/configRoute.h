#pragma once

#include <Aikari-Launcher-Public/types/components/wsTypes.h>

namespace AikariLauncherRoutes::WebSocket::Config
{
    AikariLauncherPublic::Types::Components::WebSocket::ServerWSRep
    handleConfigMethods(
        const AikariLauncherPublic::Types::Components::WebSocket::ClientWSMsg&
            clientDataIncoming,
        const std::vector<std::string>& methods
    );
}
