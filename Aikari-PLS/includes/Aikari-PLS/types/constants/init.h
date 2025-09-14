#pragma once

namespace AikariPLS::Types::Constants::Init
{
    namespace NetworkInit
    {
        constexpr const char* SW_CORE_PROC_KILLED_MSG =
            "SeewoCore process has been killed, SSA might "
            "be reloaded in seconds.";
        constexpr const char* SW_CORE_PROC_KILLED_PUSH_METHOD =
            "$aura.pls.init.pushSwCoreOnKillEvent";

        constexpr const char* HOSTNAME = "iot-broker-mis.seewo.com";
        constexpr const int PORT = 8883;
        constexpr const char* TLS_CERT_IDENTIFIER = "mqtt";
    }  // namespace NetworkInit
}  // namespace AikariPLS::Types::Constants::Init
