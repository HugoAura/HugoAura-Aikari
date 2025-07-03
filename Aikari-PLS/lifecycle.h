#pragma once

#include <Aikari-PLS-Private/types/lifecycleTypes.h>
#include <Aikari-Shared/base/SingleIns.hpp>

namespace AikariPLS::Lifecycle
{
    typedef AikariShared::base::AikariStatesManagerTemplate<
        AikariPLS::Types::lifecycle::PLSSharedIns>
        PLSSharedInsManager;

    typedef AikariShared::base::AikariStatesManagerTemplate<
        AikariPLS::Types::lifecycle::PLSSharedMsgQueues>
        PLSSharedQueuesManager;
}  // namespace AikariPLS::Lifecycle
