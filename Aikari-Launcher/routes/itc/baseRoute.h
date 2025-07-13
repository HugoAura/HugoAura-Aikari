#pragma once

#include <Aikari-Shared/types/itc/shared.h>

namespace AikariLauncherRoutes::InterThread::Base
{
    AikariShared::Types::InterThread::MainToSubControlReplyMessage
    handleBaseCtrlMsg(
        const AikariShared::Types::InterThread::SubToMainControlMessage&
            incoming,
        const std::vector<std::string>& routes
    );
}
