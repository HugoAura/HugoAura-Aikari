import QtQuick
import AikariComponents.Shapes as AikariComponentsShapes

Rectangle {
    id: progressCircularRoot
    color: "transparent"
    readonly property real customScale: 1
    height: 80 * customScale
    width: 80 * customScale

    rotation: 0

    property bool isInitialAnimStarted: false
    property bool isLoopAnimRunning: false

    property color fillColor: "white"

    Repeater {
        model: 4

        delegate: AikariComponentsShapes.Diamond {
            id: perDiamond
            function getTranslatePair(withOffset = false, offset = 50) {
                const baselineVal = 15;
                const offsetVal = withOffset ? offset : 0;
                switch (index) {
                case 0:
                    return [0, -1 * (baselineVal + offsetVal) * progressCircularRoot.customScale];
                case 1:
                    return [(baselineVal + offsetVal) * progressCircularRoot.customScale, 0];
                case 2:
                    return [0, (baselineVal + offsetVal) * progressCircularRoot.customScale];
                case 3:
                    return [(-baselineVal - offsetVal) * progressCircularRoot.customScale, 0];
                }
            }
            anchors.centerIn: parent
            strokeColor: progressCircularRoot.fillColor
            fillColor: progressCircularRoot.fillColor
            strokeWidth: 1
            customScale: 0.825
            rotation: index % 2 === 0 ? 0 : 270
            visible: false
            opacity: 0

            transform: Translate {
                id: perDiamondTranslate
                x: 0
                y: 0
            }

            SequentialAnimation {
                running: progressCircularRoot.isInitialAnimStarted

                ScriptAction {
                    script: {
                        const translatePairOffset = perDiamond.getTranslatePair(true);
                        const translatePairNonOffset = perDiamond.getTranslatePair(false);
                        perDiamondTranslate.x = translatePairOffset[0];
                        perDiamondTranslate.y = translatePairOffset[1];
                        perDiamond.visible = true;
                        translateXAnim.to = translatePairNonOffset[0];
                        translateYAnim.to = translatePairNonOffset[1];
                    }
                }

                PauseAnimation {
                    duration: index * 100
                }

                ParallelAnimation {
                    NumberAnimation {
                        id: translateXAnim
                        target: perDiamondTranslate
                        property: "x"
                        duration: 350
                        easing.type: Easing.Bezier
                        easing.bezierCurve: [.03, .78, .59, .96, 1, 1]
                    }

                    NumberAnimation {
                        id: translateYAnim
                        target: perDiamondTranslate
                        property: "y"
                        to: perDiamond.getTranslatePair(false)[1]
                        duration: 350
                        easing.type: Easing.Bezier
                        easing.bezierCurve: [.03, .78, .59, .96, 1, 1]
                    }

                    NumberAnimation {
                        target: perDiamond
                        property: "opacity"
                        to: 1
                        duration: 350
                        easing.type: Easing.InQuad
                    }
                }

                PauseAnimation {
                    duration: 1000
                }

                ScriptAction {
                    script: {
                        if (index === 0)
                            progressCircularRoot.isLoopAnimRunning = true;
                    }
                }
            }

            SequentialAnimation {
                running: progressCircularRoot.isLoopAnimRunning

                ScriptAction {
                    script: {
                        const translatePairOut = perDiamond.getTranslatePair(true, 10);
                        const translatePairIn = perDiamond.getTranslatePair(false);
                        translateXOutAnim.to = translatePairOut[0];
                        translateYOutAnim.to = translatePairOut[1];
                        translateXInAnim.to = translatePairIn[0];
                        translateYInAnim.to = translatePairIn[1];
                    }
                }

                ParallelAnimation {
                    NumberAnimation {
                        id: translateXOutAnim
                        target: perDiamondTranslate
                        property: "x"
                        duration: 400
                        easing.type: Easing.Bezier
                        easing.bezierCurve: [.03, .78, .59, .96, 1, 1]
                    }

                    NumberAnimation {
                        id: translateYOutAnim
                        target: perDiamondTranslate
                        property: "y"
                        duration: 400
                        easing.type: Easing.Bezier
                        easing.bezierCurve: [.03, .78, .59, .96, 1, 1]
                    }
                }

                PauseAnimation {
                    duration: 1400
                }

                ParallelAnimation {
                    NumberAnimation {
                        id: translateXInAnim
                        target: perDiamondTranslate
                        property: "x"
                        duration: 400
                        easing.type: Easing.Bezier
                        easing.bezierCurve: [.58, .02, .6, .94, 1, 1]
                    }

                    NumberAnimation {
                        id: translateYInAnim
                        target: perDiamondTranslate
                        property: "y"
                        duration: 400
                        easing.type: Easing.Bezier
                        easing.bezierCurve: [.58, .02, .6, .94, 1, 1]
                    }
                }

                PauseAnimation {
                    duration: 1000
                }

                loops: Animation.Infinite
            }
        }
    }

    SequentialAnimation {
        running: progressCircularRoot.isLoopAnimRunning

        PauseAnimation {
            duration: 400 + 200
        }

        RotationAnimation {
            target: progressCircularRoot
            to: 90
            duration: 1400 - 200 - 400
            easing.type: Easing.Bezier
            easing.bezierCurve: [.58, .02, .6, .94, 1, 1]
        }

        PauseAnimation {
            duration: 400 + 400
        }

        ScriptAction {
            script: {
                progressCircularRoot.rotation = 0;
            }
        }

        PauseAnimation {
            duration: 1000
        }

        loops: Animation.Infinite
    }
}
