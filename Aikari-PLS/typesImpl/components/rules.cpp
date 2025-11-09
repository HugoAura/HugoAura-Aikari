#include <Aikari-PLS-Private/types/components/rules.h>
#include <Aikari-Shared/utils/string.h>

namespace AikariPLS::Types::RuleSystem
{
    RuleProps::RuleProps(const sol::table& prop)
    {
        this->ruleType = [&prop]() -> RuleSystem::RuleType
        {
            const std::string ruleTypeStr =
                prop.get_or<std::string>("ruleType", "UNKNOWN");
            if (ruleTypeStr == "ASCEND")
            {
                return RuleSystem::RuleType::ASCEND;
            }
            if (ruleTypeStr == "DESCEND")
            {
                return RuleSystem::RuleType::DESCEND;
            }
            if (ruleTypeStr == "REWRITE")
            {
                return RuleSystem::RuleType::REWRITE;
            }
            return RuleSystem::RuleType::UNKNOWN;
        }();
        this->endpointType =
            [&prop]() -> AikariPLS::Types::MQTTMsgQueue::PACKET_ENDPOINT_TYPE
        {
            const std::string endpointTypeStr =
                prop.get_or<std::string>("endpointType", "UNKNOWN");
            if (endpointTypeStr == "GET")
            {
                return MQTTMsgQueue::PACKET_ENDPOINT_TYPE::GET;
            }
            if (endpointTypeStr == "POST")
            {
                return MQTTMsgQueue::PACKET_ENDPOINT_TYPE::POST;
            }
            if (endpointTypeStr == "RPC")
            {
                return MQTTMsgQueue::PACKET_ENDPOINT_TYPE::RPC;
            }
            return MQTTMsgQueue::PACKET_ENDPOINT_TYPE::UNKNOWN;
        }();
        this->featureType =
            [&prop]() -> std::optional<RuleSystem::RecogFeatureType>
        {
            const std::string featureTypeStr =
                prop.get_or<std::string>("featureType", "UNKNOWN");
            if (featureTypeStr == "PROPNAME")
            {
                return RuleSystem::RecogFeatureType::PROPNAME;
            }
            if (featureTypeStr == "METHOD")
            {
                return RuleSystem::RecogFeatureType::METHOD;
            }
            return std::nullopt;
        }();
        this->featureName = prop.get_or<std::optional<std::string>>(
            "featureName", std::nullopt
        );
        this->ruleName =
            prop.get_or<std::optional<std::string>>("ruleName", std::nullopt);
        this->ruleSide =
            [&prop]() -> AikariPLS::Types::MQTTMsgQueue::PACKET_SIDE
        {
            const std::string ruleSideStr =
                prop.get_or<std::string>("ruleSide", "UNKNOWN");
            if (ruleSideStr == "REQ")
            {
                return MQTTMsgQueue::PACKET_SIDE::REQ;
            }
            if (ruleSideStr == "REP")
            {
                return MQTTMsgQueue::PACKET_SIDE::REP;
            }
            return MQTTMsgQueue::PACKET_SIDE::UNKNOWN;
        }();
        this->enabledBy =
            prop.get_or<std::optional<std::string>>("enabledBy", std::nullopt);
        this->configKey =
            prop.get_or<std::optional<std::string>>("configKey", std::nullopt);
    };

    namespace RuleMapping::PerRuleProp
    {
        void Rewrite::onConfigUpdate(nlohmann::json& newConfig)
        {
            this->config = newConfig;
            if (this->enabledBy.value_or("").empty())
            {
                // ↑ handle nullopt || enabledBy exactly === ""
                this->isEnabled = true;
            }
            else
            {
                auto enabledByLevels = AikariShared::Utils::String::split(
                    this->enabledBy.value(), '.'
                );
                nlohmann::json* curConfigAt = &newConfig;
                bool isInvalidEnabledBy = false;
                for (auto iterator = enabledByLevels.begin();
                     iterator != enabledByLevels.end() - 1;
                     ++iterator)
                {
                    if (curConfigAt->contains(*iterator))
                    {
                        curConfigAt = &(*curConfigAt)[*iterator];
                    }
                    else
                    {
                        isInvalidEnabledBy = true;
                        break;
                    }
                }
                this->isEnabled =
                    isInvalidEnabledBy
                        ? false
                        : curConfigAt->value(enabledByLevels.back(), false);
            }
        }
    }  // namespace RuleMapping::PerRuleProp
}  // namespace AikariPLS::Types::RuleSystem
