#pragma once

#include <atomic>

namespace AikariUI::Settings::Infrastructure::Instances
{
    void initGlobalInstances();

    static std::atomic<bool> globalInstancesInitialized = false;
}  // namespace AikariUI::Settings::Infrastructure::Instances
