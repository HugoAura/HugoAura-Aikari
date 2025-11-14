#pragma once

#include <Aikari-Shared/virtual/itc/IMainToSubMsgHandlerBase.h>
#include <memory>

namespace AikariPLS::Infrastructure::MsgQueue
{

    class PLSThreadMsgQueueHandler : public AikariShared::Infrastructure::
                                         InterThread::MainToSubMsgHandlerBase
    {
       public:
        using AikariShared::Infrastructure::InterThread::
            MainToSubMsgHandlerBase::MainToSubMsgHandlerBase;

       protected:
        void onControlMessage(
            const AikariShared::Types::InterThread::MainToSubControlMessage&
                retMsg
        ) override final;

        void onWebSocketMessage(
            const AikariShared::Types::InterThread::MainToSubWebSocketMessage&
                retMsg
        ) override final;
    };
}  // namespace AikariPLS::Infrastructure::MsgQueue
