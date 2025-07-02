#pragma once

#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>
#include <Aikari-Shared/virtual/itc/IMainToSubMsgHandlerBase.h>
#include <memory>
#include <string>

namespace AikariPLS::Infrastructure::MsgQueue
{

class PLSThreadMsgQueueHandler
    : public AikariShared::infrastructure::InterThread::MainToSubMsgHandlerBase
{
   public:
    using AikariShared::infrastructure::InterThread::MainToSubMsgHandlerBase::
        MainToSubMsgHandlerBase;

   protected:
    void onControlMessage(
        const AikariShared::Types::InterThread::MainToSubControlMessage& retMsg
    ) override final;

    void onWebSocketMessage(
        const AikariShared::Types::InterThread::MainToSubWebSocketMessage&
            retMsg
    ) override final;
};
}  // namespace AikariPLS::Infrastructure::MsgQueue
