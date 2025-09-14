#pragma once

#include <string>

namespace AikariShared::VirtualIns
{
    struct IConfigPayload
    {
        std::string module;

        virtual ~IConfigPayload() = default;
    };
}  // namespace AikariShared::VirtualIns
