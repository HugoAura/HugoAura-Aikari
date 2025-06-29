#ifndef PCH_H
#define PCH_H

#ifdef _DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif

#define WIN32_LEAN_AND_MEAN

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>

#endif  // PCH_H
