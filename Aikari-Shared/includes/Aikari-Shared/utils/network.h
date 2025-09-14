#pragma once

#include <string>
#include <vector>

namespace AikariShared::Utils::Network
{
    namespace DNS
    {
        std::vector<std::string> getDNSARecordResult(
            const std::string& targetDomain
        );
    }
}  // namespace AikariShared::Utils::Network
