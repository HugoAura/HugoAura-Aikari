#include "config.h"

#include <Aikari-Launcher-Private/types/config/configType.h>

namespace AikariTypes::config
{
template <typename BasicJsonType>
void to_json(
    BasicJsonType& target, const AikariTypes::config::AikariConfig& origin
)
{
    target = nlohmann::json({ { "wsPreferPort", origin.wsPreferPort },
                              { "module", origin.module },
                              { "tls",
                                { { "regenWsCertNextLaunch",
                                    origin.tls.regenWsCertNextLaunch } } } });
};

template <typename BasicJsonType>
void from_json(
    const BasicJsonType& origin, AikariTypes::config::AikariConfig& target
)
{
    origin.at("wsPreferPort").get_to(target.wsPreferPort);
    origin.at("module").get_to(target.module);
    origin.at("tls")
        .at("regenWsCertNextLaunch")
        .get_to(target.tls.regenWsCertNextLaunch);
};
}  // namespace AikariTypes::config

namespace AikariLauncherComponents::AikariConfig
{
void LauncherConfigManager::loadConfigImpl(nlohmann::json& configData)
{
    auto configIns =
        configData.template get<AikariTypes::config::AikariConfig>();
    auto configInsPtr =
        std::make_shared<AikariTypes::config::AikariConfig>(configIns);
    this->config = configInsPtr;
};

nlohmann::json LauncherConfigManager::getStringifyConfigImpl()
{
    nlohmann::json stringifyConfig = *(this->config);
    return stringifyConfig;
};
};  // namespace AikariLauncherComponents::AikariConfig
