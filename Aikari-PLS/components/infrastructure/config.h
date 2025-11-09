#pragma once

#include <Aikari-PLS-Private/types/components/config.h>
#include <Aikari-Shared/virtual/IConfigManager.h>
#include <filesystem>
#include <string>
#include <utility>

namespace AikariPLS::Components::Config
{
    class PLSConfigManager
        : public AikariShared::VirtualIns::IConfigManagerBase<
              PLSConfigManager,
              AikariPLS::Types::Config::PLSConfig>
    {
       public:
        PLSConfigManager(
            std::string module,
            std::filesystem::path configPath,
            int cfgResId,
            HINSTANCE hInstance
        )
            : IConfigManagerBase(
                  std::move(module), std::move(configPath), cfgResId, hInstance
              ) {};

        void loadConfigImpl(nlohmann::json& configData) override;
        nlohmann::json getStringifyConfigImpl() override;
    };
}  // namespace AikariPLS::Components::Config
