#pragma once

namespace AikariDefaults
{
    namespace ServiceConfig
    {
        inline const std::wstring ServiceName = L"HugoAuraAikari";
        inline const std::wstring ServiceDisplayName = L"HugoAura Aikari";
        inline const std::wstring ServiceDesc =
            L"Aikari, unified UX tweak tool for Seewo family softwares.";
        inline const std::wstring StartArg = L"--service runAs";
    }  // namespace ServiceConfig

    constexpr const char* mutexName =
        "Global\\Aikari_X_{fcb7a062-ae78-451b-b7a0-62ae78b51ba2}";
}  // namespace AikariDefaults
