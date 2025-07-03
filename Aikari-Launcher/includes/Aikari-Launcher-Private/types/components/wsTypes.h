#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace ix
{
    class WebSocket;
}

namespace AikariTypes::components::websocket
{
    enum class MODULES
    {
        LAUNCHER,
        PLS,
        AUA,
        UNKNOWN
    };

    struct ClientWSMsg
    {
        std::string module;
        std::string eventId;
        std::string method;
        nlohmann::json data;
    };

    struct ClientWSTask
    {
        ClientWSMsg content;
        std::string clientId;
    };

    struct ServerWSRep
    {
        int code;
        std::optional<std::string> eventId;
        std::optional<bool> success;
        nlohmann::json data;
    };

    struct ServerWSTaskRet
    {
        ServerWSRep result;
        std::optional<std::string> clientId;
        bool isBroadcast = false;
    };
};  // namespace AikariTypes::components::websocket
