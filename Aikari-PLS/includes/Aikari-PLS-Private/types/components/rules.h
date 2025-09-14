#pragma once

#include <Aikari-PLS-Private/types/components/mqtt.h>
#include <nlohmann/json.hpp>

namespace AikariPLS::Types::RuleSystem
{
    enum class RuleType
    {
        REWRITE,  // rewrite existing
        ASCEND,   // fake ↑
        DESCEND,  // fake ↓
        UNKNOWN,
    };

    enum class RecogFeatureType
    {
        METHOD,    // packet["method"]
        PROPNAME,  // packet["method"] === "thing.property.set"
                   // && paramRules[packet["params"][thisIdx]] exists
        /*
         * if pkt == (property set pkt), iter all packet["params"].keys(), check
         * if paramRules.includes(perKey), and call rewriteFn for every perKey
         */
    };

    struct RuleProps
    {
        RuleSystem::RuleType ruleType;
        AikariPLS::Types::mqttMsgQueue::PACKET_ENDPOINT_TYPE endpointType;
        std::optional<RuleSystem::RecogFeatureType> featureType;
        std::optional<std::string>
            featureName;  // thing.xxx.xxx for METHOD, paramName for PROPNAME,
                          // nullopt for ASCEND/DESCEND
        std::optional<std::string> ruleName;  // nullopt for REWRITE, string for
                                              // VIRTUAL, should be unique
        AikariPLS::Types::mqttMsgQueue::PACKET_SIDE ruleSide;
        std::optional<std::string>
            enabledBy;  // settings key to make curRule enable, nullopt == N/A
                        // or always true

        explicit RuleProps(const nlohmann::json& prop);
    };

    // RuleMap be like:
    // clang-format off
    /*
    {
        "client2broker": {
            "rewrite": {
                "method": {
                    "GET": {
                        "FEATURE_NAME_HERE (e.g. `thing.service.postClientWebrtcCandidate`)": {
                            "rewriteFn": <ptr to function>,
                            "isEnabled": bool,
                            "enabledBy": std::optional<std::string>
                            // in some cases maybe force config refresh is
                            // required, that's why we keep enabledBy with each
                            // feature
                        },
                    },
                    "POST": {
                        "..."
                    },
                    "RPC": {
                        "..."
                    }
                },
                "prop": {
                    "PROP_NAME_HERE (e.g. `windowBlockStatus`)": {
                        "..."
                    }
                }
            },
            "virtual": { // eqeq ascend
                "RULE_NAME_HERE": {
                    "genFn": <ptr to function>
                }
            }
        },
        "broker2client": {
            "rewrite: {
                // ... same
            },
            "virtual": { // eqeq descend
                // ... same
            }
        }
    }
     */
    // clang-format on
}  // namespace AikariPLS::Types::RuleSystem
