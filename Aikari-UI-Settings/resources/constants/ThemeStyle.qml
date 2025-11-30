pragma Singleton
import QtQuick

QtObject {
    id: root

    property QtObject lightStyle: QtObject
    {
        id: _lightStyle
        readonly property color background: "#F8F8F7"
    }

    property QtObject darkStyle: QtObject
    {
        id: _darkStyle
        readonly property color background: "#1c1c1c"
    }

    property QtObject defaultStyle: QtObject
    {
        id: _defaultStyleAndSettings
        property string userSetThemeMode: "unset" // unset / dark / light
        readonly property bool isDark: userSetThemeMode !== "unset" ? (userSetThemeMode === "dark") : (Qt.styleHints.colorScheme === Qt.ColorScheme.Dark)
        readonly property color background: isDark ? root.darkStyle.background : root.lightStyle.background
    }
}