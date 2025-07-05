project_add_library(Aikari-Shared STATIC
    Aikari-Shared.cpp
    infrastructure/logger.cpp
    utils/crypto.cpp
    utils/registry.cpp
    utils/string.cpp
    utils/windows/network.cpp
    utils/windows/process.cpp
    virtual/IConfigManager.cpp
    virtual/itc/IMainToSubMsgHandlerBase.cpp
    virtual/itc/ISubToMainMsgHandlerBase.cpp
)

# --- Packages --- #

# Nlohmann/JSON
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Aikari-Shared PRIVATE nlohmann_json::nlohmann_json)

# SPDLog
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(Aikari-Shared PRIVATE spdlog::spdlog_header_only)

target_precompile_headers(Aikari-Shared
  PUBLIC
    <string>
    <vector>
    <spdlog/spdlog.h>
    <nlohmann/json.hpp>
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
