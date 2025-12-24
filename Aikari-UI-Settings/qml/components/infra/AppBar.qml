import QtQuick
import QWindowKit as QWK
import AikariConstants as AikariConstants
import AikariComponents.Animations as AikariComponentsAnimations

Rectangle {
    id: mainAppBar

    readonly property bool isAppBarDark: AikariConstants.ThemeStyle.themeStates.overlayComponentsColorReversed ?
                                             !AikariConstants.ThemeStyle.defaultStyle.isDark :
                                             AikariConstants.ThemeStyle.defaultStyle.isDark
    required property var parentWindow

    color: "transparent"
    height: 32
    visible: true
    width: parent.width

    Row {
        id: mainAppBarLeftActionBtns
        anchors.left: parent.left
        anchors.top: parent.top
        height: parent.height
        visible: true
        Component.onCompleted: qwkWindowAgent.setHitTestVisible(mainAppBarLeftActionBtns, true)

        AikariComponentsAnimations.ComponentSnapMask {
            id: sideBarToggleContainer
            maskColor: AikariConstants.ThemeStyle.defaultStyle.textColor
            maskZIndex: 3
            expectedLength: sideBarToggle.width

            height: mainAppBarLeftActionBtns.height

            isShow: AikariConstants.ThemeStyle.themeStates.mainAppBarShowSideBarSwitchBtn

            AppBarActionBtn {
                id: sideBarToggle

                btnBgColorMode: "default"
                imgSrc: "qrc:/assets/img/appbar/menu.svg"
                isDark: mainAppBar.isAppBarDark
                iconWidthOverride: 10

                onClicked: AikariConstants.ThemeStyle.themeStates.isMainWindowSideBarShow =
                           !AikariConstants.ThemeStyle.themeStates.isMainWindowSideBarShow
            }
        }
    }
    Text {
        anchors.left: mainAppBarLeftActionBtns.right
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        color: mainAppBar.isAppBarDark ? AikariConstants.ThemeStyle.darkStyle.text :
                                         AikariConstants.ThemeStyle.lightStyle.text
        font.pixelSize: AikariConstants.ThemeStyle.defaultStyle.fontSizeSm
        text: mainAppBar.parentWindow.title

        Behavior on color {
            ColorAnimation {
                duration: 150
                easing.type: Easing.InOutQuad
            }
        }
    }
    Row {
        id: mainAppBarRightActionBtns
        anchors.right: parent.right
        anchors.top: parent.top
        height: parent.height
        visible: true
        Component.onCompleted: qwkWindowAgent.setHitTestVisible(mainAppBarRightActionBtns, true)
        /* [!] Not using qwkAgent.setSystemButton, because
         * it will likely cause problems on touch screens */

        AppBarActionBtn {
            id: minimizeBtn

            btnBgColorMode: "default"
            imgSrc: "qrc:/assets/img/appbar/minimize.svg"
            isDark: mainAppBar.isAppBarDark

            // Component.onCompleted: qwkWindowAgent.setSystemButton(QWK.WindowAgent.Minimize, minimizeBtn)
            onClicked: mainAppBar.parentWindow.showMinimized()
        }
        AppBarActionBtn {
            id: showModeSwitchBtn

            btnBgColorMode: "default"
            imgSrc: mainAppBar.parentWindow.visibility === Window.Maximized ? "qrc:/assets/img/appbar/restore.svg" :
                                                                              "qrc:/assets/img/appbar/maximize.svg"
            isDark: mainAppBar.isAppBarDark

            // Component.onCompleted: qwkWindowAgent.setSystemButton(QWK.WindowAgent.Maximize, showModeSwitchBtn)
            onClicked: mainAppBar.parentWindow.visibility === Window.Maximized ? mainAppBar.parentWindow.showNormal() :
                                                                                 mainAppBar.parentWindow.showMaximized()
        }
        AppBarActionBtn {
            id: closeWindowBtn

            btnBgColorMode: "red"
            imgSrc: "qrc:/assets/img/appbar/close.svg"
            isDark: mainAppBar.isAppBarDark

            // Component.onCompleted: qwkWindowAgent.setSystemButton(QWK.WindowAgent.Close, closeWindowBtn)
            onClicked: mainAppBar.parentWindow.close()
        }
    }
}
