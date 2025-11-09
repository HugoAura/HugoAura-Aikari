#include "config.h"

namespace AikariPLS::Types::Config
{
    template <typename BasicJsonType>
    void to_json(
        BasicJsonType& target, const AikariPLS::Types::Config::PLSConfig& origin
    )
    {
        target = nlohmann::json(
            { { "rules", origin.rules }, { "module", origin.module } }
        );
    };

    template <typename BasicJsonType>
    void from_json(
        const BasicJsonType& origin, AikariPLS::Types::Config::PLSConfig& target
    )
    {
        origin.at("rules").get_to(target.rules);
        origin.at("module").get_to(target.module);
    };
}  // namespace AikariPLS::Types::Config

namespace AikariPLS::Components::Config
{
    void PLSConfigManager::loadConfigImpl(nlohmann::json& configData)
    {
        auto configIns =
            configData.template get<AikariPLS::Types::Config::PLSConfig>();
        auto configInsPtr =
            std::make_shared<AikariPLS::Types::Config::PLSConfig>(configIns);
        this->config = configInsPtr;
    }

    nlohmann::json PLSConfigManager::getStringifyConfigImpl()
    {
        nlohmann::json stringifyConfig = *(this->config);
        return stringifyConfig;
    }
}  // namespace AikariPLS::Components::Config
