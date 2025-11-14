#pragma once

#include <Aikari-PLS-Private/types/components/mqtt.h>
#include <Aikari-PLS-Private/types/components/rules.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <sol/sol.hpp>

namespace AikariPLS::Components::Rules
{
    inline std::string FILE_EXT = ".lua";

    typedef std::string ConfigKeyStr;

    class Manager
    {
       public:
        explicit Manager(
            const std::filesystem::path& ruleDir, nlohmann::json& config
        );

        ~Manager();

        bool loadRules();
        /*
        bool unloadRules();
        */

        // TODO: Impl manual rules unload

        std::unique_ptr<sol::state> luaRuntime;

        AikariPLS::Types::RuleSystem::RuleMapping::FullRuleMapping ruleMapping;
        // clang-format off
        std::unordered_map<
            ConfigKeyStr,
            std::vector<
                std::unique_ptr<
                    AikariPLS::Types::RuleSystem::RuleMapping::
                                    PerRuleProp::Rewrite
                                >*
                        >
        > configKeyAssociationMap; // stores rawPtrsForUniquePtrs
        // clang-format on

       private:
        const std::filesystem::path& ruleDir;
        nlohmann::json& config;

        void insertRule(
            const AikariPLS::Types::RuleSystem::RuleProps& ruleProps,
            sol::protected_function onRecvFn
        );

        static nlohmann::json getConfigObjByKeys(
            const std::vector<std::string>& keys, nlohmann::json& rootConfig
        );

        bool isRuleLoaded = false;
    };
}  // namespace AikariPLS::Components::Rules
