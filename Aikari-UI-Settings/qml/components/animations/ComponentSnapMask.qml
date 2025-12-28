import QtQuick
import AikariConstants as AikariConstants

Item {
    id: componentSnapMaskAnimRoot
    // >>> LAYOUT <<< //
    property alias maskColor: componentSnapMaskAnimMask.color // <<< SET
    property string swipeMode: "horizontal" // <<< SET // "horizontal" || "vertical"
    property alias maskZIndex: componentSnapMaskAnimMask.z // <<< SET (Optional)
    required property int expectedLength // <<< SET
    property int duration: 300 // <<< SET (Optional)
    property int freeze: 150 // <<< SET (Optional)
    readonly property bool swipeHorizontally: swipeMode === "horizontal" // <<< PRIVATE
    readonly property string shouldSwipeProp: swipeHorizontally ? "width" : "height" // <<< PRIV STATE
    readonly property string initialState: swipeHorizontally ? "leftAlignedHorizontal" :
                                                               "topAlignedVertical" // <<< PRIV STATE

    readonly property string endUpState: swipeHorizontally ? "rightAlignedHorizontal" :
                                                             "bottomAlignedVertical" // <<< PRIV STATE

    // >>> CONTROLS <<< //
    property bool isShow: false

    clip: true
    visible: true
    width: 0

    Rectangle {
        id: componentSnapMaskAnimMask
        // >>> color: from alias componentSnapMaskAnimRoot.maskColor
        z: 5 // >>> z: from alias componentSnapMaskAnimRoot.maskZIndex (default=5)
        height: componentSnapMaskAnimRoot.height
        width: 0

        Binding {
            id: componentSnapMaskAnimMaskWidthBind
            when: false
            target: componentSnapMaskAnimMask
            property: "width"
            value: componentSnapMaskAnimRoot.width
        }
    }

    states: [
        State {
            name: "leftAlignedHorizontal"
            AnchorChanges {
                target: componentSnapMaskAnimMask
                anchors.top: componentSnapMaskAnimRoot.top
                anchors.left: componentSnapMaskAnimRoot.left
                anchors.right: undefined
                anchors.bottom: undefined
            }
        },
        State {
            name: "rightAlignedHorizontal"
            AnchorChanges {
                target: componentSnapMaskAnimMask
                anchors.top: componentSnapMaskAnimRoot.top
                anchors.left: undefined
                anchors.right: componentSnapMaskAnimRoot.right
                anchors.bottom: undefined
            }
        },
        State {
            name: "topAlignedVertical"
            AnchorChanges {
                target: componentSnapMaskAnimMask
                anchors.top: componentSnapMaskAnimRoot.top
                anchors.left: componentSnapMaskAnimRoot.left
                anchors.right: undefined
                anchors.bottom: undefined
            }
        },
        State {
            name: "bottomAlignedVertical"
            AnchorChanges {
                target: componentSnapMaskAnimMask
                anchors.top: undefined
                anchors.left: componentSnapMaskAnimRoot.left
                anchors.right: undefined
                anchors.bottom: componentSnapMaskAnimRoot.bottom
            }
        }
    ]

    onIsShowChanged: {
        const past = isShow ? componentSnapMaskAnimHide : componentSnapMaskAnimShow;
        const present = isShow ? componentSnapMaskAnimShow : componentSnapMaskAnimHide;
        if (past.running) {
            past.pause();
        }
        present.restart();
    }

    SequentialAnimation {
        id: componentSnapMaskAnimShow
        running: false
        ScriptAction {
            script: {
                componentSnapMaskAnimRoot.state = componentSnapMaskAnimRoot.initialState;
                componentSnapMaskAnimMaskWidthBind.when = true;
            }
        }
        NumberAnimation {
            target: componentSnapMaskAnimRoot
            property: componentSnapMaskAnimRoot.shouldSwipeProp
            to: componentSnapMaskAnimRoot.expectedLength
            duration: componentSnapMaskAnimRoot.duration
            easing.type: Easing.Bezier
            easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
        }
        ScriptAction {
            script: {
                componentSnapMaskAnimRoot.state = componentSnapMaskAnimRoot.endUpState;
                componentSnapMaskAnimMaskWidthBind.when = false;
                componentSnapMaskAnimMask.width = componentSnapMaskAnimRoot.width;
            }
        }
        PauseAnimation {
            duration: componentSnapMaskAnimRoot.freeze
        }
        NumberAnimation {
            target: componentSnapMaskAnimMask
            property: componentSnapMaskAnimRoot.shouldSwipeProp
            to: 0
            duration: componentSnapMaskAnimRoot.duration
            easing.type: Easing.Bezier
            easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
        }
    }

    SequentialAnimation {
        id: componentSnapMaskAnimHide
        running: false
        ScriptAction {
            script: {
                componentSnapMaskAnimRoot.state = componentSnapMaskAnimRoot.endUpState;
                componentSnapMaskAnimMaskWidthBind.when = false;
                componentSnapMaskAnimMask.width = 0;
            }
        }
        NumberAnimation {
            target: componentSnapMaskAnimMask
            property: componentSnapMaskAnimRoot.shouldSwipeProp
            to: componentSnapMaskAnimRoot.expectedLength
            duration: componentSnapMaskAnimRoot.duration
            easing.type: Easing.Bezier
            easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
        }
        PauseAnimation {
            duration: componentSnapMaskAnimRoot.freeze
        }
        ScriptAction {
            script: {
                componentSnapMaskAnimRoot.state = componentSnapMaskAnimRoot.initialState;
                componentSnapMaskAnimMaskWidthBind.when = true;
            }
        }
        NumberAnimation {
            target: componentSnapMaskAnimRoot
            property: componentSnapMaskAnimRoot.shouldSwipeProp
            to: 0
            duration: componentSnapMaskAnimRoot.duration
            easing.type: Easing.Bezier
            easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
        }
    }
}
