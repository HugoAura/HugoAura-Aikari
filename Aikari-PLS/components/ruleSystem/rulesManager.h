#pragma once

#include <Aikari-PLS-Private/types/components/mqtt.h>
#include <Aikari-PLS-Private/types/components/rules.h>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace AikariPLS::Components::Rules
{
    inline std::string FILE_EXT = "js";

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

       private:
        const std::filesystem::path& ruleDir;
        nlohmann::json& config;

        bool isRuleLoaded = false;
    };
}  // namespace AikariPLS::Components::Rules
