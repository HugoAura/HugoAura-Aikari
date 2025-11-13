#include "./rulesManager.h"

#define CUSTOM_LOG_HEADER "[Rule Manager]"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/utils/filesystem.h>
#include <Aikari-Shared/utils/string.h>
#include <filesystem>

namespace mqttMsgQueueTypes = AikariPLS::Types::MQTTMsgQueue;

namespace AikariPLS::Components::Rules
{
    Manager::Manager(
        const std::filesystem::path& ruleDir, nlohmann::json& config
    )
        : luaRuntime(std::make_unique<sol::state>()),
          ruleDir(ruleDir),
          config(config)
    {
        this->luaRuntime->open_libraries(
            sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::utf8
        );

        std::string originalPackagePaths(
            (*this->luaRuntime)["package"]["path"].get<std::string>()
        );
        CUSTOM_LOG_DEBUG("Default Lua package.path: {}", originalPackagePaths);

        std::string baseRuleDirForwardSlash = this->ruleDir.string();
        std::ranges::replace(baseRuleDirForwardSlash, '\\', '/');

        std::string customPackagePaths = std::format(
            "{}/../utils/?.lua;{}/../utils/init.lua;{}",
            baseRuleDirForwardSlash,
            baseRuleDirForwardSlash,
            originalPackagePaths
        );
        CUSTOM_LOG_DEBUG("New Lua package.path: {}", customPackagePaths);

        (*this->luaRuntime)["package"]["path"] = customPackagePaths;
    }

    Manager::~Manager() = default;

    bool Manager::loadRules()
    {
        if (this->isRuleLoaded)
        {
            CUSTOM_LOG_WARN(
                "Rules has been already loaded, cannot call loadRules() again."
            );
            return false;
        }

        std::vector<std::filesystem::path> ruleFiles;
        CUSTOM_LOG_DEBUG(
            "Searching folder {} for scripts...", this->ruleDir.string()
        );
        AikariShared::Utils::FileSystem::grepFilesWithExt(
            this->ruleDir, Rules::FILE_EXT, &ruleFiles
        );

        CUSTOM_LOG_INFO("Start loading scripts...");
        for (const auto& ruleFileName : ruleFiles)
        {
            try
            {
                std::string ruleContent =
                    AikariShared::Utils::FileSystem::readFile(ruleFileName);

                CUSTOM_LOG_DEBUG(
                    "Trying to run script {}...", ruleFileName.string()
                );
                this->luaRuntime->script(ruleContent);
                auto ruleProp =
                    (*this->luaRuntime)["ruleProp"]
                        .get_or<std::optional<sol::table>>(std::nullopt);
                auto onRecvFn =
                    (*this->luaRuntime)["onRecv"]
                        .get_or<std::optional<sol::protected_function>>(
                            std::nullopt
                        );

                if (ruleProp != std::nullopt && onRecvFn != std::nullopt)
                {
                    AikariPLS::Types::RuleSystem::RuleProps convertedRuleProps(
                        ruleProp.value()
                    );

                    this->insertRule(
                        convertedRuleProps, std::move(onRecvFn.value())
                    );
                }
                else
                {
                    CUSTOM_LOG_WARN(
                        "Invalid rule file {}, skipping...",
                        ruleFileName.string()
                    );
                }
            }
            catch (const std::exception& err)
            {
                CUSTOM_LOG_ERROR(
                    "Failed reading or executing file: {} | Error: {}",
                    ruleFileName.string(),
                    err.what()
                );
                continue;
            }
        }
        return true;
    }

    void Manager::insertRule(
        const AikariPLS::Types::RuleSystem::RuleProps& ruleProps,
        sol::protected_function onRecvFn
    )
    {
        // ↓ Decide side (client2broker / broker2client)
        AikariPLS::Types::RuleSystem::RuleMapping::PerSideRulesStore*
            perSideRulesStore;
        switch (ruleProps.ruleSide)
        {
            case mqttMsgQueueTypes::PACKET_SIDE::REQ:
            {
                perSideRulesStore = &this->ruleMapping.client2broker;
                break;
            }
            case mqttMsgQueueTypes::PACKET_SIDE::REP:
            {
                perSideRulesStore = &this->ruleMapping.broker2client;
                break;
            }
            default:
            {
                return;
            }
        }
        // ↑ Decide side
        // ↓ Dive into
        switch (ruleProps.ruleType)
        {
            case AikariPLS::Types::RuleSystem::RuleType::REWRITE:
            {
                AikariPLS::Types::RuleSystem::RuleMapping::RewriteFeaturesMap*
                    rewriteFeaturesMap = nullptr;

                switch (ruleProps.featureType.value())
                {
                    case AikariPLS::Types::RuleSystem::RecogFeatureType::METHOD:
                    {
                        switch (ruleProps.endpointType)
                        {
                            case mqttMsgQueueTypes::PACKET_ENDPOINT_TYPE::GET:
                            {
                                rewriteFeaturesMap =
                                    &(perSideRulesStore->rewrite
                                          .method[mqttMsgQueueTypes::
                                                      PACKET_ENDPOINT_TYPE::GET]
                                    );
                                break;
                            }
                            case mqttMsgQueueTypes::PACKET_ENDPOINT_TYPE::POST:
                            {
                                rewriteFeaturesMap = &(
                                    perSideRulesStore->rewrite
                                        .method[mqttMsgQueueTypes::
                                                    PACKET_ENDPOINT_TYPE::POST]
                                );
                                break;
                            }
                            case mqttMsgQueueTypes::PACKET_ENDPOINT_TYPE::RPC:
                            {
                                rewriteFeaturesMap =
                                    &(perSideRulesStore->rewrite
                                          .method[mqttMsgQueueTypes::
                                                      PACKET_ENDPOINT_TYPE::RPC]
                                    );
                                break;
                            }
                            default:
                            {
                                return;
                            }
                        }
                        break;
                    }
                    case AikariPLS::Types::RuleSystem::RecogFeatureType::
                        PROPNAME:
                    {
                        rewriteFeaturesMap = &(perSideRulesStore->rewrite.prop);
                        break;
                    }
                }

                if (rewriteFeaturesMap == nullptr)
                    return;

                auto configLevels = AikariShared::Utils::String::split(
                    ruleProps.configKey.value(), '.'
                );
                auto thisConfig =
                    Manager::getConfigObjByKeys(configLevels, this->config);

                AikariPLS::Types::RuleSystem::RuleMapping::PerRuleProp::Rewrite
                    rulePropInMapping = { .rewriteFn = std::move(onRecvFn),
                                          .enabledBy = ruleProps.enabledBy,
                                          .configKey = ruleProps.configKey };
                rulePropInMapping.onConfigUpdate(thisConfig);

                (*rewriteFeaturesMap)[ruleProps.featureName.value()]
                    .emplace_back(rulePropInMapping);
                break;
            }
            case AikariPLS::Types::RuleSystem::RuleType::ASCEND:
            case AikariPLS::Types::RuleSystem::RuleType::DESCEND:
            default:
            {
                perSideRulesStore->virtualGen[ruleProps.ruleName.value()] = {
                    .genFn = std::move(onRecvFn)
                };
                break;
            }
        }
            // ↑ Dive into
#ifdef _DEBUG
        CUSTOM_LOG_TRACE(
            "Successfully registered rule {}.",
            ruleProps.ruleName.has_value()
                ? std::format("VIRTUAL://{}", ruleProps.ruleName.value())
                : std::format(
                      "{}://{}",
                      AikariPLS::Types::MQTTMsgQueue::to_string(
                          ruleProps.endpointType
                      ),
                      ruleProps.featureName.value_or("???")
                  )
        );
#endif
    };

    nlohmann::json Manager::getConfigObjByKeys(
        const std::vector<std::string>& keys, nlohmann::json& rootConfig
    )
    {
        nlohmann::json* curConfig = &rootConfig;
        for (const auto& key : keys)
        {
            curConfig = &((*curConfig)[key]);
        }
        return *curConfig;
    };
}  // namespace AikariPLS::Components::Rules