#include "./instances.h"

#include <QtQml/QtQml>

#include "bridges/impl/LifecycleBridge.h"
#include "bridges/impl/ThemeBridge.h"

namespace AikariUI::Settings::Infrastructure::Instances
{
    void initGlobalInstances()
    {
        if (globalInstancesInitialized)
            return;
        globalInstancesInitialized.store(true);
        /*
        const char* REG_URI = "AikariUI.Settings.Backend";
        std::vector<int> VERSION = { 1, 0 };
        */
        // >>> REG: LifecycleBridge >>> //
        auto* lifecycleBridge =
            AikariUI::Settings::Bridges::Instance::LifecycleBridge::getInstance(
            );
        // <<< DONE: LifecycleBridge <<< //

        // >>> REG: ThemeBridge >>> //
        auto* themeBridge =
            AikariUI::Settings::Bridges::Instance::ThemeBridge::getInstance();
        /* However, themeBridge will be firstly inited in cliParse */
        // <<< DONE: ThemeBridge <<< //
    }

}  // namespace AikariUI::Settings::Infrastructure::Instances
