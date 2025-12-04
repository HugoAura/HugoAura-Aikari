pragma Singleton
import QtQuick

QtObject {
    id: themeStyleSingletonRoot

    property QtObject lightStyle: QtObject
    {
        id: _lightStyle
        readonly property color background: "#F8F8F7"
        readonly property color text: "#1C1C1C"
    }

    property QtObject darkStyle: QtObject
    {
        id: _darkStyle
        readonly property color background: "#1c1c1c"
        readonly property color text: "#FFFFFF"
    }

    property QtObject defaultStyle: QtObject
    {
        id: _defaultStyleAndSettings
        property string userSetThemeMode: "unset" // unset / dark / light
        readonly property bool isDark: userSetThemeMode !== "unset" ? (userSetThemeMode === "dark") : (Qt.styleHints.colorScheme === Qt.ColorScheme.Dark)
        readonly property color background: isDark ? themeStyleSingletonRoot.darkStyle.background : themeStyleSingletonRoot.lightStyle.background
        readonly property color textColor: isDark ? themeStyleSingletonRoot.darkStyle.text : themeStyleSingletonRoot.lightStyle.text

        readonly property int fontSizeNormal: 14;
        readonly property int fontSizeSm: 12;
        readonly property int fontSizeLg: 16;
        readonly property int fontSizeLgr: 18;
        readonly property int fontSizeXLg: 20;
    }
}
