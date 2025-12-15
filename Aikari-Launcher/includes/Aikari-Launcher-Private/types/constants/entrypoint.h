#pragma once

namespace AikariTypes::Constants::Entrypoint
{
    namespace EXIT_CODES
    {
        constexpr int NORMAL_EXIT = 0;
        constexpr int MODULE_LOAD_FAILED = -1;
        constexpr int SERVICE_ACTION_FAILED = -2;
        constexpr int FS_INIT_FAILED = -3;
        constexpr int REG_INIT_FAILED = -4;
        constexpr int CONFIG_INIT_FAILED = -5;
        constexpr int NETWORK_SERVICES_INIT_FAILED = -6;
        constexpr int SSL_COMPONENTS_UNUSABLE = -7;
        constexpr int HINS_GET_FAILED = -9;  // Windows hInstance
        constexpr int SELF_PATH_GET_FAILED = -10;
        constexpr int TZ_INIT_FAILED = -11;

        constexpr int MULTI_AIKARI_INSTANCE_DETECTED = -20;
    }  // namespace EXIT_CODES
}  // namespace AikariTypes::Constants::Entrypoint
