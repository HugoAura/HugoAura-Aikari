#pragma once

#include <Aikari-Launcher-Private/types/config/configType.h>
#include <functional>

#include "../lifecycle.h"

namespace AikariLifecycle::Utils
{
    namespace Config
    {
        void editConfig(
            std::function<AikariTypes::Config::
                              AikariConfig(AikariTypes::Config::AikariConfig &)>
                lambda,
            bool writeConfig
        );
    };  // namespace Config
};  // namespace AikariLifecycle::Utils
