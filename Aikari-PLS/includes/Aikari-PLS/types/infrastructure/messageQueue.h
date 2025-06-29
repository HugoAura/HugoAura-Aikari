#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace AikariPLS::Types::infrastructure
{
enum class MESSAGE_TYPES
{
    CONTROL_MESSAGE,
    WS_MESSAGE
};

struct InputMessageStruct
{
    std::string method;
    nlohmann::json data;
    std::string eventId;
    MESSAGE_TYPES type;
    std::string fromModule;
};

struct RetMessageStruct
{
    bool success;
    int code;
    std::string eventId;
    MESSAGE_TYPES type;
    nlohmann::json data;
};
}  // namespace AikariPLS::Types::infrastructure
