#pragma once

#include <format>
#include <spdlog/spdlog.h>

#define DEFAULT_LOGGER (spdlog::get("defaultLogger"))

#define LOG_TRACE(...) SPDLOG_LOGGER_TRACE(DEFAULT_LOGGER, __VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(DEFAULT_LOGGER, __VA_ARGS__)
#define LOG_INFO(...) SPDLOG_LOGGER_INFO(DEFAULT_LOGGER, __VA_ARGS__)
#define LOG_WARN(...) SPDLOG_LOGGER_WARN(DEFAULT_LOGGER, __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(DEFAULT_LOGGER, __VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(DEFAULT_LOGGER, __VA_ARGS__)

#ifdef CUSTOM_LOG_HEADER
#define CUSTOM_LOG_TRACE(...) \
    LOG_TRACE("{} {}", CUSTOM_LOG_HEADER, std::format(__VA_ARGS__))
#define CUSTOM_LOG_DEBUG(...) \
    LOG_DEBUG("{} {}", CUSTOM_LOG_HEADER, std::format(__VA_ARGS__))
#define CUSTOM_LOG_INFO(...) \
    LOG_INFO("{} {}", CUSTOM_LOG_HEADER, std::format(__VA_ARGS__))
#define CUSTOM_LOG_WARN(...) \
    LOG_WARN("{} {}", CUSTOM_LOG_HEADER, std::format(__VA_ARGS__))
#define CUSTOM_LOG_ERROR(...) \
    LOG_ERROR("{} {}", CUSTOM_LOG_HEADER, std::format(__VA_ARGS__))
#define CUSTOM_LOG_CRITICAL(...) \
    LOG_CRITICAL("{} {}", CUSTOM_LOG_HEADER, std::format(__VA_ARGS__))
#endif
