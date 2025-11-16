#pragma once

#include <Aikari-Shared/types/itc/shared.h>

namespace AikariLauncher::Routes::InterThread::FileSystem
{
    AikariShared::Types::InterThread::MainToSubControlReplyMessage
    handleFsCtrlMessage(
        const AikariShared::Types::InterThread::SubToMainControlMessage&
            incoming,
        const std::vector<std::string>& routes
    );
}
