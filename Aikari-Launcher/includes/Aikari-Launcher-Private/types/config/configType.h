#pragma once

#include <Aikari-Shared/virtual/IConfigPayload.h>
#include <string>

namespace AikariTypes::config
{
    struct AikariConfig : public AikariShared::virtualIns::IConfigPayload
    {
        int wsPreferPort;
        struct TLSConfig
        {
            bool regenWsCertNextLaunch;
        } tls;
        std::string module = "launcher";
    };
    // to_json or from_json refer to comp/config.cpp
}  // namespace AikariTypes::config
