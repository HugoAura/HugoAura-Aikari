project_add_library(Aikari-PLS SHARED
    dllmain.cpp
    entrypoint.cpp
    init.cpp
    init.h
    lifecycle.cpp
    lifecycle.h
    components/mqttBroker.cpp
    components/mqttBroker.h
    infrastructure/threadMsgHandler.cpp
    infrastructure/threadMsgHandler.h
    utils/windowsApi.cpp
    utils/windowsApi.h
)

# --- Packages --- #

# SPDLog
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(Aikari-PLS PRIVATE spdlog::spdlog_header_only)

# Nlohmann/JSON
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Aikari-PLS PRIVATE nlohmann_json::nlohmann_json)

# mbedTLS
find_package(MbedTLS CONFIG REQUIRED)
target_link_libraries(Aikari-PLS PRIVATE MbedTLS::mbedtls)

target_precompile_headers(Aikari-PLS
  PRIVATE
    <string>
    <vector>
    <memory>
    <spdlog/spdlog.h>
    <nlohmann/json.hpp>
)

# --- Defs --- #
target_compile_definitions(Aikari-PLS
  PRIVATE
    AIKARIPLS_EXPORTS
)

# --- Link --- #
target_link_libraries(Aikari-PLS
  PUBLIC
    Aikari-Shared
)

# --- Include --- #
target_include_directories(Aikari-PLS
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(Aikari-PLS
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Aikari-Launcher/includes
)

target_include_directories(Aikari-PLS
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
)
