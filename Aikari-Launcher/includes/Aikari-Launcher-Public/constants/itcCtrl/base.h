#pragma once

namespace AikariLauncher::Public::Constants::InterThread::Base
{
    constexpr const char* _PREFIX = "base";
    namespace Props
    {
        constexpr const char* _PREFIX = "props";

        constexpr const char* GET_RUNTIME_MODE = "base.props.getRuntimeMode";
        /*
        GetRuntimeMode -> 获取运行模式
        Data IN ←
        { }
        Data REP →
        {
            "mode": enum class to int
        (AikariLauncher::Public::Constants::Lifecycle::APPLICATION_RUNTIME_MODES)
        }
        */
    }  // namespace Props
}  // namespace AikariLauncher::Public::Constants::InterThread::Base
