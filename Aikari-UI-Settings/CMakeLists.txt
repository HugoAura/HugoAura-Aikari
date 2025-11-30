# --- Early Imports & Options --- #
find_package(Qt6 CONFIG REQUIRED COMPONENTS Concurrent QmlCore Quick Core Gui Network)
find_package(Qt6Qml CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- Sources --- #
project_add_qt_executable(Aikari-UI-Settings WIN32
        entrypoint.cpp
        infrastructure/cliParse.cpp
        infrastructure/cliParse.h
        resources/qml.qrc
)

# --- Packages --- #

# QtBase
target_link_libraries(Aikari-UI-Settings PRIVATE Qt6::Concurrent)
target_link_libraries(Aikari-UI-Settings PRIVATE Qt6::Core)
target_link_libraries(Aikari-UI-Settings PRIVATE Qt6::Gui)
target_link_libraries(Aikari-UI-Settings PRIVATE Qt6::Network)

# QML
target_link_libraries(Aikari-UI-Settings PRIVATE Qt6::Quick)
target_link_libraries(Aikari-UI-Settings PRIVATE Qt6::QmlCore)

# Nlohmann/JSON
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Aikari-UI-Settings PRIVATE nlohmann_json::nlohmann_json)

# cxxopts
find_package(cxxopts CONFIG REQUIRED)
target_link_libraries(Aikari-UI-Settings PRIVATE cxxopts::cxxopts)

# QWindowKit
find_package(QWindowKit COMPONENTS Core Quick REQUIRED)
target_link_libraries(Aikari-UI-Settings PRIVATE QWindowKit::Core QWindowKit::Quick)

# PCH
target_precompile_headers(Aikari-UI-Settings
        PUBLIC
        <string>
        <format>
        <vector>
        <memory>
        <nlohmann/json.hpp>
)

# --- Options --- #
set_target_properties(Aikari-UI-Settings
        PROPERTIES
        WIN32_EXECUTABLE $<IF:$<CONFIG:Debug>,OFF,ON> # Let debug build hang the shell
)

# --- Link --- #
target_link_libraries(Aikari-UI-Settings
        PUBLIC
        Aikari-Shared
)

# --- Include --- #
target_include_directories(Aikari-UI-Settings
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(Aikari-UI-Settings
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/includes
)
