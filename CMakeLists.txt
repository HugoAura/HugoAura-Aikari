cmake_minimum_required(VERSION 3.20)

project(HugoAura-Aikari VERSION 0.0.1.3 LANGUAGES CXX)

add_library(Aikari-Global-Virtual INTERFACE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_compile_options(Aikari-Global-Virtual INTERFACE
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
        $<$<CXX_COMPILER_ID:Clang>:-finput-charset=UTF-8>
        $<$<CXX_COMPILER_ID:GNU>:-finput-charset=UTF-8>
)

target_compile_options(Aikari-Global-Virtual INTERFACE
        $<$<CXX_COMPILER_ID:MSVC>:/await /FS /W3 /Zc:wchar_t /bigobj>
)

target_compile_options(Aikari-Global-Virtual INTERFACE
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/sdl>
)

target_compile_options(Aikari-Global-Virtual INTERFACE
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/Zi>
)

target_link_options(Aikari-Global-Virtual INTERFACE
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/DEBUG /OPT:REF /OPT:ICF>
)

target_compile_definitions(Aikari-Global-Virtual INTERFACE
        _UNICODE
        UNICODE
)

target_compile_definitions(Aikari-Global-Virtual INTERFACE
        $<IF:$<CONFIG:Debug>,SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE,SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>
)

# --- Functions --- #
function(util_set_ilk_path target_name)
    set(TARGET_ILK_DIR ${CMAKE_BINARY_DIR}/artifacts/ilk/$ENV{CMAKE_BUILD_TYPE})
    if (MSVC)
        file(MAKE_DIRECTORY ${TARGET_ILK_DIR})
        target_link_options(${target_name} PRIVATE "/ILK:${TARGET_ILK_DIR}/${target_name}.ilk")
    endif ()
endfunction()

function(project_add_library target_name)
    add_library(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE Aikari-Global-Virtual)

    if (MSVC)
        set_target_properties(${target_name} PROPERTIES
                PDB_NAME ${target_name}
        )
        util_set_ilk_path(${target_name})
    endif ()

    set_target_properties(${target_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/artifacts/bin/$<CONFIG>
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/artifacts/lib/$<CONFIG>
            PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/artifacts/symbols/$<CONFIG>
    )

    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}/External)
endfunction()

function(project_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE Aikari-Global-Virtual)

    if (MSVC)
        set_target_properties(${target_name} PROPERTIES
                PDB_NAME ${target_name}
        )
        util_set_ilk_path(${target_name})
    endif ()

    set_target_properties(${target_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/artifacts/bin/$<CONFIG>
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/artifacts/lib/$<CONFIG>
            PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/artifacts/symbols/$<CONFIG>
    )
endfunction()

function(project_add_qt_executable target_name)
    qt_add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE Aikari-Global-Virtual)

    if (MSVC)
        set_target_properties(${target_name} PROPERTIES
                PDB_NAME ${target_name}
        )
        util_set_ilk_path(${target_name})
    endif ()

    set_target_properties(${target_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/artifacts/bin/$<CONFIG>
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/artifacts/lib/$<CONFIG>
            PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/artifacts/symbols/$<CONFIG>
    )
endfunction()

# --- Add-Sub --- #

add_subdirectory(External)

add_subdirectory(Aikari-Shared)
add_subdirectory(Aikari-UI-Settings)
add_subdirectory(Aikari-PLS)
add_subdirectory(Aikari-Launcher)

# --- Post Build CMDs --- #
# not using add_custom_command here because sub targets are not initialized at this time

set(ARTIFACTS_BIN_DIR ${CMAKE_BINARY_DIR}/artifacts/bin/$<CONFIG>)
set(VCPKG_INSTALLED_BASE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET})

# >>> TASK COPY_PLS_RES
set(SOURCE_PLS_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Aikari-PLS/resources/static)
set(DESTINATION_PLS_RESOURCES_DIR ${ARTIFACTS_BIN_DIR}/resources/pls)
add_custom_target(Aikari-Virtual-CMD-CP-PLS-RES ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${SOURCE_PLS_RESOURCES_DIR}" "${DESTINATION_PLS_RESOURCES_DIR}"
        COMMENT "Copying PLS resources to output directory..."
)
add_dependencies(Aikari-Launcher Aikari-Virtual-CMD-CP-PLS-RES)
# <<< TASK COPY_PLS_RES

# >>> TASK COPY_SENTRY_CRASHPAD_HANDLER
find_program(SENTRY_HANDLER_PATH
        crashpad_handler
        HINTS ${VCPKG_INSTALLED_BASE}/tools/sentry-native
)
if (SENTRY_HANDLER_PATH)
    add_custom_target(Aikari-Virtual-CMD-CP-SENTRY-CRASHPAD-HANDLER ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SENTRY_HANDLER_PATH}"
            "${ARTIFACTS_BIN_DIR}/crashpad_handler.exe"
    )
    message(STATUS "Found Sentry CrashPad Handler: ${SENTRY_HANDLER}")
else ()
    message(WARNING "Could not find Sentry CrashPad Handler executable, artifact may fail to start.")
endif ()
add_dependencies(Aikari-Launcher Aikari-Virtual-CMD-CP-SENTRY-CRASHPAD-HANDLER)
# <<< TASK COPY_SENTRY_CRASHPAD_HANDLER

# >>> TASK WIN_DEPLOY_QT_FOR_SETTINGS_UI
if (WIN32) # 但实际上一直都是 WIN32
    find_program(QT_WINDEPLOY_PATH windeployqt
            HINTS ${VCPKG_INSTALLED_BASE}/tools/Qt6/bin
    )
    if (QT_WINDEPLOY_PATH)
        message(STATUS "Found windepolyqt.exe at ${QT_WINDEPLOY_PATH} ${Qt6_DIR}")
        add_custom_target(Aikari-Virtual-CMD-QT-RUN-WINDEPLOY ALL
                COMMAND ${QT_WINDEPLOY_PATH}
                --dir $<TARGET_FILE_DIR:Aikari-UI-Settings>
                --qmldir ${CMAKE_CURRENT_SOURCE_DIR}/Aikari-UI-Settings/resources

                $<$<CONFIG:Debug>:--qtpaths>

                $<$<CONFIG:Debug>:${VCPKG_INSTALLED_BASE}/tools/Qt6/bin/qtpaths.debug.bat>

                $<$<CONFIG:Debug>:--debug>
                $<$<CONFIG:RelWithDebInfo>:--release>
                $<$<CONFIG:Release>:--release>

                --no-compiler-runtime
                --no-translations

                $<TARGET_FILE:Aikari-UI-Settings>
        )
    else ()
        message(WARNING "Could not find windeployqt.exe, copy plugin DLLs manually may required.")
    endif ()
endif ()
add_dependencies(Aikari-Virtual-CMD-QT-RUN-WINDEPLOY Aikari-UI-Settings)
# <<< TASK WIN_DEPLOY_QT_FOR_SETTINGS_UI

# >>> TASK SIGN_AIKARI_LAUNCHER_EXE
if (SIGN_LAUNCHER)
    set(SIGN_TIMESTAMP_URL "http://timestamp.digicert.com")
    find_program(SIGNTOOL_EXE "signtool.exe" PATHS)
    if (SIGNTOOL_EXE AND EXISTS $ENV{PFX_FILE})
        message(STATUS "Found signtool: ${SIGNTOOL_EXE}")
        add_custom_target(Aikari-Virtual-CMD-SIGN-LAUNCHER ALL
                COMMAND ${SIGNTOOL_EXE} sign
                /f "$ENV{PFX_FILE}"
                /p "$ENV{PFX_PASS}"
                /fd SHA256
                /tr "${SIGN_TIMESTAMP_URL}"
                /td SHA256
                "$<TARGET_FILE:Aikari-Launcher>"
                COMMENT "Signing launcher EXE with ${SIGN_PFX_FILE}..."
        )
        add_dependencies(Aikari-Virtual-CMD-SIGN-LAUNCHER Aikari-Launcher)
    else ()
        message(WARNING "Signtool not found or PFX missing. Skipping signature.")
    endif ()
endif ()
# <<< TASK SIGN_AIKARI_LAUNCHER_EXE

# --- Install --- #
install(DIRECTORY ${CMAKE_BINARY_DIR}/artifacts/bin/$<CONFIG>/
        DESTINATION .
        COMPONENT Core
        PATTERN "*.pdb" EXCLUDE
        PATTERN "*.lib" EXCLUDE
        PATTERN "*.ilk" EXCLUDE
)

set(CPACK_PACKAGE_NAME "HugoAura Aikari")
set(CPACK_PACKAGE_VERSION "0.0.1.3")
set(CPACK_PACKAGE_VENDOR "HugoAura Devs")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Unified UX tweak tool for Seewo Hugo")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/assets/Aikari_Installer_License_File.rtf")
# set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/Aikari_2D_Logo_ICO_Control_Panel.ico")

set(CPACK_COMPONENTS_ALL Core)
set(CPACK_COMPONENT_CORE_DISPLAY_NAME "主要组件")
set(CPACK_COMPONENT_CORE_DESCRIPTION "应用的主要组件")
set(CPACK_COMPONENT_CORE_REQUIRED TRUE)

set(CPACK_GENERATOR "InnoSetup")
set(CPACK_INNOSETUP_LANGUAGE "english")
set(CPACK_INNOSETUP_ARCHITECTURE "${USER_CPACK_TARGET_PROCESSOR}")
set(CPACK_INNOSETUP_SETUP_AppId "HugoAura.Aikari")
set(CPACK_INNOSETUP_SETUP_AppPublisher "HugoAura Devs")
set(CPACK_INNOSETUP_SETUP_SetupIconFile "${CMAKE_CURRENT_SOURCE_DIR}/assets/Aikari_2D_Logo_ICO_Installer.ico")
set(CPACK_INNOSETUP_SETUP_MinVersion "6.1sp2")
set(CPACK_INNOSETUP_SETUP_DefaultDirName "{autopf}\\HugoAura\\Aikari")
set(CPACK_INNOSETUP_SETUP_AllowNoIcons ON)
set(CPACK_INNOSETUP_USE_MODERN_WIZARD ON)
set(CPACK_INNOSETUP_USE_CMAKE_BOOL_FORMAT ON)
set(CPACK_INNOSETUP_EXTRA_SCRIPTS
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/InnoSetupPreInstCmd.iss"
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/InnoSetupUninstCmd.iss"
)

set(CPACK_PACKAGE_FILE_NAME "${CPACK_INNOSETUP_SETUP_AppId}-Setup-${CPACK_PACKAGE_VERSION}-Arch-${CPACK_INNOSETUP_ARCHITECTURE}")

include(CPack)
