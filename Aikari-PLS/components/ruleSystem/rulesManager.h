#pragma once

#include <Aikari-PLS-Private/types/components/mqtt.h>
#include <Aikari-PLS-Private/types/components/rules.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <sol/sol.hpp>

namespace AikariPLS::Components::Rules
{
    inline std::string FILE_EXT = ".lua";

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

        AikariPLS::Types::MQTTMsgQueue::PayloadWithInfo processPacket(
            const AikariPLS::Types::MQTTMsgQueue::PayloadWithInfo& packetSrc
        );

        AikariPLS::Types::MQTTMsgQueue::PayloadWithInfo generateVirtualPayload(
            const AikariPLS::Types::MQTTMsgQueue::PACKET_SIDE& side,
            const std::string& ruleName,
            const std::optional<nlohmann::json>& data
        );*/

        AikariPLS::Types::RuleSystem::RuleMapping::FullRuleMapping ruleMapping;

        std::unique_ptr<sol::state> luaRuntime;

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
