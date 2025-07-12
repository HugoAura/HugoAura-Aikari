project_add_library(Aikari-PLS SHARED
    dllmain.cpp
    entrypoint.cpp
    init.h
    init.cpp
    lifecycle.h
    lifecycle.cpp
    resource.h
    Aikari-PLS.rc
    components/mqttBroker.h
    components/mqttBroker.cpp
    components/mqttBrokerHandler.h
    components/mqttBrokerHandler.cpp
    components/mqttClient.h
    components/mqttClient.cpp
    infrastructure/threadMsgHandler.h
    infrastructure/threadMsgHandler.cpp
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

# cpr
find_package(cpr CONFIG REQUIRED)
target_link_libraries(Aikari-PLS PRIVATE cpr::cpr)

# async_mqtt
find_package(async_mqtt_iface CONFIG REQUIRED)
target_link_libraries(Aikari-PLS PRIVATE async_mqtt_iface::async_mqtt_iface)

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
