#pragma once

#include <Aikari-PLS-Private/types/components/mqtt.h>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <sol/table.hpp>
#include <string>
#include <unordered_map>
#include <vector>

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
        AikariPLS::Types::MQTTMsgQueue::PACKET_ENDPOINT_TYPE endpointType;
        std::optional<RuleSystem::RecogFeatureType> featureType;
        std::optional<std::string>
            featureName;  // thing.xxx.xxx for METHOD, paramName for PROPNAME,
                          // nullopt for ASCEND/DESCEND
        std::optional<std::string> ruleName;  // nullopt for REWRITE, string for
                                              // VIRTUAL, should be unique
        AikariPLS::Types::MQTTMsgQueue::PACKET_SIDE ruleSide;
        std::optional<std::string>
            enabledBy;  // settings key to make curRule enable, nullopt == N/A
                        // or always true
        std::optional<std::string> configKey;  // full settings obj key

        explicit RuleProps(const sol::table& prop);
    };

    // RuleMap be like:
    // clang-format off
    /*
    {
        "client2broker": {
            "rewrite": {
                "method": {
                    "GET": {
                        "FEATURE_NAME_HERE (e.g. `thing.service.postClientWebrtcCandidate`)":
                            [
                                {
                                    "rewriteFn": <ptr to function>,
                                    "isEnabled": bool,
                                    "enabledBy": std::optional<std::string>,
                                    "configKey": std::optional<std::string>,
                                    // in some cases maybe force config refresh is
                                    // required, that's why we keep enabledBy with each
                                    // feature
                                    "config": {
                                        <Nlohmann JSON>
                                    }
                                },
                            ]
                    },
                    "POST": {
                        "..."
                    },
                    "RPC": {
                        "..."
                    }
                },
                "prop": {
                    "PROP_NAME_HERE (e.g. `windowBlockStatus`)": [  // match thing.property.get
                        "..."
                    ]
                }
            },
            "virtualGen": { // eqeq ascend
                "RULE_NAME_HERE": {
                    "genFn": <ptr to function>
                }
            }
        },
        "broker2client": {
            "rewrite: {
                // ... same
                "prop": {  // match thing.property.set
                // ... same
                }
            },
            "virtualGen": { // eqeq descend
                // ... same
            }
        }
    }
     */
    // clang-format on

    typedef std::unique_ptr<std::function<std::string(std::string)>> fnPtr;
    namespace RuleMapping
    {
        namespace PerRuleProp
        {
            struct Rewrite
            {
                RuleSystem::fnPtr rewriteFn;
                bool isEnabled = false;
                std::optional<std::string> enabledBy;
                nlohmann::json config;
            };

            struct Virtual
            {
                RuleSystem::fnPtr genFn;
            };
        };  // namespace PerRuleProp

        typedef std::
            unordered_map<std::string, std::vector<PerRuleProp::Rewrite>>
                RewriteFeaturesMap;
        typedef std::unordered_map<
            Types::MQTTMsgQueue::PACKET_ENDPOINT_TYPE,
            RewriteFeaturesMap>
            RewriteMethodBasedMap;

        struct RewriteFeaturesStore
        {
            RewriteMethodBasedMap method;
            RewriteFeaturesMap prop;
        };

        typedef std::unordered_map<std::string, PerRuleProp::Virtual>
            VirtualGeneratorsMap;

        struct PerSideRulesStore
        {
            RewriteFeaturesStore rewrite;
            VirtualGeneratorsMap virtualGen;
        };

        struct FullRuleMapping
        {
            PerSideRulesStore client2broker;
            PerSideRulesStore broker2client;
        };
    }  // namespace RuleMapping
}  // namespace AikariPLS::Types::RuleSystem
