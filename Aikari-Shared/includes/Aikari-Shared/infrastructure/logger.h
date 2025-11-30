#pragma once

#include <string>
#include <unordered_set>

namespace AikariShared::LoggerSystem
{
    enum class LOGGER_SINK
    {
        CONSOLE,
        FILE,
    };

    inline std::unordered_set<LOGGER_SINK> loggerSinkSettings{
        LOGGER_SINK::CONSOLE
    };

    namespace Utility
    {
        static void cleanOldLogs(
            const std::filesystem::path& baseDir,
            std::string_view fileFormat,
            unsigned int preserveDays
        );
    }

    std::unordered_set<LOGGER_SINK>* getLoggerSinkSettingsPtr();

    int initLogger(
        const std::string& moduleName,
        int moduleTextColor,
        int moduleBgColor,
        const std::string& loggerName
    );
}  // namespace AikariShared::LoggerSystem
