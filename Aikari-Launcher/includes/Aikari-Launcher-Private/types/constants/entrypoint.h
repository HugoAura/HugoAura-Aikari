#pragma once

namespace AikariTypes::Constants::Entrypoint
{
    namespace EXIT_CODES
    {
        constexpr int NORMAL_EXIT = 0;
        constexpr int MODULE_LOAD_FAILED = -1;
        constexpr int SERVICE_ACTION_FAILED = -2;
        constexpr int HINS_GET_FAILED = -9;
    }  // namespace EXIT_CODES
}  // namespace AikariTypes::Constants::Entrypoint
