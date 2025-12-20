set(AIKARI_UI_SETTINGS_BACKEND_SOURCES
        bridges/virtual/ILifecycleBridge.h
        bridges/virtual/IThemeBridge.h
        bridges/impl/LifecycleBridge.h
        includes/Aikari-UI-Settings-Private/types/bridges/lifecycle.h
        includes/Aikari-UI-Settings-Private/types/bridges/theme.h
)

list(APPEND AIKARI_UI_SETTINGS_BACKEND_SOURCES
        bridges/impl/ThemeBridge.cpp
        bridges/impl/ThemeBridge.h
) # Shared

if (AIKARI_UI_SETTINGS_USE_MOCK_CLASS OR GLOBAL_USE_MOCK_CLASS)
    message(STATUS "Using mock bridges impl for Aikari-UI-Settings")
    list(APPEND AIKARI_UI_SETTINGS_BACKEND_SOURCES
            bridges/impl/LifecycleBridgeMock.cpp
    )
    target_compile_definitions(Aikari-UI-Settings PRIVATE
            AIKARI_UI_SETTINGS_USE_MOCK_CLASS
    )
else ()
    message(STATUS "Using actual bridges impl for Aikari-UI-Settings")
    list(APPEND AIKARI_UI_SETTINGS_BACKEND_SOURCES
            bridges/impl/LifecycleBridgeImpl.cpp
    )
endif ()

qt_add_qml_module(Aikari-UI-Settings
        URI "AikariUI.Settings.Backend"
        VERSION 1.0
        SOURCES
        ${AIKARI_UI_SETTINGS_BACKEND_SOURCES}
)
