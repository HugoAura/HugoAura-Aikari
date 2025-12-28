import QtQuick
import QtQuick.Effects
import AikariConstants as AikariConstants

Item {
    id: elementCursorHorizontalRoot
    property real marginHorizontal: 2.5 // <<< SET (Optional)
    property real marginVertical: 0 // <<< SET (Optional)

    readonly property real residentHeight: 0
    readonly property real residentWidth: 5
    readonly property real extendWidth: 20
    readonly property real extendHeight: 10

    readonly property int animDuration: 300

    property bool showRectangleColorOverlay: false // <<< SET
    property var targetElement: null // <<< SET

    property real fromHeight: 0
    property real destHeight: 0
    property real destY: 0

    property real fromWidth: 0
    property real destWidth: 0
    property real destX: 0

    property bool posBindEnabled: false // <<< SET CTRL

    opacity: 0
    visible: false

    onTargetElementChanged: {
        if (!targetElement) {
            if (onTargetChangeAnim.running) {
                onTargetChangeAnim.pause();
            }
            onHideAnim.restart();
        } else {
            if (onHideAnim.running) {
                onHideAnim.pause();
            }
            if (onTargetChangeAnim.running) {
                onTargetChangeAnim.pause();
            }
            elementCursorHorizontalRoot.visible = false;
            const targetAbsPos = targetElement.mapToItem(parent, 0, 0);
            elementCursorHorizontalRoot.x = targetAbsPos.x - marginHorizontal - residentWidth - extendWidth;
            elementCursorHorizontalRoot.y = targetAbsPos.y - marginVertical - residentHeight - extendHeight;
            fromHeight = targetElement.height + 2 * (marginVertical + residentHeight + extendHeight);
            destHeight = targetElement.height + 2 * (marginVertical + residentHeight);
            fromWidth = targetElement.width + 2 * (marginHorizontal + residentWidth + extendWidth);
            destWidth = targetElement.width + 2 * (marginHorizontal + residentWidth);

            destX = targetAbsPos.x - marginHorizontal - residentWidth;
            destY = targetAbsPos.y - marginVertical - residentHeight;
            onTargetChangeAnim.restart();
        }
    }

    function forceUpdatePos() {
        if (!targetElement)
            return;
        const targetAbsPos = targetElement.mapToItem(parent, 0, 0);
        elementCursorHorizontalRoot.x = targetAbsPos.x - marginHorizontal - residentWidth;
        elementCursorHorizontalRoot.y = targetAbsPos.y - marginVertical - residentHeight;
    }

    FrameAnimation {
        running: elementCursorHorizontalRoot.posBindEnabled
        onTriggered: {
            elementCursorHorizontalRoot.forceUpdatePos();
        }
    }

    /*
    Binding {
    id: elementCursorHorizontalRootXBind
    when: elementCursorHorizontalRoot.posBindEnabled
    target: elementCursorHorizontalRoot
    property: "x"
    value:
    }
    */

    SequentialAnimation {
        id: onHideAnim
        running: false

        NumberAnimation {
            target: elementCursorHorizontalRoot
            property: "opacity"
            to: 0
            duration: 150
            easing.type: Easing.OutQuad
        }

        PropertyAction {
            target: elementCursorHorizontalRoot
            property: "visible"
            value: false
        }
    }

    SequentialAnimation {
        id: onTargetChangeAnim
        running: false

        PauseAnimation {
            duration: 10
        }

        ScriptAction {
            script: {
                elementCursorHorizontalRoot.width = elementCursorHorizontalRoot.fromWidth;
                elementCursorHorizontalRoot.height = elementCursorHorizontalRoot.fromHeight;
                elementCursorHorizontalRoot.opacity = 0;
                elementCursorHorizontalRoot.visible = true;
            }
        }

        /*
        ParallelAnimation {
        PropertyAction {
        target: elementCursorHorizontalRoot
        property: "opacity"
        value: 0
        }

        PropertyAction {
        target: elementCursorHorizontalRoot
        property: "visible"
        value: true
        }

        PropertyAction {
        target: elementCursorHorizontalRoot
        property: "width"
        value: elementCursorHorizontalRoot.fromWidth
        }

        PropertyAction {
        target: elementCursorHorizontalRoot
        property: "height"
        value: elementCursorHorizontalRoot.fromHeight
        }
        }
        */

        ParallelAnimation {
            NumberAnimation {
                target: elementCursorHorizontalRoot
                property: "width"
                to: elementCursorHorizontalRoot.destWidth
                duration: elementCursorHorizontalRoot.animDuration
                easing.type: Easing.Bezier
                easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
            }

            NumberAnimation {
                target: elementCursorHorizontalRoot
                property: "height"
                to: elementCursorHorizontalRoot.destHeight
                duration: elementCursorHorizontalRoot.animDuration
                easing.type: Easing.Bezier
                easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
            }

            NumberAnimation {
                target: elementCursorHorizontalRoot
                property: "x"
                to: elementCursorHorizontalRoot.destX
                duration: elementCursorHorizontalRoot.animDuration
                easing.type: Easing.Bezier
                easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
            }

            NumberAnimation {
                target: elementCursorHorizontalRoot
                property: "y"
                to: elementCursorHorizontalRoot.destY
                duration: elementCursorHorizontalRoot.animDuration
                easing.type: Easing.Bezier
                easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
            }

            NumberAnimation {
                target: elementCursorHorizontalRoot
                property: "opacity"
                duration: elementCursorHorizontalRoot.animDuration
                to: 1
            }
        }
    }

    Rectangle {
        id: elementCursorHorizontalFrameContainer
        anchors.fill: parent
        color: elementCursorHorizontalRoot.showRectangleColorOverlay
               ? AikariConstants.ThemeStyle.defaultStyle.textColorMoreOpacity : "transparent"

        Repeater {
            model: [0, 90, 180, 270] // top-left | top-right | bottom-right | bottom-left
            delegate: Item {
                readonly property real sideLen: elementCursorHorizontalRoot.height / 2 * 0.55

                height: sideLen
                width: sideLen

                anchors {
                    top: (modelData === 0 || modelData === 90) ? elementCursorHorizontalFrameContainer.top : undefined
                    bottom: (modelData === 180 || modelData === 270) ? elementCursorHorizontalFrameContainer.bottom :
                                                                       undefined
                    left: (modelData === 0 || modelData === 270) ? elementCursorHorizontalFrameContainer.left :
                                                                   undefined
                    right: (modelData === 90 || modelData === 180) ? elementCursorHorizontalFrameContainer.right :
                                                                     undefined
                }

                rotation: modelData

                Image {
                    id: frameSrcImg
                    anchors.fill: parent
                    visible: false
                    mipmap: true
                    fillMode: Image.PreserveAspectFit
                    source: 'qrc:/assets/img/components/atom/frame.svg'
                }

                MultiEffect {
                    source: frameSrcImg
                    anchors.fill: parent
                    visible: true
                    colorization: 1.0
                    colorizationColor: AikariConstants.ThemeStyle.defaultStyle.textColor
                }
            }
        }
    }
}
