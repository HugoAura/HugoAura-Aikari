#include "pch.h"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/virtual/IConfigManager.h>
#include <fstream>
#include <windows.h>

namespace AikariShared::virtualIns
{
// --- Begin implementations of IConfigManager --- //

// ↓ protected
void IConfigManager::deepMergeConfig(
    const nlohmann::json& defaultConfig, nlohmann::json& userConfig
) const
{
    for (auto& [key, val] : defaultConfig.items())
    {
        if (userConfig.contains(key))
        {
            if (userConfig[key].is_object() && val.is_object())
            {
                this->deepMergeConfig(val, userConfig[key]);
            }
        }
        else
        {
            userConfig[key] = val;
        }
    };
};

// ↓ virtual public
LoadDefaultConfigRet IConfigManager::loadDefaultConfig()
{
    int& resId = this->defaultConfigResId;
    LoadDefaultConfigRet ret;
    ret.success = false;
    ret.result = "";

    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resId), RT_RCDATA);
    if (hRes == NULL)
    {
        ret.errorDetail = std::make_optional<std::string>(
            "Failed to find res. ResId: " + std::to_string(resId)
        );
        return ret;
    }

    HGLOBAL hResLoad = LoadResource(NULL, hRes);
    if (hResLoad == NULL)
    {
        ret.errorDetail =
            std::make_optional<std::string>("Failed to load res.");
        return ret;
    }

    void* resPtr = LockResource(hResLoad);
    if (resPtr == NULL)
    {
        ret.errorDetail = std::make_optional<std::string>(
            "Unexpected error while locking res."
        );
        return ret;
    }

    DWORD resSize = SizeofResource(NULL, hRes);

    ret.result = std::string(static_cast<char*>(resPtr), resSize);
    ret.success = true;
    return ret;
};

// ↓ virtual public
bool IConfigManager::writeConfigRaw(nlohmann::json stringifyConfig)
{
    this->configWriteLock.lock();
    const std::filesystem::path configDirPath = this->configPath.parent_path();
    bool isConfigDirExists = std::filesystem::exists(configDirPath);
    if (!isConfigDirExists)
    {
        LOG_INFO(
            "Config dir {} not exists, creating...", configDirPath.string()
        );
        std::filesystem::create_directories(configDirPath);
    }

    {
        std::ofstream configFile(this->configPath);
        if (!configFile.is_open())
        {
            LOG_ERROR(
                "Failed to write config: cannot open config file. Module: " +
                this->module
            );
            this->configWriteLock.unlock();
            return false;
        }

        std::string configStr = stringifyConfig.dump();
        const char* configCStr = configStr.c_str();
        configFile.write(configCStr, strlen(configCStr));
        configFile.close();
    }

#ifdef _DEBUG
    LOG_DEBUG("Successfully write config to disk. Module: " + this->module);
#endif

    this->configWriteLock.unlock();
    return true;
};

// ↓ public
bool IConfigManager::initConfig()
{
    bool isExists = std::filesystem::exists(this->configPath);

    LoadDefaultConfigRet defaultConfigRet = this->loadDefaultConfig();
    if (!defaultConfigRet.success)
    {
        LOG_ERROR(
            "Failed to load default config for module {}: {}",
            this->module,
            defaultConfigRet.errorDetail.value_or("Unknown Error")
        );
        return false;
    }

    nlohmann::json defaultConfigJson =
        nlohmann::json::parse(defaultConfigRet.result);

    if (isExists)
    {
        {
            std::ifstream configFileStream(this->configPath);
            if (!configFileStream.is_open())
            {
                LOG_ERROR(
                    "Failed to open user config file. Module: " + this->module
                );
                return false;
            }
            nlohmann::json userConfigJson =
                nlohmann::json::parse(configFileStream);

            LOG_INFO(
                "Merging user config with default preset... | Module: " +
                this->module
            );

            this->deepMergeConfig(defaultConfigJson, userConfigJson);

#ifdef _DEBUG
            LOG_DEBUG("Merged config: " + userConfigJson.dump());
#endif

            this->loadConfig(userConfigJson);
            configFileStream.close();
        }
        LOG_INFO("Loaded user config for module {}.", this->module);
        return true;
    }
    else
    {
        this->loadConfig(defaultConfigJson);
        bool writeResult = this->writeConfigRaw(defaultConfigJson);
        if (!writeResult)
        {
            LOG_ERROR(
                "Failed to initialize default config for module {}.",
                this->module
            );
            return false;
        }
        LOG_INFO("Loaded default config for module {}.", this->module);
        return true;
    }
};
// --- End implementations of IConfigManager --- //
};  // namespace AikariShared::virtualIns
