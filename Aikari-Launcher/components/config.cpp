#include "config.h"

#include <Aikari-Launcher-Private/types/config/configType.h>

namespace AikariTypes::Config
{
    template <typename BasicJsonType>
    void to_json(
        BasicJsonType& target, const AikariTypes::Config::AikariConfig& origin
    )
    {
        target = nlohmann::json(
            { { "wsPreferPort", origin.wsPreferPort },
              { "module", origin.module },
              { "tls",
                { { "regenWsCertNextLaunch",
                    origin.tls.regenWsCertNextLaunch } } } }
        );
    };

    template <typename BasicJsonType>
    void from_json(
        const BasicJsonType& origin, AikariTypes::Config::AikariConfig& target
    )
    {
        origin.at("wsPreferPort").get_to(target.wsPreferPort);
        origin.at("module").get_to(target.module);
        origin.at("tls")
            .at("regenWsCertNextLaunch")
            .get_to(target.tls.regenWsCertNextLaunch);
    };
}  // namespace AikariTypes::Config

namespace AikariLauncher::Components::AikariConfig
{
    void LauncherConfigManager::loadConfigImpl(nlohmann::json& configData)
    {
        auto configIns =
            configData.template get<AikariTypes::Config::AikariConfig>();
        auto configInsPtr =
            std::make_shared<AikariTypes::Config::AikariConfig>(configIns);
        // this->config = configInsPtr;
        std::atomic_store(&this->config, configInsPtr);
    };

    nlohmann::json LauncherConfigManager::getStringifyConfigImpl()
    {
        nlohmann::json stringifyConfig = *std::atomic_load(&this->config);
        return stringifyConfig;
    };
};  // namespace AikariLauncher::Components::AikariConfig
