#pragma once

#include <string>
#include <vector>

namespace AikariShared::utils::network
{
    namespace DNS
    {
        std::vector<std::string> getDNSARecordResult(
            const std::string& targetDomain
        );
    }
}  // namespace AikariShared::utils::network
