#pragma once

#include <Aikari-Shared/virtual/itc/ISubToMainMsgHandlerBase.h>

namespace AikariShared::Infrastructure::MessageQueue
{
    template <typename T>
    class SinglePointMessageQueue;
}

namespace AikariShared::Types::InterThread
{
    struct SubToMainMessageInstance;
}

namespace AikariLauncher::Components::SubModuleSystem::ThreadMsgHandlers
{
    class PLSMsgHandler : public AikariShared::Infrastructure::InterThread::
                              SubToMainMsgHandlerBase
    {
       public:
        using AikariShared::Infrastructure::InterThread::
            SubToMainMsgHandlerBase::SubToMainMsgHandlerBase;

       protected:
        void onControlMessage(
            AikariShared::Types::InterThread::SubToMainControlMessage& retMsg
        ) override final;

        void onWebSocketMessage(
            AikariShared::Types::InterThread::SubToMainWebSocketReply& wsReply
        ) override final;
    };
}  // namespace AikariLauncher::Components::SubModuleSystem::ThreadMsgHandlers
