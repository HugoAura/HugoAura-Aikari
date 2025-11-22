#pragma once

#include <Aikari-Shared/types/constants/version.h>

namespace AikariShared::Constants::Telemetry
{
    constexpr const char* SENTRY_DSN =
        "https://9573dcdcb8e8e756d925cb28d962e773@telemetry.delta.ooo/2";
    inline const std::string SENTRY_REL =
        "hugoaura-aikari@" + AikariShared::Constants::Version::Version;
}  // namespace AikariShared::Constants::Telemetry
