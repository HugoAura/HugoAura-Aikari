#pragma once

namespace AikariPLS::Types::constants::init
{
    namespace networkInit
    {
        const char* SW_CORE_PROC_KILLED_MSG =
            "SeewoCore process has been killed, SSA might "
            "be reloaded in seconds.";
        const char* SW_CORE_PROC_KILLED_PUSH_METHOD =
            "$aura.pls.init.pushSwCoreOnKillEvent";

        const char* HOSTNAME = "iot-broker-mis.seewo.com";
        const int PORT = 8883;
        const char* TLS_CERT_IDENTIFIER = "mqtt";
    }  // namespace networkInit
}  // namespace AikariPLS::Types::constants::init
