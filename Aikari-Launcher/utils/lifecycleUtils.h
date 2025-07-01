#pragma once

#include <Aikari-Launcher-Private/types/config/configType.h>
#include <functional>

#include "../lifecycle.h"

namespace AikariLifecycle::Utils
{
namespace Config
{
void editConfig(
    std::function<void(std::shared_ptr<AikariTypes::config::AikariConfig> &)>
        lambda,
    bool writeConfig = false
);
};  // namespace Config
};  // namespace AikariLifecycle::Utils
