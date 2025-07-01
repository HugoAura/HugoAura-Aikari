#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace AikariPLS::Types::infrastructure
{
enum class MESSAGE_TYPES
{
    CONTROL_MESSAGE,
    WS_MESSAGE,
    DESTROY_MESSAGE
};

struct WebSocketInfo
{
    std::optional<bool> isBroadcast;
    std::optional<std::string> clientId;
};

struct InputMessageStruct
{
    std::string method;
    nlohmann::json data;
    MESSAGE_TYPES type;
    std::string fromModule;
    std::optional<std::string> eventId;
    std::optional<WebSocketInfo> wsInfo;
};

struct RetMessageStruct
{
    std::optional<bool> success;
    int code;
    MESSAGE_TYPES type;
    nlohmann::json data;
    std::optional<std::string> eventId;
    std::optional<WebSocketInfo> wsInfo;
};
}  // namespace AikariPLS::Types::infrastructure
