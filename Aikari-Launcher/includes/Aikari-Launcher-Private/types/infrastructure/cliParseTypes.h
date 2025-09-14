#pragma once

#include <Aikari-Launcher-Private/common.h>
#include <string>

namespace AikariTypes::Infrastructure::CLIParse
{
    struct CLIOptionsRet
    {
        bool isDebug = false;
        std::string serviceCtrl = "";
    };
}  // namespace AikariTypes::Infrastructure::CLIParse
