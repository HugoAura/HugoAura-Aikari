#pragma once

#include <Aikari-Launcher-Public/types/components/wsTypes.h>

namespace AikariLauncher::Routes::WebSocket::Config
{
    AikariLauncher::Public::Types::Components::WebSocket::ServerWSRep
    handleConfigMethods(
        const AikariLauncher::Public::Types::Components::WebSocket::ClientWSMsg&
            clientDataIncoming,
        const std::vector<std::string>& methods
    );
}
