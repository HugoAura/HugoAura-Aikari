#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/utils/windows.h>
#include <Aikari-Shared/virtual/IConfigManager.h>
#include <fstream>
#include <stack>

namespace AikariShared::VirtualIns
{
    // --- Begin implementations of IConfigManager --- //

    // ↓ protected
    void IConfigManager::deepMergeConfig(
        const nlohmann::json& defaultConfig, nlohmann::json& userConfig
    ) const
    {
        std::stack<std::pair<const nlohmann::json*, nlohmann::json*>> workStack;
        workStack.push({ &defaultConfig, &userConfig });
        while (!workStack.empty())
        {
            auto [defaultThis, userThis] = workStack.top();
            workStack.pop();
            for (auto& [key, val] : defaultThis->items())
            {
                if (userThis->contains(key))
                {
                    auto& userVal = (*userThis)[key];
                    if (userVal.is_object() && val.is_object())
                    {
                        workStack.push({ &val, &userVal });
                    }
                }
                else
                {
                    (*userThis)[key] = val;
                }
            };
        }
    };

    // ↓ virtual public
    LoadDefaultConfigRet IConfigManager::loadDefaultConfig()
    {
        int& resId = this->defaultConfigResId;
        LoadDefaultConfigRet ret;
        ret.success = false;
        ret.result = "";

        auto loadResRet =
            AikariShared::Utils::Windows::RC::loadStringResource<char>(
                this->hInstance_, resId
            );

        ret.success = loadResRet.success;
        if (loadResRet.success)
        {
            ret.result = std::string(loadResRet.retPtr, loadResRet.size);
        }
        else
        {
            ret.errorDetail =
                std::make_optional<std::string>(loadResRet.message);
        }

        return ret;
    };

    // ↓ virtual public
    bool IConfigManager::writeConfigRaw(nlohmann::json& stringifyConfig)
    {
        this->configWriteLock.lock();
        const std::filesystem::path configDirPath =
            this->configPath.parent_path();
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
                    "Failed to write config: cannot open config file. "
                    "Module: " +
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
                        "Failed to open user config file. Module: " +
                        this->module
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

                LOG_DEBUG("Writing merged config to disk...");
                this->writeConfigRaw(userConfigJson);

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
};  // namespace AikariShared::VirtualIns
