#pragma once

#include <filesystem>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>

#include "IConfigPayload.h"

namespace AikariLauncherPublic::virtualIns
{
struct LoadDefaultConfigRet
{
    bool success;
    std::string result;
    std::optional<std::string> errorDetail;
};

class IConfigManager
{
   public:
    std::filesystem::path configPath;
    int defaultConfigResId;
    std::string module;
    std::mutex configEditLock;
    std::mutex configWriteLock;

    virtual void loadConfig(nlohmann::json& configData) = 0;
    virtual void loadConfigImpl(nlohmann::json& configData) = 0;
    virtual nlohmann::json getStringifyConfig() = 0;
    virtual nlohmann::json getStringifyConfigImpl() = 0;
    virtual bool writeConfigRaw(nlohmann::json stringifyConfig);
    virtual bool writeConfig() = 0;
    virtual LoadDefaultConfigRet loadDefaultConfig();
    virtual bool initConfig();
    virtual ~IConfigManager() = default;

   protected:
    virtual void deepMergeConfig(
        const nlohmann::json& defaultConfig, nlohmann::json& userConfig
    ) const;
};

template <typename DerivedRoot, typename ConfigType>
class IConfigManagerBase : public IConfigManager
{
   public:
    std::shared_ptr<ConfigType> config;

    IConfigManagerBase(
        std::string module,
        std::filesystem::path configPath,
        int defaultConfigResId
    )
    {
        this->module = module;
        this->configPath = configPath;
        this->defaultConfigResId = defaultConfigResId;
    };

    void loadConfig(nlohmann::json& configData) override final
    {
        downcast()->loadConfigImpl(configData);
    };

    nlohmann::json getStringifyConfig() override final
    {
        return downcast()->getStringifyConfigImpl();
    };

    bool writeConfig() override
    {
        return this->writeConfigRaw(this->getStringifyConfig());
    };

   protected:
    DerivedRoot* downcast()
    {
        return static_cast<DerivedRoot*>(this);
    }

    const DerivedRoot* downcast() const
    {
        return static_cast<const DerivedRoot*>(this);
    }
};
};  // namespace AikariLauncherPublic::virtualIns
