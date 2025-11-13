#pragma once

#include <filesystem>
#include <minwindef.h>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>

namespace AikariShared::VirtualIns
{
    struct LoadDefaultConfigRet
    {
        bool success;
        std::string result;
        std::optional<std::string> errorDetail;
    };

    // This one is not for production usage, see IConfigManagerBase below
    // instead.
    class IConfigManager
    {
       public:
        std::filesystem::path configPath;
        int defaultConfigResId;
        std::string module;
        std::mutex configEditLock;
        std::mutex configWriteLock;

        // clang-format off
        virtual void loadConfig(nlohmann::json& configData) = 0; // × Placeholder for IConfigManagerBase
        virtual void loadConfigImpl(nlohmann::json& configData) = 0; // [!] Actual impl
        virtual nlohmann::json getStringifyConfig() = 0; // × Placeholder for IConfigManagerBase
        virtual nlohmann::json getStringifyConfigImpl() = 0; // [!] Actual impl
        // clang-format on
        virtual bool writeConfigRaw(nlohmann::json& stringifyConfig);
        virtual bool writeConfig() = 0;
        virtual LoadDefaultConfigRet loadDefaultConfig();
        virtual bool initConfig();
        virtual ~IConfigManager() = default;

       protected:
        virtual void deepMergeConfig(
            const nlohmann::json& defaultConfig, nlohmann::json& userConfig
        ) const;

        HINSTANCE hInstance_;
    };

    template <typename DerivedRoot, typename ConfigType>
    class IConfigManagerBase : public IConfigManager
    {
       public:
        std::atomic<std::shared_ptr<ConfigType>> config;

        IConfigManagerBase(
            std::string module,
            std::filesystem::path configPath,
            int defaultConfigResId,
            HINSTANCE hInstance
        )
        {
            this->module = module;
            this->configPath = configPath;
            this->defaultConfigResId = defaultConfigResId;
            this->hInstance_ = hInstance;
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
            auto stringifyConfig = this->getStringifyConfig();
            return this->writeConfigRaw(stringifyConfig);
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
};  // namespace AikariShared::VirtualIns
