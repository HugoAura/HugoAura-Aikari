#pragma once

#include <Aikari-Shared/infrastructure/telemetry.h>
#include <string>

namespace Telemetry
{
    inline void addBreadcrumb(
        const std::string& type,
        const std::string& msg,
        const std::string& category,
        const std::string& level
    )
    {
        static auto& telemetryManager = AikariShared::Infrastructure::
            Telemetry::TelemetryManager::getInstance();
        telemetryManager.addBreadcrumb(type, msg, category, level);
    };

    inline void sendEventStr(
        const sentry_level_e level,
        const std::string& moduleName,
        const std::string& message
    )
    {
        static auto& telemetryManager = AikariShared::Infrastructure::
            Telemetry::TelemetryManager::getInstance();
        telemetryManager.sendMsgEvent(level, moduleName, message);
    };
}  // namespace Telemetry
