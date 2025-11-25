#pragma once

#include <optional>
#include <string>

namespace AikariPLS::Types::Infrastructure::Init
{
    struct PLSInitResult
    {
        bool isSuccess;
        std::optional<std::string> message;
    };
}  // namespace AikariPLS::Types::Infrastructure::Init
