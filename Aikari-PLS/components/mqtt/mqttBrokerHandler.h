#pragma once

#include <async_mqtt/protocol/connection.hpp>
#include <chrono>
#include <optional>

namespace AikariPLS::Components::MQTTBroker::Class
{
    using BrokerConnection = async_mqtt::connection<async_mqtt::role::server>;

    namespace Constants
    {
        constexpr const char* ascendOper = "<↑ REPLY>";
        constexpr const char* recvOper = "<↓ ENTER>";
        constexpr const char* dataHr = "----- 📦 DATA -----";
        constexpr const char* propHr = "----- ✍️ PROPS -----";

    }  // namespace Constants

    class MQTTBrokerConnection : public BrokerConnection
    {
       public:
        MQTTBrokerConnection(
            std::function<void(async_mqtt::packet_variant packet)> onSendLambda,
            std::function<void()> onCloseLambda,
            std::function<void(async_mqtt::error_code errCode)> onErrLambda
        );

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
        std::function<void(async_mqtt::packet_variant packet)> onSendLambda_;
        std::function<void()> onCloseLambda_;
        std::function<void(async_mqtt::error_code errCode)> onErrorLambda_;

        async_mqtt::packet_id_type getPacketId();

        void logSendPacket(const async_mqtt::packet_variant& packet);

        std::string clientId = "UNKNOWN";

        bool isDebugEnv = false;
        bool isSharedInfoInitialized = false;
    };
}  // namespace AikariPLS::Components::MQTTBroker::Class
