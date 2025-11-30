#include <Aikari-Shared/infrastructure/logger.h>
// clang-format off
#include <windows.h>
#include <filesystem>
#include <string>
#include <ShlObj.h>
#include <spdlog/spdlog.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
// clang-format on

#ifdef _DEBUG
#include <iostream>
#endif

class AikariCustomFormatter : public spdlog::formatter
{
   public:
    AikariCustomFormatter(const std::string& moduleSection)
        : sharedHead{ "\033[0;36m<Aikari>\033[0m [%Y-%m-%d %H:%M:%S]" },
          sharedEnd{ "@ \033[0;33m/%s:%#/\033[0m %v" },
          moduleSection(moduleSection)
    {
        const std::string traceMiddle = "\033[0;30m\033[42m|TRACE|\033[0m";
        const std::string debugMiddle = "\033[0;30m\033[47m|DEBUG|\033[0m";
        const std::string infoMiddle = "\033[0;37m\033[46m|INFO|\033[0m";
        const std::string warningMiddle = "\033[0;30m\033[43m|WARN|\033[0m";
        const std::string errorMiddle = "\033[0;37m\033[41m|ERROR|\033[0m";
        const std::string criticalMiddle =
            "\033[0;37m\033[45m|CRITICAL|\033[0m";
        this->traceFormatter = std::make_unique<spdlog::pattern_formatter>(
            this->constructPattern(traceMiddle)
        );
        this->debugFormatter = std::make_unique<spdlog::pattern_formatter>(
            this->constructPattern(debugMiddle)
        );
        this->infoFormatter = std::make_unique<spdlog::pattern_formatter>(
            this->constructPattern(infoMiddle)
        );
        this->warningFormatter = std::make_unique<spdlog::pattern_formatter>(
            this->constructPattern(warningMiddle)
        );
        this->errorFormatter = std::make_unique<spdlog::pattern_formatter>(
            this->constructPattern(errorMiddle)
        );
        this->criticalFormatter = std::make_unique<spdlog::pattern_formatter>(
            this->constructPattern(criticalMiddle)
        );
    };

    void format(
        const spdlog::details::log_msg& msg, spdlog::memory_buf_t& destination
    ) override
    {
        switch (msg.level)
        {
            case spdlog::level::trace:
                this->traceFormatter->format(msg, destination);
                break;
            case spdlog::level::debug:
                this->debugFormatter->format(msg, destination);
                break;
            case spdlog::level::info:
                this->infoFormatter->format(msg, destination);
                break;
            case spdlog::level::warn:
                this->warningFormatter->format(msg, destination);
                break;
            case spdlog::level::err:
                this->errorFormatter->format(msg, destination);
                break;
            case spdlog::level::critical:
                this->criticalFormatter->format(msg, destination);
                break;
            default:
                break;  // silently ignore
        }
    }

    std::unique_ptr<spdlog::formatter> clone() const override
    {
        return std::make_unique<AikariCustomFormatter>(this->moduleSection);
    }

   private:
    const std::string sharedHead;
    const std::string sharedEnd;
    const std::string moduleSection;
    std::unique_ptr<spdlog::formatter> traceFormatter;
    std::unique_ptr<spdlog::formatter> debugFormatter;
    std::unique_ptr<spdlog::formatter> infoFormatter;
    std::unique_ptr<spdlog::formatter> warningFormatter;
    std::unique_ptr<spdlog::formatter> errorFormatter;
    std::unique_ptr<spdlog::formatter> criticalFormatter;

    std::string constructPattern(const std::string& middle)
    {
        return std::format(
            "{} {} {} {}",
            this->sharedHead,
            this->moduleSection,
            middle,
            this->sharedEnd
        );
    }
};

namespace AikariShared::LoggerSystem
{
    namespace Utility
    {
        static std::filesystem::path getOrCreateLogFileBaseDir()
        {
            PWSTR pathPtr = NULL;
            HRESULT hResult =
                SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &pathPtr);
            std::filesystem::path programDataDir;
            if (SUCCEEDED(hResult))
            {
                programDataDir = std::filesystem::path(pathPtr);
            }
            else
            {
                programDataDir = std::filesystem::path("C:") / "ProgramData";
            }
            CoTaskMemFree(pathPtr);
            std::filesystem::path logDir =
                programDataDir / "HugoAura" / "Aikari" / "log";
            if (!std::filesystem::exists(logDir))
                std::filesystem::create_directories(logDir);
            return logDir;
        };

        static void cleanOldLogs(
            const std::filesystem::path& baseDir,
            std::string_view fileFormat,
            unsigned int preserveDays
        )
        {
            if (!std::filesystem::exists(baseDir) ||
                !std::filesystem::is_directory(baseDir))
                return;
            const auto dateFmtStartPos = fileFormat.find('<');
            const auto dateFmtEndPos = fileFormat.find('>');
            if (dateFmtStartPos == std::string::npos ||
                dateFmtEndPos == std::string::npos)
                return;
            const std::string_view filePrefix =
                fileFormat.substr(0, dateFmtStartPos);
            const std::string_view fileSuffix =
                fileFormat.substr(dateFmtEndPos + 1);

            std::string dateFmtStr = std::string(fileFormat.substr(
                dateFmtStartPos + 1, dateFmtEndPos - dateFmtStartPos - 1
            ));

            const std::chrono::zoned_time ztLocal(
                std::chrono::current_zone(), std::chrono::system_clock::now()
            );
            const auto curDay =
                std::chrono::floor<std::chrono::days>(ztLocal.get_local_time());
            const auto cutoffDayLocal =
                curDay - std::chrono::days(preserveDays);
            const std::chrono::zoned_time ztCutoff(
                ztLocal.get_time_zone(), cutoffDayLocal
            );
            const std::chrono::sys_days cutoffDay =
                std::chrono::floor<std::chrono::days>(ztCutoff.get_sys_time());

            for (const auto& fileEntity :
                 std::filesystem::recursive_directory_iterator(baseDir))
            {
                if (!fileEntity.is_regular_file())
                    continue;
                const auto& path = fileEntity.path();
                const std::string fileName = path.filename().string();
                if (!fileName.starts_with(filePrefix) ||
                    !fileName.ends_with(fileSuffix))
                    continue;

                const std::string_view fileNameDate =
                    std::string_view(fileName).substr(
                        filePrefix.length(),
                        fileName.length() - filePrefix.length() -
                            fileSuffix.length()
                    );

                std::chrono::sys_days fileDate;
                std::stringstream fileNameDateSS{ std::string(fileNameDate) };
                fileNameDateSS >> std::chrono::parse(dateFmtStr, fileDate);

                if (fileNameDateSS.fail())
                    continue;

                if (fileDate < cutoffDay)
                {
                    std::error_code ec;
                    std::filesystem::remove(path, ec);
                    if (ec)
                    {
#ifdef _DEBUG
                        std::cerr << "[Log Cleaner] Error removing file: "
                                  << ec.message() << std::endl;
#endif
                    }
                }
            }
        };

        static std::filesystem::path getTodayLogFileName(
            const std::string& moduleName, const std::filesystem::path& baseDir
        )
        {
            std::chrono::zoned_time ztLocal(
                std::chrono::current_zone(), std::chrono::system_clock::now()
            );

            std::string fileNameTimePart = std::format("{:%Y-%m-%d}", ztLocal);
            std::filesystem::path finalLogFileName =
                baseDir /
                std::format("Aikari_{}_{}.log", moduleName, fileNameTimePart);

            return finalLogFileName.string();
        };
    }  // namespace Utility

    std::unordered_set<LOGGER_SINK>* getLoggerSinkSettingsPtr()
    {
        return &AikariShared::LoggerSystem::loggerSinkSettings;
    };

    int initLogger(
        const std::string& moduleName,
        int moduleTextColor,
        int moduleBgColor,
        const std::string& loggerName
    )
    {
        std::vector<std::shared_ptr<spdlog::sinks::sink>> loggerSinks;
        if (AikariShared::LoggerSystem::loggerSinkSettings.contains(
                AikariShared::LoggerSystem::LOGGER_SINK::CONSOLE
            ))
        {
            SetConsoleOutputCP(CP_UTF8);  // Support emoji output 🥰, refer to:
            // https://stackoverflow.com/questions/71342226/c-not-printing-emojis-as-expected

            std::string moduleSectionContent = std::format(
                "\033[0;{}m\033[{}m|{}|\033[0m",
                moduleTextColor,
                moduleBgColor,
                moduleName
            );

            auto consoleSink =
                std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
            consoleSink->set_level(spdlog::level::trace);
            consoleSink->set_formatter(
                std::make_unique<AikariCustomFormatter>(moduleSectionContent)
            );
            loggerSinks.emplace_back(consoleSink);
        }

        if (AikariShared::LoggerSystem::loggerSinkSettings.contains(
                AikariShared::LoggerSystem::LOGGER_SINK::FILE
            ))
        {
            const std::filesystem::path targetLogDir =
                Utility::getOrCreateLogFileBaseDir();
            Utility::cleanOldLogs(
                targetLogDir, "Aikari_" + moduleName + "_<%Y-%m-%d>.log", 7
            );
            auto targetLogPath =
                Utility::getTodayLogFileName(moduleName, targetLogDir);
            auto dailyFileLoggerSink =
                std::make_shared<spdlog::sinks::daily_file_format_sink_mt>(
                    targetLogPath.string(), 0, 0, false, 10
                );
            dailyFileLoggerSink->set_level(spdlog::level::info);
            dailyFileLoggerSink->set_pattern(
                "[%Y-%m-%d %H:%M:%S] |" + moduleName + "| [%l] /%s:%#/ %v"
            );
            loggerSinks.emplace_back(dailyFileLoggerSink);
        }

        auto thisLogger = std::make_shared<spdlog::logger>(
            loggerName, loggerSinks.begin(), loggerSinks.end()
        );
        thisLogger->set_level(spdlog::level::trace);
        spdlog::register_logger(thisLogger);

        thisLogger->info("📃 Logger for module {} initialized!", moduleName);
        return 0;
    }

}  // namespace AikariShared::LoggerSystem
