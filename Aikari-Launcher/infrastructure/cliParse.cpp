#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Launcher-Private/types/infrastructure/cliParseTypes.h>
#include <cxxopts.hpp>

namespace AikariCliUtils
{
    cxxopts::Options constructCliOptions()
    {
        cxxopts::Options cliOptions{
            "Aikari", "Privileged access services for HugoAura"
        };
        cliOptions
            .add_options()("d,debug", "Enable debug logging", cxxopts::value<bool>())(
                "s,service",
                "Windows SCM control options",
                cxxopts::value<std::string>()
            );
        return cliOptions;
    }

    AikariTypes::Infrastructure::CLIParse::CLIOptionsRet parseCliOptions(
        cxxopts::Options& cliOptions, const int& argc, const char* argv[]
    )
    {
        auto parseResult = cliOptions.parse(argc, argv);
        AikariTypes::Infrastructure::CLIParse::CLIOptionsRet retVal;
        try
        {
            retVal.isDebug = parseResult["debug"].as<bool>();
        }
        catch (const std::exception& err)
        {
            retVal.isDebug = false;
        }

        try
        {
            retVal.serviceCtrl = parseResult["service"].as<std::string>();
        }
        catch (const std::exception& err)
        {
            retVal.serviceCtrl = "";
        }

        return retVal;
    }
}  // namespace AikariCliUtils
