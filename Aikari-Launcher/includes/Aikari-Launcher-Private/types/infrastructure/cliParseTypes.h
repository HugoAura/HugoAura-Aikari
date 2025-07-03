#pragma once

#include <Aikari-Launcher-Private/common.h>
#include <string>

namespace AikariTypes::infrastructure::cliParse
{

    struct CliOptionsRet
    {
        bool isDebug = false;
        std::string serviceCtrl = "";
    };
}  // namespace AikariTypes::infrastructure::cliParse
