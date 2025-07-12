#pragma once

#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Launcher-Private/types/config/configType.h>
#include <Aikari-Shared/virtual/IConfigManager.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>

namespace AikariLauncherComponents::AikariConfig
{

    class LauncherConfigManager
        : public AikariShared::virtualIns::IConfigManagerBase<
              LauncherConfigManager,
              AikariTypes::config::AikariConfig>
    {
       public:
        LauncherConfigManager(
            std::string module,
            std::filesystem::path configPath,
            int defaultConfigResId,
            HINSTANCE hInstance
        )
            : IConfigManagerBase(
                  module, configPath, defaultConfigResId, hInstance
              ) {};

        void loadConfigImpl(nlohmann::json& configData) override;
        nlohmann::json getStringifyConfigImpl() override;
    };
}  // namespace AikariLauncherComponents::AikariConfig
