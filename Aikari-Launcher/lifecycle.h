#pragma once

#include <Aikari-Launcher-Private/types/global/lifecycleTypes.h>
#include <Aikari-Shared/base/SingleIns.hpp>

namespace AikariLifecycle
{
    typedef AikariShared::Base::AikariStatesManagerTemplate<
        AikariTypes::Global::Lifecycle::GlobalLifecycleStates>
        AikariStatesManager;

    typedef AikariShared::Base::AikariStatesManagerTemplate<
        AikariTypes::Global::Lifecycle::SharedInstances>
        AikariSharedInstances;

    typedef AikariShared::Base::AikariStatesManagerTemplate<
        AikariTypes::Global::Lifecycle::GlobalSharedHandlersRegistry>
        AikariSharedHandlers;
}  // namespace AikariLifecycle
