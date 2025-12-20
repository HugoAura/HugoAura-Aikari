# module AikariConstants

# singleton ThemeStyle 1.0 ThemeStyle.qml
# singleton FontLoaders 1.0 FontLoaders.qml

set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/ThemeStyle.qml" PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)
set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/FontLoaders.qml" PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

aikari_ui_settings_add_qml_module(TARGET_NAME Aikari-UI-Settings-Constants
        URI "AikariConstants"
        VERSION 1.0
        QML_FILES
        "${CMAKE_CURRENT_LIST_DIR}/ThemeStyle.qml"
        "${CMAKE_CURRENT_LIST_DIR}/FontLoaders.qml"
)
