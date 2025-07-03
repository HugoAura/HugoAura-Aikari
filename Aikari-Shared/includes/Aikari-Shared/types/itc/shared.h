#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <variant>

namespace AikariShared::Types::InterThread
{
    typedef std::string eventId;

    enum class MESSAGE_TYPES
    {
        CONTROL_MESSAGE,
        CONTROL_MESSAGE_REPLY,
        WS_MESSAGE,
        DESTROY_MESSAGE
    };

    struct WebSocketInfo
    {
        std::optional<bool> isBroadcast;
        std::optional<std::string> clientId;
    };

    // Main -> Sub

    struct MainToSubControlMessage
    {
        std::string method;
        nlohmann::json data;
        std::string fromModule = "launcher";
        eventId eventId;
    };

    struct MainToSubControlReplyMessage
    {
        nlohmann::json data;
        std::string fromModule = "launcher";
        eventId eventId;
    };

    struct MainToSubWebSocketMessage
    {
        std::string method;
        nlohmann::json data;
        eventId eventId;
        WebSocketInfo wsInfo;
    };

    struct MainToSubDestroyMessage
    {
        bool destory = true;
    };

    struct MainToSubMessageInstance
    {
        MESSAGE_TYPES type;
        std::variant<
            MainToSubControlMessage,
            MainToSubControlReplyMessage,
            MainToSubWebSocketMessage,
            MainToSubDestroyMessage>
            msg;
    };

    // Sub -> Main

    struct SubToMainControlMessage
    {
        std::string method;
        nlohmann::json data;
        std::string fromModule;
        eventId eventId;
    };

    struct SubToMainControlReplyMessage
    {
        nlohmann::json data;
        std::string fromModule;
        eventId eventId;
    };

    struct SubToMainWebSocketReply
    {
        bool success;
        int code;
        nlohmann::json data;
        eventId eventId;
        WebSocketInfo wsInfo;
    };

    struct SubToMainDestroyMessage
    {
        bool destory = true;
    };

    struct SubToMainMessageInstance
    {
        MESSAGE_TYPES type;
        std::variant<
            SubToMainControlMessage,
            SubToMainControlReplyMessage,
            SubToMainWebSocketReply,
            SubToMainDestroyMessage>
            msg;
    };
}  // namespace AikariShared::Types::InterThread
