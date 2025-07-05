cmake_minimum_required(VERSION 3.20)

project(HugoAura-Aikari VERSION 0.0.1 LANGUAGES CXX)

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
    $<$<CXX_COMPILER_ID:MSVC>:/await /FS /W3 /Zc:wchar_t>
)

target_compile_options(Aikari-Global-Virtual INTERFACE
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/sdl>
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

    if(MSVC)
        set_target_properties(${target_name} PROPERTIES
            PDB_NAME ${target_name}
        )
    endif()

    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/$<CONFIG>
        PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
    )
endfunction()

function(project_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE Aikari-Global-Virtual)

    if(MSVC)
        set_target_properties(${target_name} PROPERTIES
            PDB_NAME ${target_name}
        )
    endif()

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
