#pragma once

namespace AikariLauncher::Public::Constants::WebSocket::Config
{
    constexpr const char* _PREFIX = "config";
    namespace Actions
    {
        constexpr const char* _PREFIX = "actions";

        constexpr const char* GET_CONFIG = "config.actions.getConfig";
        /*
        GetConfig -> 获取 Aikari Launcher 完整 Config
        Data IN ←
        N/A
        Data REP →
        {
            "wsPreferPort": ...,
            ... // 省略
        }
        */

        constexpr const char* UPDATE_CONFIG = "config.actions.updateConfig";
        /*
        UpdateConfig -> 按 Key 更新 Aikari Launcher 的配置
        Data IN ←
        {
            "key": "foo.bar",
            "value": any,
            "write": std::optional<boolean> // default === true
        }
        Data REP →
        {
            "message": "..."
        }
        */

        constexpr const char* GET_TELEMETRY_STATUS =
            "config.actions.getTelemetryIsEnabled";
        /*
        Data IN ←
        N/A
        Data REP →
        {
            "isEnabled": bool
        }
        */

        constexpr const char* SET_TELEMETRY_STATUS =
            "config.actions.setTelemetryIsEnabled";
        /*
        Data IN ←
        {
            "isEnabled": bool
        }
        Data REP →
        {
            "message": "..."
        }
        */
    }  // namespace Actions
}  // namespace AikariLauncher::Public::Constants::WebSocket::Config
