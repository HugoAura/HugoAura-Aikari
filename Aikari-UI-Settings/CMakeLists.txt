# --- Early Imports & Options --- #
find_package(Qt6 CONFIG REQUIRED COMPONENTS Concurrent QmlCore Quick Core Gui Network)
find_package(Qt6Qml CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(AIKARI_UI_SETTINGS_QRC_RESOURCE_PREFIX "/")

# --- Sources --- #
project_add_qt_executable(Aikari-UI-Settings WIN32
        entrypoint.cpp
        lifecycle.cpp
        lifecycle.h
        infrastructure/cliParse.cpp
        infrastructure/cliParse.h
        infrastructure/instances.cpp
        infrastructure/instances.h
        infrastructure/qmlFontInit.cpp
        infrastructure/qmlFontInit.h
        qml/qml.qrc
)

# --- Functions --- #
function(aikari_ui_settings_add_qml_module)
    set(oneValueArgs TARGET_NAME URI VERSION)
    set(multiValueArgs QML_FILES RESOURCES SOURCES)

    cmake_parse_arguments(AIKARIFN "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    message(STATUS "--- Aikari-UI-Settings Adding QML Module: ${AIKARIFN_URI} ---")
    message(STATUS "Target Name: ${AIKARIFN_TARGET_NAME}")
    message(STATUS "Version: ${AIKARIFN_VERSION}")
    qt_add_library(${AIKARIFN_TARGET_NAME} STATIC)
    string(REPLACE "." "/" AIKARIFN_TARGET_OUTPUT_DIRECTORY "${AIKARIFN_URI}")
    set(AIKARIFN_TARGET_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Aikari-UI-Settings/${AIKARIFN_TARGET_OUTPUT_DIRECTORY}")
    message(STATUS "Output DIR expected: ${AIKARIFN_TARGET_OUTPUT_DIRECTORY}")
    foreach (instance_path ${AIKARIFN_QML_FILES})
        cmake_path(GET instance_path FILENAME AIKARIFN_FILE_NAME)
        set_source_files_properties(${instance_path} PROPERTIES
                QT_RESOURCE_ALIAS ${AIKARIFN_FILE_NAME}
        )
    endforeach ()
    qt_add_qml_module(${AIKARIFN_TARGET_NAME}
            URI "${AIKARIFN_URI}"
            VERSION ${AIKARIFN_VERSION}
            QML_FILES
            ${AIKARIFN_QML_FILES}
            RESOURCES
            ${AIKARIFN_RESOURCES}
            SOURCES
            ${AIKARIFN_SOURCES}
            RESOURCE_PREFIX
            ${AIKARI_UI_SETTINGS_QRC_RESOURCE_PREFIX}
            OUTPUT_DIRECTORY "${AIKARIFN_TARGET_OUTPUT_DIRECTORY}"
    )

    target_include_directories(${AIKARIFN_TARGET_NAME}
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/includes
    )

    target_link_libraries(Aikari-UI-Settings PRIVATE ${AIKARIFN_TARGET_NAME})
    target_link_libraries(Aikari-UI-Settings PRIVATE ${AIKARIFN_TARGET_NAME}plugin)
endfunction()

set(AIKARI_UI_SETTINGS_QML_IMPORT_PATH "${CMAKE_BINARY_DIR}/Aikari-UI-Settings")
list(APPEND AIKARI_UI_SETTINGS_QML_IMPORT_PATH "${CMAKE_CURRENT_LIST_DIR}/../External/Mocked-Types")

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

# --- QML-specified --- #
set(QML_IMPORT_PATH "${AIKARI_UI_SETTINGS_QML_IMPORT_PATH}" CACHE PATH "Aikari UI Settings QML Path" FORCE)

# --- Registration --- #
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Reg QML modules
include(RegQMLModulesBackend)
include(RegQMLModulesFrontend)

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
