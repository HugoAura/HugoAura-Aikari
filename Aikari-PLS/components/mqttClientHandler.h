#pragma once

#include <Aikari-PLS-Private/types/components/mqtt.h>
#include <async_mqtt/protocol/connection.hpp>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <unordered_set>

namespace AikariPLS::Components::MQTTClient::Class
{
    using ClientConnection = async_mqtt::connection<async_mqtt::role::client>;

    namespace Constants
    {
        constexpr const char* ascendOper = "<↑ ASCEND>";
        constexpr const char* recvOper = "<↓ INCOME>";
        constexpr const char* dataHr = "----- 📦 DATA -----";
        constexpr const char* propHr = "----- ✍️ PROPS -----";
    }  // namespace Constants

    namespace Types
    {
        typedef std::string OriginalID;
        typedef std::string ReplacedID;
    }  // namespace Types

    class MQTTClientConnection : public ClientConnection
    {
       public:
        MQTTClientConnection(
            std::function<void(const async_mqtt::packet_variant& packet)>
                onAscendLambda,
            std::function<void()> onCloseLambda,
            std::function<void(async_mqtt::error_code errCode)> onErrorLambda,
            std::function<void()> onConnAckSuccessLambda
        );

        void checkTimerTimeout();

        std::unordered_set<std::string> endpointGetIgnoredIds;
        std::unordered_set<std::string> endpointRpcIgnoredIds;

        std::unordered_map<Types::ReplacedID, Types::OriginalID>
            endpointGetIdsMap;

        std::atomic<int> endpointGetMsgIdCounter = 1;

       protected:
        void on_send(
            async_mqtt::packet_variant packet,
            std::optional<async_mqtt::packet_id_type>
                release_packet_id_if_send_error = std::nullopt
        ) override final;

        void on_receive(async_mqtt::packet_variant packet) override final;

        void on_close() override final;
        void on_error(async_mqtt::error_code errCode) override final;
        void on_packet_id_release(
            async_mqtt::packet_id_type packetId
        ) override final;
        void on_timer_op(
            async_mqtt::timer_op op,
            async_mqtt::timer_kind kind,
            std::optional<std::chrono::milliseconds> ms
        ) override final;

       private:
        std::function<void(const async_mqtt::packet_variant& packet)>
            onAscendLambda_;
        std::function<void()> onCloseLambda_;
        std::function<void(async_mqtt::error_code errCode)> onErrorLambda_;
        std::function<void()> onConnAckSuccessLambda_;

        std::vector<async_mqtt::packet_variant> pktTempStore;

        async_mqtt::packet_id_type getPacketId();

        void logSendPacket(const async_mqtt::packet_variant& packet) const;

        std::chrono::time_point<std::chrono::steady_clock> nextPingReqEtc;
        std::chrono::seconds keepAliveDuration;

        bool isConnected = false;
        bool isDebugEnv = false;
    };
}  // namespace AikariPLS::Components::MQTTClient::Class
