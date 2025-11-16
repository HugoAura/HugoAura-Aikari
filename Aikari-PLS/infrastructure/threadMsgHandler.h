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
            AikariShared::Types::InterThread::MainToSubControlMessage& srcMsg
        ) override final;

        void onWebSocketMessage(
            AikariShared::Types::InterThread::MainToSubWebSocketMessage& srcMsg
        ) override final;
    };
}  // namespace AikariPLS::Infrastructure::MsgQueue
