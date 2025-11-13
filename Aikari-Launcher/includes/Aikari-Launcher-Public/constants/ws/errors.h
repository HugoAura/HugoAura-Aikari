#pragma once

namespace AikariLauncherPublic::Constants::WebSocket::Errors
{
    constexpr const char* INVALID_ARG = "E_INVALID_ARG";
    constexpr const char* MODULE_NOT_FOUND = "E_MODULE_NOT_FOUND";
    constexpr const char* METHOD_NOT_FOUND = "E_METHOD_NOT_FOUND";

    constexpr const char* MODULE_CONNECTION_BROKEN_VARIANT_QUEUE =
        "E_MODULE_QUEUE_CONN_BROKEN";

    namespace Codes
    {
        constexpr int UNEXPECTED_ERROR = -5125;

        constexpr int AUTH_FAILURE = -8000;

        constexpr int INVALID_ARG = -2;
        constexpr int MODULE_NOT_FOUND = -9000;
        constexpr int METHOD_NOT_FOUND = -1;
        constexpr int MODULE_CONNECTION_BROKEN = -9001;

        constexpr int GENERIC_FAILURE = -3;
        constexpr int RUNTIME_FAILURE = -4;
    }  // namespace Codes
}  // namespace AikariLauncherPublic::Constants::WebSocket::Errors
