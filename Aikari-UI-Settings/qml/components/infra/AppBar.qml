import QtQuick
import QWindowKit as QWK
import AikariConstants as AikariConstants

Rectangle {
    id: mainAppBar
    width: parent.width
    height: 32
    visible: true
    readonly property bool isAppBarDark: AikariConstants.ThemeStyle.themeStates.overlayComponentsColorReversed ?
        !AikariConstants.ThemeStyle.defaultStyle.isDark :
        AikariConstants.ThemeStyle.defaultStyle.isDark
    color: "transparent"

    Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 10

        text: mainWindowRoot.title
        font.pixelSize: AikariConstants.ThemeStyle.defaultStyle.fontSizeSm
        color: mainAppBar.isAppBarDark ?
            AikariConstants.ThemeStyle.darkStyle.text :
            AikariConstants.ThemeStyle.lightStyle.text

        Behavior on color {
            ColorAnimation {
                duration: 150
                easing.type: Easing.InOutQuad
            }
        }
    }

    Row {
        anchors.right: parent.right
        anchors.top: parent.top

        height: parent.height
        visible: true

        AppBarActionBtn {
            id: minimizeBtn
            imgSrc: "qrc:/assets/img/appbar/minimize.svg"
            btnBgColorMode: "default"
            isDark: mainAppBar.isAppBarDark
            onClicked: mainWindowRoot.showMinimized()
            Component.onCompleted: qwkWindowAgent.setSystemButton(
                QWK.WindowAgent.Minimize, minimizeBtn
            )
        }

        AppBarActionBtn {
            id: showModeSwitchBtn
            imgSrc: mainWindowRoot.visibility === Window.Maximized ?
                "qrc:/assets/img/appbar/restore.svg" :
                "qrc:/assets/img/appbar/maximize.svg"
            btnBgColorMode: "default"
            isDark: mainAppBar.isAppBarDark
            onClicked: mainWindowRoot.visibility === Window.Maximized ?
                mainWindowRoot.showNormal() :
                mainWindowRoot.showMaximized()
            Component.onCompleted: qwkWindowAgent.setSystemButton(
                QWK.WindowAgent.Maximize, showModeSwitchBtn
            )
        }

        AppBarActionBtn {
            id: closeWindowBtn
            imgSrc: "qrc:/assets/img/appbar/close.svg"
            btnBgColorMode: "red"
            isDark: mainAppBar.isAppBarDark
            onClicked: mainWindowRoot.close()
            Component.onCompleted: qwkWindowAgent.setSystemButton(
                QWK.WindowAgent.Close, closeWindowBtn
            )
        }
    }
}
