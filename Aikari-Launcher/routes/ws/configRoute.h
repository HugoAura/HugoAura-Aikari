#pragma once

#include <Aikari-Launcher-Private/types/components/wsTypes.h>

namespace AikariLauncherRoutes::WebSocket::Config
{
    AikariTypes::Components::WebSocket::ServerWSRep handleConfigMethods(
        const AikariTypes::Components::WebSocket::ClientWSMsg&
            clientDataIncoming,
        const std::vector<std::string>& methods
    );
}
