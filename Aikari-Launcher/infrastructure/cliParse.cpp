#include "./cliParse.h"

#include <Aikari-Launcher-Private/types/infrastructure/cliParseTypes.h>
#include <cxxopts.hpp>
#include <iostream>

namespace AikariCliUtils
{
    AikariTypes::Infrastructure::CLIParse::CLIOptionsRet parseCliOptions(
        const int& argc, const char* argv[]
    )
    {
        cxxopts::Options cliOptions{ "Aikari",
                                     "Unified UX tweak tool for Seewo Hugo" };
        std::string strDefaultVal = "";
        // clang-format off
        cliOptions
            .add_options()
            (
                "d,debug", "Enable debug logging",
                cxxopts::value<bool>()->default_value("false")
            )
            (
                "s,service",
                "Windows SCM control options",
                cxxopts::value<std::string>()->default_value(strDefaultVal)
            )
            (
                "logMode", "Set logging mode (\"none\", \"tty\", \"ttyAndFile\", \"file\")",
                cxxopts::value<std::string>()->default_value(strDefaultVal)
            )
            (
                "h,help",
                "Print help"
            );
        // clang-format on
        cliOptions.allow_unrecognised_options();

        auto parseResult = cliOptions.parse(argc, argv);
        AikariTypes::Infrastructure::CLIParse::CLIOptionsRet retVal;
        if (parseResult.contains("help"))
        {
            std::cout << cliOptions.help({ "" }) << std::endl;
            retVal.exitNow = true;
            return retVal;
        }

        try
        {
            retVal.isDebug = parseResult["debug"].as<bool>();
        }
        catch (...)
        {
            std::cerr << "[ERROR] Invalid CLI Option: --debug" << std::endl;
            retVal.exitNow = true;
            return retVal;
        }

        retVal.serviceCtrl = parseResult["service"].as<std::string>();

        retVal.logMode = parseResult["logMode"].as<std::string>();
        if (retVal.logMode != "none" && retVal.logMode != "ttyAndFile" &&
            retVal.logMode != "tty" && retVal.logMode != "file")
        {
            std::cerr
                << "[WARN] Invalid CLI Option: --logMode, using tty as default."
                << std::endl;
            retVal.logMode = "tty";
        }

        return retVal;
    }
}  // namespace AikariCliUtils
