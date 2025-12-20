# module AikariComponents.Animations

# PageAnimAgent 1.0 PageAnimAgent.qml
# PageSwipeHorizontal 1.0 PageSwipeHorizontal.qml

aikari_ui_settings_add_qml_module(TARGET_NAME Aikari-UI-Settings-Components-Animations
        URI "AikariComponents.Animations"
        VERSION 1.0
        QML_FILES
        "${CMAKE_CURRENT_LIST_DIR}/PageAnimAgent.qml"
        "${CMAKE_CURRENT_LIST_DIR}/PageSwipeHorizontal.qml"
)
