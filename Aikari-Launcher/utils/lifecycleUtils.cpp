#include "pch.h"

#include "lifecycleUtils.h"

#include "../components/config.h"

namespace AikariLifecycle::Utils
{
AikariTypes::global::lifecycle::SharedInstances getSharedIns()
{
    auto &lifecycleStates = AikariLifecycle::AikariStatesManager::getInstance();
    auto sharedIns = lifecycleStates.getVal(
        &AikariTypes::global::lifecycle::GlobalLifecycleStates::sharedIns
    );
    return sharedIns;
};

namespace Config
{
void editConfig(
    std::function<void(std::shared_ptr<AikariTypes::config::AikariConfig> &)>
        lambda,
    bool writeConfig
)
{
    auto sharedIns = AikariLifecycle::Utils::getSharedIns();
    auto &configManagerIns = sharedIns.configManagerIns;
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
