#pragma once

namespace AikariTypes::constants::entrypoint
{
    namespace EXIT_CODES
    {
        constexpr int NORMAL_EXIT = 0;
        constexpr int MODULE_LOAD_FAILED = -1;
        constexpr int SERVICE_ACTION_FAILED = -2;
        constexpr int HINS_GET_FAILED = -9;
    }
}
