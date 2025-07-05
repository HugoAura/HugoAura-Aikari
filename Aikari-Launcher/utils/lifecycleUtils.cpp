#include "lifecycleUtils.h"

#include "../components/config.h"

namespace AikariLifecycle::Utils
{
    namespace Config
    {
        void editConfig(
            std::function<
                void(std::shared_ptr<AikariTypes::config::AikariConfig> &)>
                lambda,
            bool writeConfig
        )
        {
            auto &sharedIns =
                AikariLifecycle::AikariSharedInstances::getInstance();
            auto *configManagerIns =
                sharedIns.getPtr(&AikariTypes::global::lifecycle::
                                     SharedInstances::configManagerIns);
            configManagerIns->configEditLock.lock();
            lambda(configManagerIns->config);
            configManagerIns->configEditLock.unlock();
            if (writeConfig)
            {
                configManagerIns->writeConfig();
            }
        };
    }  // namespace Config
};  // namespace AikariLifecycle::Utils
