set(AIKARI_UI_SETTINGS_BACKEND_SOURCES
        bridges/virtual/ILifecycleBridge.h
        includes/Aikari-UI-Settings-Private/types/bridges/lifecycle.h
)

if (AIKARI_UI_SETTINGS_USE_MOCK_CLASS OR GLOBAL_USE_MOCK_CLASS)
    message(STATUS "Using mock bridges impl for Aikari-UI-Settings")
    list(APPEND AIKARI_UI_SETTINGS_BACKEND_SOURCES
            bridges/impl/LifecycleBridgeMock.cpp
            bridges/impl/LifecycleBridgeMock.h
    )
else ()
    message(STATUS "Using actual bridges impl for Aikari-UI-Settings")
endif ()

qt_add_qml_module(Aikari-UI-Settings
        URI "AikariUI.Settings.Backend"
        VERSION 1.0
        SOURCES
        ${AIKARI_UI_SETTINGS_BACKEND_SOURCES}
)
