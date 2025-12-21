import QtQuick
import AikariConstants as AikariConstants

Rectangle {
    id: mountedPageSwipeHorizontal
    width: parent.width
    height: 0
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    color: AikariConstants.ThemeStyle.defaultStyle.textColor
    z: 100

    property int animDurationMs: 300
    property int animFreezeMs: 500

    function start({
                   duration = 300,
                   freeze = 500
}) { // Powered by qmlformat, i have no idea 😓
        visible = true;
        animDurationMs = duration;
        animFreezeMs = freeze;
        startAnim.running = true;
    }

    function reset() {
        startAnim.running = false;
        anchors.bottom = parent.bottom;
        height = 0;
        visible = false;
    }

    SequentialAnimation {
        id: startAnim
        running: false

        ParallelAnimation {
            NumberAnimation {
                target: mountedPageSwipeHorizontal
                property: "height"
                to: parent.height
                duration: animDurationMs
                easing.type: Easing.Bezier
                easing.bezierCurve: [0, 0.83, 0.53, 0.99, 1, 1]
            }

            ParallelAnimation {
                PauseAnimation {
                    duration: mountedPageSwipeHorizontal.animDurationMs - 150
                }

                ScriptAction {
                    script: {
                        AikariConstants.ThemeStyle.themeStates.overlayComponentsColorReversed = true;
                    }
                }
            }
        }

        PauseAnimation {
            duration: animFreezeMs
        }

        ScriptAction {
            script: {
                anchors.bottom = undefined;
                anchors.top = mountedPageSwipeHorizontal.parent.top;
                mountedPageSwipeHorizontal.parent.onAnimCompleted();
                AikariConstants.ThemeStyle.themeStates.overlayComponentsColorReversed = false;
            }
        }

        NumberAnimation {
            target: mountedPageSwipeHorizontal
            property: "height"
            to: 0
            duration: animDurationMs
            easing.type: Easing.Bezier
            easing.bezierCurve: [0, 0.83, 0.53, 0.99, 1, 1]
        }
    }
}
