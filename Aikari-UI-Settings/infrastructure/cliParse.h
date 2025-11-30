#pragma once

#include <Aikari-UI-Settings-Private/types/infra/cliOpts.h>

namespace AikariUI::Settings::Infrastructure::CliParse
{
    AikariUI::Settings::Includes::Private::Types::Infrastructure::CliOpts::
        CliOptions
        parseCliOpts(const int& argc, char* argv[]);
}
