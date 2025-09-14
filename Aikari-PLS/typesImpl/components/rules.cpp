#include <Aikari-PLS-Private/types/components/rules.h>

namespace AikariPLS::Types::RuleSystem
{
    RuleProps::RuleProps(const nlohmann::json& prop)
    {
        this->ruleType = [&prop]() -> RuleSystem::RuleType
        {
            auto ruleTypeStr = prop.value("ruleType", "UNKNOWN");
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
            auto endpointTypeStr = prop.value("endpointType", "UNKNOWN");
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
            auto featureTypeStr = prop.value("featureType", "UNKNOWN");
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
        this->featureName =
            prop.value<std::optional<std::string>>("featureName", std::nullopt);
        this->ruleName =
            prop.value<std::optional<std::string>>("ruleName", std::nullopt);
        this->ruleSide =
            [&prop]() -> AikariPLS::Types::MQTTMsgQueue::PACKET_SIDE
        {
            auto ruleSideStr = prop.value("ruleSide", "UNKNOWN");
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
            prop.value<std::optional<std::string>>("enabledBy", std::nullopt);
    };
}  // namespace AikariPLS::Types::RuleSystem
