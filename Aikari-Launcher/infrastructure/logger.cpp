#include <Aikari-Launcher-Private/common.h>
#include <windows.h>

class AikariCustomFormatter : public spdlog::formatter
{
   public:
    AikariCustomFormatter() : sharedHead{"\033[0;36m<Aikari>\033[0m [%Y-%m-%d %H:%M:%S]"}, sharedEnd{"@ \033[0;33m/%s:%#/\033[0m %v"}
    {
        const std::string traceMiddle = "\033[0;30m\033[42m|TRACE|\033[0m";
        const std::string debugMiddle = "\033[0;30m\033[47m|DEBUG|\033[0m";
        const std::string infoMiddle = "\033[0;37m\033[46m|INFO|\033[0m";
        const std::string warningMiddle = "\033[0;30m\033[43m|WARN|\033[0m";
        const std::string errorMiddle = "\033[0;37m\033[41m|ERROR|\033[0m";
        const std::string criticalMiddle = "\033[0;37m\033[45m|CRITICAL|\033[0m";
        this->traceFormatter = std::make_unique<spdlog::pattern_formatter>(this->constructPattern(traceMiddle));
        this->debugFormatter = std::make_unique<spdlog::pattern_formatter>(this->constructPattern(debugMiddle));
        this->infoFormatter = std::make_unique<spdlog::pattern_formatter>(this->constructPattern(infoMiddle));
        this->warningFormatter = std::make_unique<spdlog::pattern_formatter>(this->constructPattern(warningMiddle));
        this->errorFormatter = std::make_unique<spdlog::pattern_formatter>(this->constructPattern(errorMiddle));
        this->criticalFormatter = std::make_unique<spdlog::pattern_formatter>(this->constructPattern(criticalMiddle));
    };

    void format(const spdlog::details::log_msg& msg, spdlog::memory_buf_t& destination) override
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

    std::unique_ptr<spdlog::formatter> clone() const override { return std::make_unique<AikariCustomFormatter>(); }

   private:
    const std::string sharedHead;
    const std::string sharedEnd;
    std::unique_ptr<spdlog::formatter> traceFormatter;
    std::unique_ptr<spdlog::formatter> debugFormatter;
    std::unique_ptr<spdlog::formatter> infoFormatter;
    std::unique_ptr<spdlog::formatter> warningFormatter;
    std::unique_ptr<spdlog::formatter> errorFormatter;
    std::unique_ptr<spdlog::formatter> criticalFormatter;

    std::string constructPattern(const std::string& middle)
    {
        return std::format("{} {} {}", this->sharedHead, middle, this->sharedEnd);
    }
};

namespace AikariLoggerSystem
{
int initLogger()
{
    SetConsoleOutputCP(CP_UTF8);  // Support emoji output 🥰, refer to:
                                  // https://stackoverflow.com/questions/71342226/c-not-printing-emojis-as-expected

    auto consoleSink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
    consoleSink->set_level(spdlog::level::trace);
    consoleSink->set_formatter(std::make_unique<AikariCustomFormatter>());

    std::vector loggerSinks{consoleSink};
    auto defaultLogger = std::make_shared<spdlog::logger>("defaultLogger", loggerSinks.begin(), loggerSinks.end());
    defaultLogger->set_level(spdlog::level::trace);
    spdlog::register_logger(defaultLogger);

    LOG_INFO("📃 Logger initialized!");
    return 0;
}

}  // namespace AikariLoggerSystem
