﻿#pragma once

#include <Aikari-Shared/types/itc/shared.h>
#include <nlohmann/json.hpp>

namespace AikariLauncherRoutes::InterThread::Network
{
    AikariShared::Types::InterThread::MainToSubControlReplyMessage
    handleNetworkCtrlMessage(
        const AikariShared::Types::InterThread::SubToMainControlMessage&
            incoming,
        const std::vector<std::string>& routes
    );
}
