#pragma once

#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Shared/virtual/itc/ISubToMainMsgHandlerBase.h>

namespace AikariShared::infrastructure::MessageQueue
{
    template <typename T>
    class SinglePointMessageQueue;
}

namespace AikariShared::Types::InterThread
{
    struct SubToMainMessageInstance;
}

namespace AikariLauncherComponents::SubModuleSystem::ThreadMsgHandlers
{
    class PLSMsgHandler : public AikariShared::infrastructure::InterThread::
                              SubToMainMsgHandlerBase
    {
       public:
        using AikariShared::infrastructure::InterThread::
            SubToMainMsgHandlerBase::SubToMainMsgHandlerBase;

       protected:
        void onControlMessage(
            const AikariShared::Types::InterThread::SubToMainControlMessage&
                retMsg
        ) override final;

        void onWebSocketMessage(
            const AikariShared::Types::InterThread::SubToMainWebSocketReply&
                wsReply
        ) override final;
    };
}  // namespace AikariLauncherComponents::SubModuleSystem::ThreadMsgHandlers
