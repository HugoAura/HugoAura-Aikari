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
function(project_add_library target_name)
    add_library(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE Aikari-Global-Virtual)

    if (MSVC)
        set_target_properties(${target_name} PROPERTIES
                PDB_NAME ${target_name}
        )
    endif ()

    set_target_properties(${target_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/$<CONFIG>
            PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
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
    endif ()

    set_target_properties(${target_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/$<CONFIG>
            PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
    )
endfunction()

# --- Add-Sub --- #

add_subdirectory(Aikari-Shared)
add_subdirectory(Aikari-PLS)
add_subdirectory(Aikari-Launcher)

# --- Post Build CMDs --- #
# not using add_custom_command here because it will cause problems
# idk why

set(ARTIFACTS_DIR ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)

# >>> TASK COPY_PLS_RES
set(SOURCE_PLS_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Aikari-PLS/resources/static)
set(DESTINATION_PLS_RESOURCES_DIR ${ARTIFACTS_DIR}/resources/pls)
add_custom_target(Aikari-Virtual-CMD-CP-PLS-RES ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${SOURCE_PLS_RESOURCES_DIR}" "${DESTINATION_PLS_RESOURCES_DIR}"
        COMMENT "Copying PLS resources to output directory..."
)
add_dependencies(Aikari-Launcher Aikari-Virtual-CMD-CP-PLS-RES)
# <<< TASK COPY_PLS_RES

# >>> TASK COPY_SENTRY_CRASHPAD_HANDLER
find_program(SENTRY_HANDLER_PATH
        crashpad_handler
        HINTS ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/sentry-native
)
if (SENTRY_HANDLER_PATH)
    add_custom_target(Aikari-Virtual-CMD-CP-SENTRY-CRASHPAD-HANDLER ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SENTRY_HANDLER_PATH}"
            "${ARTIFACTS_DIR}/crashpad_handler.exe"
    )
    message(STATUS "Copied Sentry CrashPad Handler: ${SENTRY_HANDLER}")
else ()
    message(WARNING "Could not find Sentry CrashPad Handler executable, artifact may fail to start.")
endif ()
add_dependencies(Aikari-Launcher Aikari-Virtual-CMD-CP-SENTRY-CRASHPAD-HANDLER)
# <<< TASK COPY_SENTRY_CRASHPAD_HANDLER

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
# <<<

# --- Install --- #
install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/
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
