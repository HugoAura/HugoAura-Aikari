#pragma once

#include <vector>

namespace AikariShared::Constants::Utils
{
    namespace Network
    {
        namespace DNS
        {
            inline const std::vector<std::string> dohQueryHosts = {
                "https://223.5.5.5/resolve", "https://doh.360.cn/resolve"
            };
        }
    }  // namespace Network
}  // namespace AikariShared::Constants::Utils
