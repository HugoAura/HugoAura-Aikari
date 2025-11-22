#include <Aikari-Shared/infrastructure/telemetry.h>  // Self
#include <Aikari-Shared/types/constants/telemetry.h>
#include <Aikari-Shared/utils/filesystem.h>
#include <Aikari-Shared/utils/random.h>
#include <Aikari-Shared/utils/string.h>
#include <fstream>

#define TELEMETRY_MODULE_NAME "Shared - Telemetry"

namespace AikariShared::Infrastructure::Telemetry
{
    TelemetryManager& TelemetryManager::getInstance()
    {
        static TelemetryManager instance;
        return instance;
    };

    TelemetryManager::TelemetryManager()
        : sentryDsn(AikariShared::Constants::Telemetry::SENTRY_DSN),
          sentryRel(AikariShared::Constants::Telemetry::SENTRY_REL),
          sentryOptions(nullptr)
    {
        this->sentryDatabasePath =
            AikariShared::Utils::FileSystem::getAikariRootDir(false) /
            ".sentry-native";
        this->telemetryIdPath =
            sentryDatabasePath.parent_path() / ".telemetryId";
        this->telemetryEnableFlagPath =
            sentryDatabasePath.parent_path() / ".telemetryEnabled";
        this->delFunc = [](sentry_options_t* ptr)
        {
            sentry_options_free(ptr);
        };
        try
        {
            this->isTelemetryEnabled = this->getTelemetryEnableStatus();
        }
        catch (...)
        {
            this->isTelemetryEnabled = false;
        }
    };

    TelemetryManager::~TelemetryManager()
    {
        this->unloadTelemetry();
    };

    void TelemetryManager::setupTelemetry()
    {
        if (!isTelemetryEnabled)
            return;

        this->telemetryId = this->getOrCreateTelemetryId();

        decltype(this->sentryOptions) sentryOptionsPtr(
            sentry_options_new(), this->delFunc
        );
        this->sentryOptions = std::move(sentryOptionsPtr);
        auto* sentryOptionsRawPtr = this->sentryOptions.get();
        sentry_options_set_dsn(sentryOptionsRawPtr, this->sentryDsn.c_str());
        sentry_options_set_database_path(
            sentryOptionsRawPtr, this->sentryDatabasePath.string().c_str()
        );
        sentry_options_set_release(
            sentryOptionsRawPtr, this->sentryRel.c_str()
        );
#ifdef _DEBUG
        sentry_options_set_debug(sentryOptionsRawPtr, 1);
        sentry_options_set_environment(sentryOptionsRawPtr, "development");
#else
        sentry_options_set_environment(sentryOptionsRawPtr, "production");
#endif
        sentry_init(sentryOptionsRawPtr);
        this->isTelemetryActive = true;

        auto sentryUserInfo = sentry_value_new_object();
        sentry_value_set_by_key(
            sentryUserInfo,
            "id",
            sentry_value_new_string(this->telemetryId.c_str())
        );
        sentry_set_user(sentryUserInfo);
    };

    void TelemetryManager::unloadTelemetry()
    {
        this->isTelemetryActive = false;
        sentry_close();
        this->sentryOptions.reset();
        this->telemetryId = Telemetry::defaultTelemetryId;
    };

    void TelemetryManager::setTelemetryEnableStatus(bool enableStatus)
    {
        if (!std::filesystem::exists(
                this->telemetryEnableFlagPath.parent_path()
            ))
        {
            std::filesystem::create_directories(
                this->telemetryEnableFlagPath.parent_path()
            );
        }

        if (enableStatus == true)
        {
            std::ofstream outStream(this->telemetryEnableFlagPath);
            if (!outStream.is_open())
                return;
            outStream << "";
            outStream.close();
        }
        else
        {
            std::filesystem::remove(this->telemetryEnableFlagPath);
        }

        this->isTelemetryEnabled = enableStatus;

        if (!this->isTelemetryActive && enableStatus)
        {
            this->setupTelemetry();
        }
        else if (this->isTelemetryActive && !enableStatus)
        {
            this->unloadTelemetry();
        }
    };

    void TelemetryManager::startSession()
    {
        if (this->isTelemetryActive)
        {
            sentry_start_session();
        }
    };

    void TelemetryManager::endSession()
    {
        if (this->isTelemetryActive)
        {
            sentry_end_session();
        }
    }

    void TelemetryManager::addBreadcrumb(
        const std::string& type,
        const std::string& msg,
        const std::string& category,
        const std::string& level
    )
    {
        if (!this->isTelemetryActive || !this->isTelemetryEnabled)
            return;
        auto crumbData = sentry_value_new_breadcrumb(type.c_str(), msg.c_str());
        sentry_value_set_by_key(
            crumbData, "category", sentry_value_new_string(category.c_str())
        );
        sentry_value_set_by_key(
            crumbData, "level", sentry_value_new_string(level.c_str())
        );
        sentry_add_breadcrumb(crumbData);
    };

    void TelemetryManager::sendMsgEvent(
        const sentry_level_e level,
        const std::string& moduleName,
        const std::string& message
    )
    {
        if (!this->isTelemetryActive || !this->isTelemetryEnabled)
            return;
        auto msgEvent = sentry_value_new_message_event(
            level, moduleName.c_str(), message.c_str()
        );
        sentry_capture_event(msgEvent);
    };

    bool TelemetryManager::getTelemetryEnableStatus()
    {
        return std::filesystem::exists(this->telemetryEnableFlagPath);
    };

    std::string TelemetryManager::getOrCreateTelemetryId()
    {
        try
        {
            const bool isRegularFile =
                std::filesystem::is_regular_file(this->telemetryIdPath);
            const bool isExists =
                std::filesystem::exists(this->telemetryIdPath);
            if (!isExists || !isRegularFile)
            {
                if (!isRegularFile)
                {
                    std::filesystem::remove(this->telemetryIdPath);
                }
                std::filesystem::create_directories(
                    this->telemetryIdPath.parent_path()
                );
                std::ofstream outStream(this->telemetryIdPath);
                if (outStream.is_open())
                {
                    auto telemetryIdGenerated =
                        AikariShared::Utils::Random::genUUIDv4();
                    outStream << telemetryIdGenerated << std::endl;
                    outStream.close();
                    return telemetryIdGenerated;
                }
                else
                {
                    throw std::runtime_error(
                        "Failed to open telemetry ID path: " +
                        this->telemetryIdPath.string()
                    );
                }
            }
            else
            {
                std::ifstream ifStream(this->telemetryIdPath);
                std::stringstream fileBuffer;
                fileBuffer << ifStream.rdbuf();
                return AikariShared::Utils::String::trim(fileBuffer.str());
            }
        }
        catch (const std::exception& err)
        {
            this->sendMsgEvent(
                SENTRY_LEVEL_ERROR,
                TELEMETRY_MODULE_NAME,
                std::format("Failed to get telemetry ID: {}", err.what())
            );
            return Telemetry::defaultTelemetryId;
        }
    };
}  // namespace AikariShared::Infrastructure::Telemetry
