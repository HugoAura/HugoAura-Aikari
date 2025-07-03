#include "pch.h"

#include "threadMsgHandler.h"

#include <Aikari-Shared/infrastructure/loggerMacro.h>

namespace AikariPLS::Infrastructure::MsgQueue
{
    void PLSThreadMsgQueueHandler::onControlMessage(
        const AikariShared::Types::InterThread::MainToSubControlMessage& retMsg
    )
    {
#ifdef _DEBUG
        LOG_TRACE(
            "{} Received ctrl message, method: {}, data:\n{}",
            this->logHeader,
            retMsg.method,
            retMsg.data.dump()
        );
#endif
    };

    void PLSThreadMsgQueueHandler::onWebSocketMessage(
        const AikariShared::Types::InterThread::MainToSubWebSocketMessage&
            retMsg
    )
    {
#ifdef _DEBUG
        LOG_TRACE(
            "{} Received ws message, method: {}, client: {}, data:\n{}",
            this->logHeader,
            retMsg.method,
            retMsg.wsInfo.clientId.value_or("UNKNOWN"),
            retMsg.data.dump()
        );
#endif
    };
};  // namespace AikariPLS::Infrastructure::MsgQueue
