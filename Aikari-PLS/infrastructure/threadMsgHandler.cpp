#include "threadMsgHandler.h"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/types/itc/shared.h>
#include <string>

#include "../components/ws/dispatcher.h"

namespace AikariPLS::Infrastructure::MsgQueue
{
    void PLSThreadMsgQueueHandler::onControlMessage(
        AikariShared::Types::InterThread::MainToSubControlMessage& srcMsg
    )
    {
#ifdef _DEBUG
        LOG_TRACE(
            "{} Received ctrl message, method: {}, data:\n{}",
            this->logHeader,
            srcMsg.method,
            srcMsg.data.dump()
        );
#endif
    };

    void PLSThreadMsgQueueHandler::onWebSocketMessage(
        AikariShared::Types::InterThread::MainToSubWebSocketMessage& srcMsg
    )
    {
#ifdef _DEBUG
        LOG_TRACE(
            "{} Received ws message, method: {}, client: {}, data:\n{}",
            this->logHeader,
            srcMsg.method,
            srcMsg.wsInfo.clientId.value_or("UNKNOWN"),
            srcMsg.data.dump()
        );
#endif
        AikariShared::Types::InterThread::MainToSubWebSocketMessageCore
            clientMsgCore = { .method = std::move(srcMsg.method),
                              .data = std::move(srcMsg.data) };
        auto wsReply =
            AikariPLS::Components::WebSocket::Dispatcher::dispatchWsMsg(
                clientMsgCore
            );
        AikariShared::Types::InterThread::SubToMainWebSocketReply fullReply = {
            .success = wsReply.success,
            .code = wsReply.code,
            .data = std::move(wsReply.data),
            .method = std::move(wsReply.method),
            .eventId = std::move(srcMsg.eventId),
            .wsInfo = std::move(srcMsg.wsInfo),
        };
        AikariShared::Types::InterThread::SubToMainMessageInstance ins = {
            .type = AikariShared::Types::InterThread::MESSAGE_TYPES::WS_MESSAGE,
            .msg = std::move(fullReply),
        };
        this->destQueue->push(std::move(ins));
    };
};  // namespace AikariPLS::Infrastructure::MsgQueue
