#include "cliParse.h"

#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <unordered_set>

namespace CliOptsType =
    AikariUI::Settings::Includes::Private::Types::Infrastructure::CliOpts;

namespace AikariUI::Settings::Infrastructure::CliParse
{
    CliOptsType::CliOptions parseCliOpts(const int& argc, char* argv[])
    {
        cxxopts::Options cliOptions{ "Aikari Settings UI" };
        // clang-format off
        cliOptions.add_options()
            (
                "h,help",
                "Print help"
            )
            (
                "fcm,forceColorTheme",
                "Force UI to use target color scheme (\"unset\", \"light\", \"dark\")",
                cxxopts::value<std::string>()->default_value("unset")
            );
        // clang-format on
        cliOptions.allow_unrecognised_options();

        auto parseResult = cliOptions.parse(argc, argv);
        CliOptsType::CliOptions retOptions;
        if (parseResult.contains("help"))
        {
            std::cout << cliOptions.help() << std::endl;
            retOptions.fastFail = true;
            return retOptions;
        }

        try
        {
            retOptions.forceColorTheme =
                parseResult["forceColorTheme"].as<std::string>();
            if (std::ranges::contains(
                    std::array{ "unset", "light", "dark" },
                    retOptions.forceColorTheme
                ))
            {
                throw std::logic_error(
                    "Invalid option \"--forceColorTheme " +
                    retOptions.forceColorTheme + "\""
                );
            }
            // ...
        }
        catch (const std::exception& e)
        {
            std::cerr << "[INFRA ERROR] CLI Options parse error:" << e.what()
                      << std::endl;
            retOptions.fastFail = true;
            return retOptions;
        }

        return retOptions;
    };
}  // namespace AikariUI::Settings::Infrastructure::CliParse
