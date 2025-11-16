#pragma once

#include <Aikari-Launcher-Public/types/components/wsTypes.h>

namespace AikariLauncher::Routes::WebSocket::Basic
{
    AikariLauncher::Public::Types::Components::WebSocket::ServerWSRep
    handleBasicMethods(
        const AikariLauncher::Public::Types::Components::WebSocket::ClientWSMsg&
            clientDataIncoming,
        const std::vector<std::string>& methods
    );
}
