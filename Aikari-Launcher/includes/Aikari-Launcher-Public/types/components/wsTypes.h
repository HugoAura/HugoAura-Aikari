#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace ix
{
    class WebSocket;
}

namespace AikariLauncher::Public::Types::Components::WebSocket
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
        std::optional<int> code;
        std::optional<std::string> eventId;
        std::optional<bool> success;
        std::optional<std::string> method; // for wsPush
        nlohmann::json data;
    };

    struct ServerWSTaskRet
    {
        ServerWSRep result;
        std::optional<std::string> clientId;
        bool isBroadcast = false;
    };
};  // namespace AikariLauncher::Public::Types::Components::WebSocket
