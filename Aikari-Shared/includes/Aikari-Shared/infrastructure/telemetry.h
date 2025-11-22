#pragma once

#include <filesystem>
#include <sentry.h>
#include <string>

namespace AikariShared::Infrastructure::Telemetry
{
    inline const std::string defaultTelemetryId =
        "00000000-0000-0000-0000-EEEEEEEEEEEE";

    class TelemetryManager
    {
       public:
        static TelemetryManager& getInstance();

        std::string sentryDsn;
        std::string sentryRel;
        std::filesystem::path sentryDatabasePath;
        std::filesystem::path telemetryIdPath;
        std::filesystem::path telemetryEnableFlagPath;

        std::string telemetryId = Telemetry::defaultTelemetryId;
        bool isTelemetryEnabled = false;

        void setupTelemetry();
        void unloadTelemetry();

        void addBreadcrumb(
            const std::string& type,
            const std::string& msg,
            const std::string& category,
            const std::string& level
        );
        void sendMsgEvent(
            const sentry_level_e level,
            const std::string& moduleName,
            const std::string& message
        );

        void setTelemetryEnableStatus(bool enableStatus);

        void startSession();
        void endSession();

       private:
        TelemetryManager();
        ~TelemetryManager();

        TelemetryManager(const TelemetryManager&) = delete;
        TelemetryManager& operator=(const TelemetryManager&) = delete;

        bool isTelemetryActive = false;

        bool getTelemetryEnableStatus();

        std::string getOrCreateTelemetryId();

        std::function<void(sentry_options_t*)> delFunc;
        std::unique_ptr<sentry_options_t, decltype(delFunc)> sentryOptions;
    };
}  // namespace AikariShared::Infrastructure::Telemetry
