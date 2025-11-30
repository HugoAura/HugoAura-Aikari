project_add_library(Aikari-PLS SHARED
        dllmain.cpp
        entrypoint.cpp
        init.h
        init.cpp
        lifecycle.h
        lifecycle.cpp
        resource.h
        Aikari-PLS.rc
        components/infrastructure/config.h
        components/infrastructure/config.cpp
        components/mqtt/mqttBroker.h
        components/mqtt/mqttBroker.cpp
        components/mqtt/mqttBrokerHandler.h
        components/mqtt/mqttBrokerHandler.cpp
        components/mqtt/mqttClient.h
        components/mqtt/mqttClient.cpp
        components/mqtt/mqttClientHandler.h
        components/mqtt/mqttClientHandler.cpp
        components/mqtt/mqttLifecycle.h
        components/mqtt/mqttLifecycle.cpp
        components/ruleSystem/rulesManager.h
        components/ruleSystem/rulesManager.cpp
        components/ws/dispatcher.h
        components/ws/dispatcher.cpp
        infrastructure/threadMsgHandler.h
        infrastructure/threadMsgHandler.cpp
        routes/ws/configRoute.h
        routes/ws/configRoute.cpp
        typesImpl/components/mqtt.cpp
        typesImpl/components/rules.cpp
        utils/mqttPacketUtils.h
        utils/mqttPacketUtils.cpp
        utils/windowsApi.cpp
        utils/windowsApi.h
)

# --- Ext Src Defs --- #
set(AIKARI_PLS_STATIC_RESOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/config/default.json"
)

set_source_files_properties("Aikari-PLS.rc" PROPERTIES
        OBJECT_DEPENDS ${AIKARI_PLS_STATIC_RESOURCES}
)

# --- Packages --- #

# SPDLog
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(Aikari-PLS PRIVATE spdlog::spdlog)

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

# lua
find_package(Lua REQUIRED)
target_include_directories(Aikari-PLS PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(Aikari-PLS PRIVATE ${LUA_LIBRARIES})

# sol2
find_path(SOL2_INCLUDE_DIRS "sol/abort.hpp")
target_include_directories(Aikari-PLS PRIVATE ${SOL2_INCLUDE_DIRS})

# lua cjson
find_package(cjson CONFIG REQUIRED)
target_link_libraries(Aikari-PLS PRIVATE cjson::cjson)

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
        AIKARI_MODULE_NAME="Aikari-PLS"
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
) # for Aikari-Launcher-Public

target_include_directories(Aikari-PLS
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/includes
)

# --- Post Build CMDs --- #
add_custom_command(
        TARGET Aikari-PLS
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:cjson::cjson>
        ${CMAKE_BINARY_DIR}/artifacts/bin/$<CONFIG>
        COMMENT "Copying lua-cjson DLL to output directory"
)
