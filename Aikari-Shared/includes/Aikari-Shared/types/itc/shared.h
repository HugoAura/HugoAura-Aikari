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
        bool destroy = true;
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
        std::optional<bool> success;
        std::optional<int> code;
        nlohmann::json data;
        std::optional<std::string> method;
        std::optional<eventId> eventId;
        WebSocketInfo wsInfo;
    };

    struct SubToMainDestroyMessage
    {
        bool destroy = true;
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

    // Msg Cores
    struct MainToSubWebSocketMessageCore
    {
        std::string method;
        nlohmann::json data;
    };

    struct SubToMainWebSocketMessageCore
    {
        std::optional<int> code;
        std::optional<bool> success;
        nlohmann::json data;
        std::optional<std::string> method;
    };
}  // namespace AikariShared::Types::InterThread
