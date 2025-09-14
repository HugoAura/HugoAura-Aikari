#pragma once

#include <Aikari-PLS-Private/types/lifecycleTypes.h>
#include <Aikari-Shared/base/SingleIns.hpp>

namespace AikariPLS::Lifecycle
{
    typedef AikariShared::Base::AikariStatesManagerTemplate<
        AikariPLS::Types::Lifecycle::PLSSharedStates>
        PLSSharedStates;

    typedef AikariShared::Base::AikariStatesManagerTemplate<
        AikariPLS::Types::Lifecycle::PLSSharedIns>
        PLSSharedInsManager;

    typedef AikariShared::Base::AikariStatesManagerTemplate<
        AikariPLS::Types::Lifecycle::PLSSharedMsgQueues>
        PLSSharedQueuesManager;
}  // namespace AikariPLS::Lifecycle
