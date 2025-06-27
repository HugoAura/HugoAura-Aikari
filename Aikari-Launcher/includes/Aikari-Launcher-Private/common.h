#pragma once

#include <Aikari-Shared/infrastructure/logger.h>

namespace AikariDefaults
{
namespace ServiceConfig
{
inline const std::wstring ServiceName = L"HugoAuraAikari";
inline const std::wstring ServiceDisplayName = L"HugoAura Aikari";
inline const std::wstring ServiceDesc =
    L"Aikari, privileged access services for HugoAura.";
inline const std::wstring StartArg = L"--service run-as";
}  // namespace ServiceConfig
}  // namespace AikariDefaults
