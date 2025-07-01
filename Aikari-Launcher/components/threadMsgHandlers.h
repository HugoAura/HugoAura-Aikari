#pragma once

#include <Aikari-Launcher-Private/common.h>

namespace AikariShared::infrastructure::MessageQueue
{
template <typename T>
class SinglePointMessageQueue;
}

namespace AikariPLS::Types::infrastructure
{
struct RetMessageStruct;
}

namespace AikariLauncherComponents::SubModuleSystem::ThreadMsgHandlers
{
void plsIncomingMsgHandler(
    AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
        AikariPLS::Types::infrastructure::RetMessageStruct>* retMsgQueue
);
}  // namespace AikariLauncherComponents::SubModuleSystem::ThreadMsgHandlers
