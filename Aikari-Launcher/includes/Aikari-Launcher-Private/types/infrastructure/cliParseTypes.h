#pragma once

#include <Aikari-Launcher-Private/common.h>
#include <string>

namespace AikariTypes::Infrastructure::CLIParse
{
    struct CLIOptionsRet
    {
        bool isDebug = false;
        std::string serviceCtrl = "";
        std::string logMode = "";
        bool exitNow = false;
    };
}  // namespace AikariTypes::Infrastructure::CLIParse
