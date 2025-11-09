#pragma once

#include <Aikari-Shared/virtual/IConfigPayload.h>
#include <nlohmann/json.hpp>

namespace AikariPLS::Types::Config
{
    struct PLSConfig : public AikariShared::VirtualIns::IConfigPayload
    {
        nlohmann::json rules;
        std::string module;
    };
}  // namespace AikariPLS::Types::Config
