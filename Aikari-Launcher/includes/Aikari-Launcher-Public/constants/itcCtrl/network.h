#pragma once

namespace AikariLauncherPublic::Constants::InterThread::Network
{
    constexpr const char* _PREFIX = "network";
    namespace TLS
    {
        constexpr const char* _PREFIX = "tls";

        constexpr const char* GEN_TLS_CERTS = "network.tls.genTlsCert";
        /*
        GenTlsCert -> 调用 sslUtils 里的那个玩意
        Data IN ←
        {
            "baseDir": "/path/to/cert/dir/base", <OPTIONAL>
            "hostname": "genshin.hoyoverse.fqdn",
            "identifier": "wss"
        }
        Data REP →
        {
            "message": "Successfully generated" || err.what(),
            "diagnoseCode": "E_ONLY_IF_ERROR_OCCURRED", <OPTIONAL>
            "success": bool
        }
        */
    }  // namespace TLS
}  // namespace AikariLauncherPublic::Constants::InterThread::Network
