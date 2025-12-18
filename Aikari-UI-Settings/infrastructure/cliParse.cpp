#include "cliParse.h"

#include <Aikari-UI-Settings-Private/types/bridges/theme.h>
#include <cxxopts.hpp>
#include <iostream>
#include <string>

#include "bridges/impl/ThemeBridge.h"
#include "lifecycle.h"

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
                R"(Force UI to use target color scheme ("unset", "light", "dark"))",
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
            if (!std::ranges::contains(
                    std::array{ "unset", "light", "dark" },
                    retOptions.forceColorTheme
                ))
            {
                throw std::logic_error(
                    "Invalid option \"--forceColorTheme " +
                    retOptions.forceColorTheme + "\""
                );
            }
            auto& lifecycleStates = AikariUI::Settings::Lifecycle::
                AikariUISettingsStates::getInstance();
            auto forceThemeMode = AikariUI::Settings::Includes::Private::Types::
                Bridges::Theme::ForceThemeMode::Enum::UNSET;
            if (retOptions.forceColorTheme == "light")
            {
                forceThemeMode = AikariUI::Settings::Includes::Private::Types::
                    Bridges::Theme::ForceThemeMode::Enum::LIGHT;
            }
            else if (retOptions.forceColorTheme == "dark")
            {
                forceThemeMode = AikariUI::Settings::Includes::Private::Types::
                    Bridges::Theme::ForceThemeMode::Enum::DARK;
            }
            auto* themeBridge =
                AikariUI::Settings::Bridges::Instance::ThemeBridge::getInstance(
                );
            themeBridge->setForceThemeMode(forceThemeMode);
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
