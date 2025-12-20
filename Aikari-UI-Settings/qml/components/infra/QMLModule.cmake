# module AikariComponents.Infra

# AppBar 1.0 AppBar.qml
# internal AppBarActionBtn 1.0 AppBarActionBtn.qml

set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/AppBarActionBtn.qml" PROPERTIES
        QT_QML_INTERNAL_TYPE TRUE
)

aikari_ui_settings_add_qml_module(TARGET_NAME Aikari-UI-Settings-Components-Infra
        URI "AikariComponents.Infra"
        VERSION 1.0
        QML_FILES
        "${CMAKE_CURRENT_LIST_DIR}/AppBar.qml"
        "${CMAKE_CURRENT_LIST_DIR}/AppBarActionBtn.qml"
)
