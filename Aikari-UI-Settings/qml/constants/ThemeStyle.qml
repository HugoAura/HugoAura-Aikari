pragma Singleton
import QtQuick
import AikariUI.Settings.Backend

QtObject {
    id: themeStyleSingletonRoot

    property QtObject lightStyle: QtObject
    {
        id: _lightStyle
        readonly property color background: "#F8F8F7"
        readonly property color text: "#1C1C1C"
        readonly property color textOpacity: Qt.rgba(0, 0, 0, 0.5)
        readonly property color textMoreOpacity: Qt.rgba(0, 0, 0, 0.25)
    }

    property QtObject darkStyle: QtObject
    {
        id: _darkStyle
        readonly property color background: "#1c1c1c"
        readonly property color text: "#FFFFFF"
        readonly property color textOpacity: Qt.rgba(255, 255, 255, 0.5)
        readonly property color textMoreOpacity: Qt.rgba(255, 255, 255, 0.25)
    }

    property QtObject defaultStyle: QtObject
    {
        id: _defaultStyleAndSettings
        property string userSetThemeMode: "unset" // unset / dark / light
        readonly property bool isDark: (() => {
            if (userSetThemeMode !== "unset") {
                return userSetThemeMode === "dark";
            }
            if (BridgesImpl_Theme.forceThemeMode !== BridgeTypes_Theme_ForceThemeMode.UNSET) {
                return BridgesImpl_Theme.forceThemeMode === BridgeTypes_Theme_ForceThemeMode.DARK;
            }
            return Qt.styleHints.colorScheme === Qt.ColorScheme.Dark;
        })()
        readonly property color background: isDark ? themeStyleSingletonRoot.darkStyle.background : themeStyleSingletonRoot.lightStyle.background
        readonly property color textColor: isDark ? themeStyleSingletonRoot.darkStyle.text : themeStyleSingletonRoot.lightStyle.text
        readonly property color textColorOpacity: isDark ? themeStyleSingletonRoot.darkStyle.textOpacity : themeStyleSingletonRoot.lightStyle.textOpacity
        readonly property color textColorMoreOpacity: isDark ? themeStyleSingletonRoot.darkStyle.textMoreOpacity : themeStyleSingletonRoot.lightStyle.textMoreOpacity

        readonly property int fontSizeNormal: 14;
        readonly property int fontSizeSm: 12;
        readonly property int fontSizeLg: 16;
        readonly property int fontSizeLgr: 18;
        readonly property int fontSizeXLg: 20;
    }

    property QtObject themeStates: QtObject
    {
        id: _themeStates
        property bool overlayComponentsColorReversed: false
    }
}
