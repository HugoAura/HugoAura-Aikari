import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import AikariConstants as AikariConstants

Button {
    id: actionBtnRoot
    height: parent.height
    width: parent.height * 1.25
    leftInset: 0
    rightInset: 0
    topInset: 0
    bottomInset: 0

    contentItem: Item {
        Image {
            id: actionBtnImg
            anchors.centerIn: parent
            visible: false
            mipmap: true
            width: 8
            height: 8
            fillMode: Image.PreserveAspectFit
            opacity: actionBtnRoot.enabled ? 1 : 0.5
        }

        MultiEffect {
            source: actionBtnImg
            anchors.fill: actionBtnImg
            colorization: 1.0
            colorizationColor: {
                if (AikariConstants.ThemeStyle.defaultStyle.isDark) {
                    return "white"
                } else {
                    if (btnBgColorMode === "red" && actionBtnRoot.hovered) {
                        return "white"
                    }
                    return "black"
                }
            }
            Behavior on colorizationColor {
                ColorAnimation {
                    duration: 150
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
    property alias imgSrc: actionBtnImg.source

    property string btnBgColorMode: "default" // default or red
    background: Rectangle {
        color: {
            if (btnBgColorMode === "default") {
                if (!actionBtnRoot.enabled) {
                    return AikariConstants.ThemeStyle.defaultStyle.isDark ?
                        Qt.rgba(255, 255, 255, 0.35) : Qt.rgba(0, 0, 0, 0.35);
                }
                if (actionBtnRoot.pressed) {
                    return AikariConstants.ThemeStyle.defaultStyle.isDark ?
                        Qt.rgba(255, 255, 255, 0.125) : Qt.rgba(0, 0, 0, 0.125);
                }
                if (actionBtnRoot.hovered) {
                    return AikariConstants.ThemeStyle.defaultStyle.isDark ?
                        Qt.rgba(255, 255, 255, 0.2) : Qt.rgba(0, 0, 0, 0.2);
                }
                return "transparent";
            } else if (btnBgColorMode === "red") {
                if (!actionBtnRoot.enabled) {
                    return AikariConstants.ThemeStyle.defaultStyle.isDark ?
                        Qt.rgba(255, 255, 255, 0.35) : Qt.rgba(0, 0, 0, 0.35);
                }
                if (actionBtnRoot.pressed) {
                    return "#D55243";
                }
                if (actionBtnRoot.hovered) {
                    return "#C42C1D";
                }
                return "transparent";
            }
        }

        Behavior on color {
            ColorAnimation {
                duration: 150
                easing.type: Easing.InOutQuad
            }
        }
    }
}
