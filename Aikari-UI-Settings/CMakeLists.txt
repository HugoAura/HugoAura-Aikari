# --- Early Imports & Options --- #
find_package(Qt6 CONFIG REQUIRED COMPONENTS Concurrent QmlCore Quick Core Gui Network)
find_package(Qt6Qml CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- Sources --- #
project_add_qt_executable(Aikari-UI-Settings WIN32
        entrypoint.cpp
        lifecycle.cpp
        lifecycle.h
        infrastructure/cliParse.cpp
        infrastructure/cliParse.h
        infrastructure/instances.cpp
        infrastructure/instances.h
        qml/qml.qrc
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

# QCoro
find_package(QCoro6Coro CONFIG REQUIRED)
find_package(QCoro6Core CONFIG REQUIRED)
find_package(QCoro6Network CONFIG REQUIRED)
find_package(QCoro6Qml CONFIG REQUIRED)
find_package(QCoro6Quick CONFIG REQUIRED)
target_link_libraries(Aikari-UI-Settings PRIVATE QCoro6::Coro)
target_link_libraries(Aikari-UI-Settings PRIVATE QCoro6::Core)
target_link_libraries(Aikari-UI-Settings PRIVATE QCoro6::Network)
target_link_libraries(Aikari-UI-Settings PRIVATE QCoro6::Qml)
target_link_libraries(Aikari-UI-Settings PRIVATE QCoro6::Quick)

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

# --- Registration --- #

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Reg QML modules
include(RegQMLModules)

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

target_include_directories(Aikari-UI-Settings
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/bridges/virtual
        ${CMAKE_CURRENT_SOURCE_DIR}/bridges/impl
)
