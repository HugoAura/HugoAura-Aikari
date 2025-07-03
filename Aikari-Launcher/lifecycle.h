#pragma once

#include <Aikari-Launcher-Private/types/global/lifecycleTypes.h>
#include <Aikari-Shared/base/SingleIns.hpp>
#include <mutex>

namespace AikariLifecycle
{
    typedef AikariShared::base::AikariStatesManagerTemplate<
        AikariTypes::global::lifecycle::GlobalLifecycleStates>
        AikariStatesManager;

    typedef AikariShared::base::AikariStatesManagerTemplate<
        AikariTypes::global::lifecycle::SharedInstances>
        AikariSharedInstances;

    typedef AikariShared::base::AikariStatesManagerTemplate<
        AikariTypes::global::lifecycle::GlobalSharedHandlersRegistry>
        AikariSharedHandlers;
}  // namespace AikariLifecycle
