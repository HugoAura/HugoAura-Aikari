project_add_executable(Aikari-Launcher
    entrypoint.cpp
    lifecycle.cpp
    lifecycle.h
    pch.cpp
    pch.h
    resource.h
    Aikari-Launcher.rc
    components/config.cpp
    components/config.h
    components/threadMsgHandlers.cpp
    components/threadMsgHandlers.h
    components/wsMsgHandler.cpp
    components/wsMsgHandler.h
    components/wsServer.cpp
    components/wsServer.h
    infrastructure/cliParse.cpp
    infrastructure/cliParse.h
    infrastructure/fileSystem.cpp
    infrastructure/fileSystem.h
    infrastructure/registry.cpp
    infrastructure/registry.h
    infrastructure/winSvc.cpp
    infrastructure/winSvc.h
    middleware/wsAuthHandler.cpp
    middleware/wsAuthHandler.h
    routes/itc/filesystemRoute.cpp
    routes/itc/filesystemRoute.h
    routes/itc/networkRoute.cpp
    routes/itc/networkRoute.h
    utils/lifecycleUtils.cpp
    utils/lifecycleUtils.h
    utils/sslUtils.cpp
    utils/sslUtils.h
)

# --- Packages --- #

# SPDLog
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(Aikari-Launcher PRIVATE spdlog::spdlog_header_only)

# Nlohmann/JSON
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Aikari-Launcher PRIVATE nlohmann_json::nlohmann_json)

# ADA-URL
find_package(ada CONFIG REQUIRED)
target_link_libraries(Aikari-Launcher PRIVATE ada::ada)

# cxxopts
find_package(cxxopts CONFIG REQUIRED)
target_link_libraries(Aikari-Launcher PRIVATE cxxopts::cxxopts)

# mbedTLS
find_package(MbedTLS CONFIG REQUIRED)
target_link_libraries(Aikari-Launcher PRIVATE MbedTLS::mbedtls)

# IXWebSocket
find_package(ixwebsocket CONFIG REQUIRED)
target_link_libraries(Aikari-Launcher PRIVATE ixwebsocket::ixwebsocket)

# --- Link --- #
target_link_libraries(Aikari-Launcher
  PUBLIC
    Aikari-Shared
)

target_link_libraries(Aikari-Launcher
  PRIVATE
    Aikari-PLS
)

# --- Include --- #
target_include_directories(Aikari-Launcher
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(Aikari-Launcher
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
)
