#pragma once

#include <Aikari-Launcher-Private/types/config/configType.h>
#include <Aikari-Shared/virtual/IConfigManager.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>

namespace AikariTypes::Config
{
    template <typename BasicJsonType>
    void to_json(
        BasicJsonType& target, const AikariTypes::Config::AikariConfig& origin
    );

    template <typename BasicJsonType>
    void from_json(
        const BasicJsonType& origin, AikariTypes::Config::AikariConfig& target
    );
}  // namespace AikariTypes::Config

namespace AikariLauncher::Components::AikariConfig
{
    class LauncherConfigManager
        : public AikariShared::VirtualIns::IConfigManagerBase<
              LauncherConfigManager,
              AikariTypes::Config::AikariConfig>
    {
       public:
        LauncherConfigManager(
            std::string module,
            std::filesystem::path configPath,
            int defaultConfigResId,
            HINSTANCE hInstance
        )
            : IConfigManagerBase(
                  std::move(module),
                  std::move(configPath),
                  defaultConfigResId,
                  hInstance
              ) {};

        void loadConfigImpl(nlohmann::json& configData) override;
        nlohmann::json getStringifyConfigImpl() override;
    };
}  // namespace AikariLauncher::Components::AikariConfig
