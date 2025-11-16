#pragma once

#include <Aikari-Launcher-Private/types/infrastructure/cliParseTypes.h>

namespace AikariCliUtils
{
    AikariTypes::Infrastructure::CLIParse::CLIOptionsRet parseCliOptions(
        const int& argc, const char* argv[]
    );
}  // namespace AikariCliUtils
