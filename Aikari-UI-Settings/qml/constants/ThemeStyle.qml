pragma Singleton
import QtQuick
import AikariUI.Settings.Backend

QtObject {
    id: themeStyleSingletonRoot

    property var lightStyle: QtObject {
        id: _lightStyle

        readonly property color background: "#F8F8F7"
        readonly property color text: "#1C1C1C"
        readonly property color textMoreOpacity: Qt.rgba(0, 0, 0, 0.25)
        readonly property color textOpacity: Qt.rgba(0, 0, 0, 0.5)
    }

    property var darkStyle: QtObject {
        id: _darkStyle

        readonly property color background: "#1c1c1c"
        readonly property color text: "#FFFFFF"
        readonly property color textMoreOpacity: Qt.rgba(255, 255, 255, 0.25)
        readonly property color textOpacity: Qt.rgba(255, 255, 255, 0.5)
    }

    property var cubicBeziers: QtObject {
        id: _cubicBezierConstants

        readonly property var snapTo: [0, 0.83, 0.53, 0.99, 1, 1]
        readonly property var ease: [0.25, 0.1, 0.25, 1, 1, 1]
        readonly property var easeIn: [0.42, 0, 1, 1, 1, 1]
    }

    property var defaultStyle: QtObject {
        id: _defaultStyleAndSettings

        readonly property color background: isDark ? themeStyleSingletonRoot.darkStyle.background :
                                                     themeStyleSingletonRoot.lightStyle.background
        readonly property int fontSizeLg: 16
        readonly property int fontSizeLgr: 18
        readonly property int fontSizeNormal: 14
        readonly property int fontSizeSm: 12
        readonly property int fontSizeXLg: 20
        readonly property bool isDark: (() => {
                                            if (userSetThemeMode !== "unset") {
                                                return userSetThemeMode === "dark";
                                            }
                                            if (BridgesImpl_Theme.forceThemeMode
                                                !== BridgeTypes_Theme_ForceThemeMode.UNSET) {
                                                return BridgesImpl_Theme.forceThemeMode
                                                === BridgeTypes_Theme_ForceThemeMode.DARK;
                                            }
                                            return Qt.styleHints.colorScheme === Qt.ColorScheme.Dark;
                                        })()
        readonly property color textColor: isDark ? themeStyleSingletonRoot.darkStyle.text :
                                                    themeStyleSingletonRoot.lightStyle.text
        readonly property color textColorMoreOpacity: isDark ? themeStyleSingletonRoot.darkStyle.textMoreOpacity :
                                                               themeStyleSingletonRoot.lightStyle.textMoreOpacity
        readonly property color textColorOpacity: isDark ? themeStyleSingletonRoot.darkStyle.textOpacity :
                                                           themeStyleSingletonRoot.lightStyle.textOpacity
        property string userSetThemeMode: "unset" // unset / dark / light
    }

    property var environmentStates: QtObject {
        id: _environmentStates

        property var rootWindow
        property bool isScreenSm: {
            if (rootWindow) {
                return rootWindow.width < 768;
            } else {
                return false;
            }
        }
    }

    property var themeStates: QtObject {
        id: _themeStates

        /* Global Style */
        property bool overlayComponentsColorReversed: false
        /* Main Window Appbar */
        property bool mainAppBarShowSideBarSwitchBtn: false
        /* Main Window SideBar */
        property bool isMainWindowSideBarShow: true
        property var isMainWindowSideBarShowConnection: Connections {
            target: _environmentStates

            function onIsScreenSmChanged() {
                _themeStates.isMainWindowSideBarShow = !_environmentStates.isScreenSm;
            }
        }
    }
}
