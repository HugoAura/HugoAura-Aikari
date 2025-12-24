# module AikariComponents.Shapes

# Diamond 1.0 Diamond.qml

aikari_ui_settings_add_qml_module(TARGET_NAME Aikari-UI-Settings-Components-Shapes
        URI "AikariComponents.Shapes"
        VERSION 1.0
        QML_FILES
        "${CMAKE_CURRENT_LIST_DIR}/Diamond.qml"
        "${CMAKE_CURRENT_LIST_DIR}/MaskClippedRectangle.qml"
)
