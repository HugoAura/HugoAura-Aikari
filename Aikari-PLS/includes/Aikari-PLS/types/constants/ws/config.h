#pragma once

namespace AikariPLS::Types::Constants::WebSocket::Config
{
    constexpr const char* _PREFIX = "config";

    namespace Rules
    {
        constexpr const char* _PREFIX = "rules";

        constexpr const char* GET_RULE_CONFIG = "config.rules.getConfig";
        /*
        GetRuleConfig -> 获取 PLS 完整规则
        Data IN ←
        N/A
        Data REP →
        nlohmann::json
        */

        constexpr const char* UPDATE_RULE_CONFIG = "config.rules.updateConfig";
        // clang-format off
        /*
        UpdateRuleConfig -> 更新 PLS 规则
        Data IN ←
        {
            std::string key; // foo.bar.enabled, relative to PLS_CONFIG_ROOT.rules
            std::??? value;
            std::string affiliated; // foo.bar, configKey for edited perRule
            std::optional<bool> write;
        }
        Data REP →
        {
            "message": "..."
        }
        */
        // clang-format on
    }  // namespace Rules

    namespace Common
    {
        // edit config items other than PLS_CONFIG_ROOT.rules
        // TODO: Impl pls config.common.XXX (or maybe this will never be
        // required to impl)
    }
}  // namespace AikariPLS::Types::Constants::WebSocket::Config
