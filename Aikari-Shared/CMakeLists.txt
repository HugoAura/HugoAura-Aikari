project_add_library(Aikari-Shared SHARED
        Aikari-Shared.cpp
        infrastructure/logger.cpp
        infrastructure/telemetry.cpp
        utils/crypto.cpp
        utils/cstring.cpp
        utils/filesystem.cpp
        utils/network.cpp
        utils/random.cpp
        utils/registry.cpp
        utils/string.cpp
        utils/windows/network.cpp
        utils/windows/process.cpp
        utils/windows/winString.cpp
        virtual/IConfigManager.cpp
        virtual/itc/IMainToSubMsgHandlerBase.cpp
        virtual/itc/ISubToMainMsgHandlerBase.cpp
)

set_target_properties(Aikari-Shared PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# --- Packages --- #

# Nlohmann/JSON
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Aikari-Shared PUBLIC nlohmann_json::nlohmann_json)

# cpr
find_package(cpr CONFIG REQUIRED)
target_link_libraries(Aikari-Shared PUBLIC cpr::cpr)

# SPDLog
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(Aikari-Shared PUBLIC spdlog::spdlog)

# Sentry Native SDK
find_package(sentry CONFIG REQUIRED)
target_link_libraries(Aikari-Shared PRIVATE sentry::sentry)

target_precompile_headers(Aikari-Shared
        PUBLIC
        <string>
        <vector>
        <windows.h>
        <spdlog/spdlog.h>
        <nlohmann/json.hpp>
)

# --- Options --- #
target_compile_definitions(Aikari-Shared PUBLIC
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _WIN32_WINNT=0x0601
)

target_compile_definitions(Aikari-Shared PRIVATE
        AIKARI_MODULE_NAME="Aikari-Shared"
        AIKARI_LOGGER_MODULE_NAME="Aikari-Any"
)

# --- Link --- #

# --- Include --- #
target_include_directories(Aikari-Shared
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(Aikari-Shared
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/includes
)
