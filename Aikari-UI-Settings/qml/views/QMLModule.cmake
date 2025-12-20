# module AikariViews

# SplashView 1.0 SplashView.qml
# HomeView 1.0 HomeView.qml

aikari_ui_settings_add_qml_module(TARGET_NAME Aikari-UI-Settings-Views
        URI "AikariViews"
        VERSION 1.0
        QML_FILES
        "${CMAKE_CURRENT_LIST_DIR}/SplashView.qml"
        "${CMAKE_CURRENT_LIST_DIR}/HomeView.qml"
)
