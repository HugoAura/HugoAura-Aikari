set(AIKARI_UI_SETTINGS_FRONTEND_MODULES
        qml/components/animations/QMLModule.cmake
        qml/components/atom/QMLModule.cmake
        qml/components/infra/QMLModule.cmake
        qml/components/shapes/QMLModule.cmake
        qml/constants/QMLModule.cmake
        qml/routes/QMLModule.cmake
        qml/views/QMLModule.cmake
        qml/windows/QMLModule.cmake
)

foreach (module_path ${AIKARI_UI_SETTINGS_FRONTEND_MODULES})
    include("${CMAKE_CURRENT_SOURCE_DIR}/${module_path}")
endforeach ()
