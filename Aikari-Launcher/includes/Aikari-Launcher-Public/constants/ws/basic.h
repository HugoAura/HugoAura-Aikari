#pragma once

namespace AikariLauncherPublic::Constants::WebSocket::Basic
{
    constexpr const char* _PREFIX = "basic";
    namespace Props
    {
        constexpr const char* _PREFIX = "props";

        constexpr const char* GET_VERSION = "basic.props.getVersion";
        /*
        GetVersion -> 获取 Aikari 版本
        Data IN ←
        N/A
        Data REP →
        {
            "version": std::string,
            "versionCode": int
        }
        */
    }  // namespace Props
}  // namespace AikariLauncherPublic::Constants::WebSocket::Basic
