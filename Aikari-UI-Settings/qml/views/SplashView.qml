import QtQuick
import QtQuick.Effects
import AikariConstants as AikariConstants
import AikariComponents.Atom as AikariComponentsAtom
import AikariUI.Settings.Backend

Rectangle {
    id: splashViewRoot
    color: "transparent"
    height: parent.height
    width: parent.width
    anchors {
        top: parent.top
        left: parent.left
    }

    Timer {
        id: startEstablishBackendConnectionTimer
        interval: 2000
        repeat: false
        running: true

        onTriggered: {
            BridgesImpl_Lifecycle.establishBackendConnection();
        }
    }

    Connections {
        target: BridgesImpl_Lifecycle

        function onBackendConnStatusChanged() {
            const val = BridgesImpl_Lifecycle.backendConnStatus;
            if (!val) return;
            if (val === BridgeTypes_Lifecycle_BackendConnectionStatus.CONNECTED) {
                console.log("[QML] done");
            } else if (val === BridgeTypes_Lifecycle_BackendConnectionStatus.FAILED) {

            }
        }
    }

    Item {
        id: logoRowContainer
        readonly property real logoScale: 0.675
        width: 909 * logoScale
        height: 153 * logoScale
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        scale: 1
        opacity: 1

        transform: Translate {
            id: logoRowContainerTranslate
            y: 0
        }

        Repeater {
            id: logoRowRepeater
            model: ["A_1", "I_1", "K", "A_2", "R", "I_2"]
            readonly property var fromLeft: [0, 119, 224, 424, 561, 729]
            property bool isAnimSeqRunning: false
            Component.onCompleted: {
                logoRowRepeater.isAnimSeqRunning = true
            }

            delegate: Item {
                required property string modelData
                required property int index

                readonly property real targetTranslateX: logoRowRepeater.fromLeft[index] * logoRowContainer.logoScale

                id: perLogoItem
                width: 180 * logoRowContainer.logoScale
                height: 153 * logoRowContainer.logoScale
                anchors.left: logoRowContainer.left
                visible: false
                opacity: 0

                transform: Translate {
                    id: perLogoItemTranslate
                    x: 0
                }

                Image {
                    id: perLogoCharImg
                    anchors.fill: parent
                    source: `qrc:/assets/img/views/splashScreen/${modelData}.png`
                    mipmap: true
                    fillMode: Image.PreserveAspectFit
                    visible: false
                }

                MultiEffect {
                    source: perLogoCharImg
                    anchors.fill: perLogoCharImg
                    visible: true
                    colorization: 1.0
                    colorizationColor: {
                        return AikariConstants.ThemeStyle.defaultStyle.isDark ? "white" : "black";
                    }
                }

                SequentialAnimation {
                    running: logoRowRepeater.isAnimSeqRunning

                    ScriptAction {
                        script: {
                            perLogoItemTranslate.x = perLogoItem.targetTranslateX + 500
                            perLogoItem.visible = true
                        }
                    }

                    PauseAnimation {
                        duration: index * 150
                    }

                    ParallelAnimation {
                        NumberAnimation {
                            target: perLogoItem
                            property: "opacity"
                            to: 1
                            duration: 750
                            easing.type: Easing.InOutQuad
                        }

                        NumberAnimation {
                            target: perLogoItemTranslate
                            property: "x"
                            to: perLogoItem.targetTranslateX
                            duration: 750
                            easing.type: Easing.Bezier
                            easing.bezierCurve: [0, 0.83, 0.53, 0.99, 1, 1]
                        }
                    }

                    onFinished: {
                        if (index === logoRowRepeater.count - 1) {
                            loadingSeqAnim.start()
                        }
                    }
                }
            }
        }
    }

    Text {
        id: loadingText
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: logoRowContainer.bottom
        anchors.topMargin: 40
        text: BridgesImpl_Lifecycle.backendConnStatistics.curTaskText
        color: AikariConstants.ThemeStyle.defaultStyle.textColor
        visible: false
        opacity: 0
        /* font.family: AikariConstants.FontLoaders.sairaLightLoader.name */
        font.pixelSize: AikariConstants.ThemeStyle.defaultStyle.fontSizeLgr
    }

    AikariComponentsAtom.ProgressBar {
        id: progressBar
        width: logoRowContainer.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: loadingText.bottom
        anchors.topMargin: 30
        fillColor: AikariConstants.ThemeStyle.defaultStyle.textColor
        backgroundColor: AikariConstants.ThemeStyle.defaultStyle.isDark ?
            Qt.rgba(255, 255, 255, 0.3) : Qt.rgba(0, 0, 0, 0.3)
        opacity: 0

        percent: BridgesImpl_Lifecycle.backendConnStatistics.connectionProgressPercent
    }

    Text {
        id: backlogText
        anchors.left: splashViewRoot.left
        anchors.bottom: splashViewRoot.bottom
        anchors.leftMargin: 10
        anchors.bottomMargin: 10
        color: AikariConstants.ThemeStyle.defaultStyle.textColorMoreOpacity
        font.pixelSize: AikariConstants.ThemeStyle.defaultStyle.fontSizeSm
        font.family: AikariConstants.FontLoaders.consolasLoader.name
        text: `${BridgesImpl_Lifecycle.backendConnStatistics.backlog.join("<br />")}`
    }

    SequentialAnimation {
        id: loadingSeqAnim

        PauseAnimation {
            duration: 300
        }

        ParallelAnimation {
            ScriptAction {
                script: {
                    loadingText.visible = true
                }
            }

            NumberAnimation {
                target: logoRowContainer
                property: "scale"
                to: 0.715
                duration: 500
                easing.type: Easing.Bezier
                easing.bezierCurve: [.13, .67, .65, .99, 1, 1]
            }

            NumberAnimation {
                target: logoRowContainerTranslate
                property: "y"
                to: -50
                duration: 500
                easing.type: Easing.Bezier
                easing.bezierCurve: [.13, .67, .65, .99, 1, 1]
            }

            NumberAnimation {
                target: loadingText
                property: "opacity"
                to: 1
                duration: 500
            }

            NumberAnimation {
                target: progressBar
                property: "opacity"
                to: 1
                duration: 500
            }

            NumberAnimation {
                target: loadingText
                property: "anchors.topMargin"
                to: -20
                duration: 500
                easing.type: Easing.Bezier
                easing.bezierCurve: [.13, .67, .65, .99, 1, 1]
            }

            SequentialAnimation {
                PauseAnimation {
                    duration: 350
                }

                SequentialAnimation {
                    NumberAnimation {
                        target: logoRowContainer
                        property: "opacity"
                        to: 0.625
                        duration: 100
                    }

                    NumberAnimation {
                        target: logoRowContainer
                        property: "opacity"
                        to: 1
                        duration: 100
                    }

                    loops: 2
                }
            }
        }
    }
}
