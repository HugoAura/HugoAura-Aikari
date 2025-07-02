#pragma once

namespace AikariLauncherPublic::Constants::InterThread::Network
{
constexpr const char* PREFIX = "network";
namespace TLS
{
constexpr const char* PREFIX = "tls";

constexpr const char* GEN_TLS_CERTS = "genTlsCert";
/*
GenTlsCert -> 调用 sslUtils 里的那个玩意
Data IN ←
{
    "baseDir": "/path/to/cert/dir/base",
    "hostname": "genshin.hoyoverse.fqdn",
    "identifier": "wss"
}
Data REP →
{
    "message": "Successfully generated" || err.what(),
    "diagnoseCode": "E_ONLY_IF_ERROR_OCCURRED (OPTIONAL)"
}
success = isGenSuccess;
code = [0 == "成功"] // [-1 == "失败"]
*/
}  // namespace TLS
}  // namespace AikariLauncherPublic::Constants::InterThread::Network
