#pragma once

#include <Aikari-Shared/virtual/IConfigPayload.h>
#include <string>

namespace AikariTypes::Config
{
    struct AikariConfig : public AikariShared::VirtualIns::IConfigPayload
    {
        int wsPreferPort;
        struct TLSConfig
        {
            bool regenWsCertNextLaunch;
        } tls;
        std::string module = "launcher";
    };
    // to_json or from_json refer to comp/config.cpp
}  // namespace AikariTypes::Config
