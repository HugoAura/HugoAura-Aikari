#pragma once
#include <Aikari-Shared/base/SingleIns.hpp>
#include <Aikari-UI-Settings-Private/types/infra/lifecycle.h>

namespace AikariUI::Settings::Lifecycle
{
    typedef AikariShared::Base::AikariStatesManagerTemplate<
        AikariUI::Settings::Includes::Private::Types::Infrastructure::
            Lifecycle::LifecycleStates>
        AikariUISettingsStates;
}
