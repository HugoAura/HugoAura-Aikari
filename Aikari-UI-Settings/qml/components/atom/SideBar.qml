import QtQuick
import AikariConstants as AikariConstants

Rectangle {
    id: sideBarAtomRoot

    width: 0
    property bool sideBarIsShow: false
    property alias sideBarMaskColor: sideBarAtomMask.color
    property alias sideBarIsShowBinding: sideBarIsShowBind
    property int sideBarAnimationDuration: 300
    property int sideBarPauseAnimationDuration: 100
    property int expectedSideBarWidth: 0
    clip: true

    color: "transparent"

    Binding {
        id: sideBarIsShowBind
        target: sideBarAtomRoot
        property: "sideBarIsShow"
        value: AikariConstants.ThemeStyle.themeStates.isMainWindowSideBarShow
        when: false
    }

    onSideBarIsShowChanged: {
        if (sideBarIsShow) {
            if (sideBarAnimationGone.running) {
                sideBarAnimationGone.pause();
            }
            sideBarAnimationShow.restart(); // TODO: Not directly restart, but check cur anim step
        } else {
            if (sideBarAnimationShow.running) {
                sideBarAnimationShow.pause();
            }
            sideBarAnimationGone.restart();
        }
    }

    Rectangle {
        id: sideBarAtomMask

        height: sideBarAtomRoot.height
        width: 0
        z: 5

        /*
        Behavior on width {
        NumberAnimation {
        duration: 200
        easing.type: Easing.Bezier
        easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
        }
        }
        */

        anchors {
            left: sideBarAtomRoot.left
            top: sideBarAtomRoot.top
        }

        Binding {
            id: sideBarMaskWidthBind
            target: sideBarAtomMask
            property: "width"
            value: sideBarAtomRoot.width
            when: false
        }
    }

    // TODO: Switch impl to state
    SequentialAnimation {
        id: sideBarAnimationShow
        running: false
        ScriptAction {
            script: {
                sideBarMaskWidthBind.when = true;
                sideBarAtomMask.width = Qt.binding(() => sideBarAtomRoot.width);
                sideBarAtomMask.anchors.left = Qt.binding(() => sideBarAtomRoot.left);
                sideBarAtomMask.anchors.right = undefined;
            }
        }
        NumberAnimation {
            target: sideBarAtomRoot
            property: "width"
            to: sideBarAtomRoot.expectedSideBarWidth
            duration: sideBarAtomRoot.sideBarAnimationDuration
            easing.type: Easing.Bezier
            easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
        }
        ScriptAction {
            script: {
                sideBarMaskWidthBind.when = false;
                sideBarAtomMask.width = sideBarAtomRoot.width;
                sideBarAtomMask.anchors.left = undefined;
                sideBarAtomMask.anchors.right = Qt.binding(() => sideBarAtomRoot.right);
            }
        }
        PauseAnimation {
            duration: sideBarAtomRoot.sideBarPauseAnimationDuration
        }
        NumberAnimation {
            target: sideBarAtomMask
            property: "width"
            to: 0
            duration: sideBarAtomRoot.sideBarAnimationDuration
            easing.type: Easing.Bezier
            easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
        }
    }

    SequentialAnimation {
        id: sideBarAnimationGone
        running: false
        ScriptAction {
            script: {
                sideBarMaskWidthBind.when = false;
                sideBarAtomMask.width = 0;
                sideBarAtomMask.anchors.left = undefined;
                sideBarAtomMask.anchors.right = Qt.binding(() => sideBarAtomRoot.right);
            }
        }
        NumberAnimation {
            target: sideBarAtomMask
            property: "width"
            to: sideBarAtomRoot.width
            duration: sideBarAtomRoot.sideBarAnimationDuration
            easing.type: Easing.Bezier
            easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
        }
        PauseAnimation {
            duration: sideBarAtomRoot.sideBarPauseAnimationDuration
        }
        ScriptAction {
            script: {
                sideBarMaskWidthBind.when = true;
                // sideBarAtomMask.width = Qt.binding(() => sideBarAtomRoot.width);
                sideBarAtomMask.anchors.left = Qt.binding(() => sideBarAtomRoot.left);
                sideBarAtomMask.anchors.right = undefined;
            }
        }
        NumberAnimation {
            target: sideBarAtomRoot
            property: "width"
            to: 0
            duration: sideBarAtomRoot.sideBarAnimationDuration
            easing.type: Easing.Bezier
            easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
        }
        /*
        ScriptAction {
        script: {
        console.debug(1, sideBarAtomRoot.width);
        console.debug(2, sideBarAtomMask.width);
        }
        }
        */
    }
}
