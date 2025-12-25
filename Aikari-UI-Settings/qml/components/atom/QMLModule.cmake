# module AikariComponents.Atom

# ProgressCircular 1.0 ProgressCircular.qml
# ProgressBar 1.0 ProgressBar.qml

aikari_ui_settings_add_qml_module(TARGET_NAME Aikari-UI-Settings-Components-Atom
        URI "AikariComponents.Atom"
        VERSION 1.0
        QML_FILES
        "${CMAKE_CURRENT_LIST_DIR}/ProgressCircular.qml"
        "${CMAKE_CURRENT_LIST_DIR}/ProgressBar.qml"
        "${CMAKE_CURRENT_LIST_DIR}/SideBar.qml"
        "${CMAKE_CURRENT_LIST_DIR}/SideBarCommonItem.qml"
        "${CMAKE_CURRENT_LIST_DIR}/SideBarGroupContainer.qml"
        "${CMAKE_CURRENT_LIST_DIR}/SideBarGroupItem.qml"
)
