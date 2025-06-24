#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

#define DEFAULT_LOGGER (spdlog::get("defaultLogger"))

#define LOG_TRACE(...) SPDLOG_LOGGER_TRACE(DEFAULT_LOGGER, __VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(DEFAULT_LOGGER, __VA_ARGS__)
#define LOG_INFO(...) SPDLOG_LOGGER_INFO(DEFAULT_LOGGER, __VA_ARGS__)
#define LOG_WARN(...) SPDLOG_LOGGER_WARN(DEFAULT_LOGGER, __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(DEFAULT_LOGGER, __VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(DEFAULT_LOGGER, __VA_ARGS__)

namespace AikariDefaults
{
inline const std::string Version = "v0.0.1-early-alpha";

inline const unsigned int VersionCode = 1;

namespace ServiceConfig
{
inline const std::wstring ServiceName = L"HugoAuraAikari";
inline const std::wstring ServiceDisplayName = L"HugoAura Aikari";
inline const std::wstring ServiceDesc =
    L"Aikari, privileged access services for HugoAura.";
inline const std::wstring StartArg = L"--service run-as";
}  // namespace ServiceConfig
}  // namespace AikariDefaults
