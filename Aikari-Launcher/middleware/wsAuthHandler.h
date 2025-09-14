#pragma once

#include <Aikari-Launcher-Private/types/middleware/websocket.h>
#include <string>

namespace AikariLauncherMiddlewares::WebSocket
{
    AikariTypes::Middleware::WebSocket::WEBSOCKET_AUTH_STATUS handleClientAuth(
        const std::string& uri, const std::string& authToken
    );
}
