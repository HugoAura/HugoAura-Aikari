#pragma once

#include <Aikari-Launcher-Private/types/components/wsTypes.h>

namespace AikariLauncherRoutes::WebSocket::Basic
{
    AikariTypes::Components::WebSocket::ServerWSRep handleBasicMethods(
        const AikariTypes::Components::WebSocket::ClientWSMsg&
            clientDataIncoming,
        const std::vector<std::string>& methods
    );
}
