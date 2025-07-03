#pragma once

#include <string>

namespace AikariShared::virtualIns
{
    struct IConfigPayload
    {
        std::string module;

        virtual ~IConfigPayload() = default;
    };
}  // namespace AikariShared::virtualIns
