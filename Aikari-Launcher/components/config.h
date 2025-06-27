#pragma once

#include <string>
#include <filesystem>
#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Launcher-Private/types/config/configType.h>
#include <Aikari-Shared/virtual/IConfigManager.h>
#include <Aikari-Shared/virtual/IConfigPayload.h>
#include <nlohmann/json.hpp>

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
        int defaultConfigResId
    )
        : IConfigManagerBase(module, configPath, defaultConfigResId) {};

    void loadConfigImpl(nlohmann::json& configData) override;
    nlohmann::json getStringifyConfigImpl() override;
};
}  // namespace AikariLauncherComponents::AikariConfig
