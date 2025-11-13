#include "lifecycleUtils.h"

#include "../components/config.h"

namespace AikariLifecycle::Utils
{
    namespace Config
    {
        void editConfig(
            std::function<AikariTypes::Config::
                              AikariConfig(AikariTypes::Config::AikariConfig &)>
                lambda,
            bool writeConfig
        )
        {
            auto &sharedIns =
                AikariLifecycle::AikariSharedInstances::getInstance();
            auto *configManagerIns =
                sharedIns.getPtr(&AikariTypes::Global::Lifecycle::
                                     SharedInstances::configManagerIns);
            AikariTypes::Config::AikariConfig curConfig =
                *std::atomic_load(&configManagerIns->config);
            auto newConfig =
                std::make_shared<AikariTypes::Config::AikariConfig>(
                    lambda(curConfig)
                );
            configManagerIns->configEditLock.lock();
            std::atomic_store(&configManagerIns->config, newConfig);
            configManagerIns->configEditLock.unlock();
            if (writeConfig)
            {
                configManagerIns->writeConfig();
            }
        };
    }  // namespace Config
};  // namespace AikariLifecycle::Utils
