#pragma once

#include <Aikari-Launcher-Private/types/infrastructure/cliParseTypes.h>

namespace cxxopts
{
    class Options;
}

namespace AikariCliUtils
{
    cxxopts::Options constructCliOptions();
    AikariTypes::Infrastructure::CLIParse::CLIOptionsRet parseCliOptions(
        cxxopts::Options& cliOptions, const int& argc, const char* argv[]
    );
}  // namespace AikariCliUtils
