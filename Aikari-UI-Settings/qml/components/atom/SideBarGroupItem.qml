import QtQuick
import AikariComponents.Animations as AikariComponentsAnimations
import AikariConstants as AikariConstants

Item {
    id: sideBarGroupItemRoot
    property alias icon: sideBarGroupItemInner.icon // <<< SET
    property alias title: sideBarGroupItemInner.title // <<< SET
    required property var reusableStyleDefinition
    property alias sideBarItemOnClicked: sideBarGroupItemInner.sideBarItemOnClicked // <<< SET
    property alias animTimerShowDelay: componentSnapMaskAnimIsShowDelayToggler.toShowDelay // <<< SET
    property alias animTimerHideDelay: componentSnapMaskAnimIsShowDelayToggler.toHideDelay // <<< SET
    width: reusableStyleDefinition.width
    height: reusableStyleDefinition.height

    anchors.left: parent.left

    function triggerTimer(toShow) {
        componentSnapMaskAnimIsShowDelayToggler.interval = toShow ?
            componentSnapMaskAnimIsShowDelayToggler.toShowDelay :
            componentSnapMaskAnimIsShowDelayToggler.toHideDelay;
        componentSnapMaskAnimIsShowDelayToggler.targetIsShowVal = toShow;
        componentSnapMaskAnimIsShowDelayToggler.restart();
    }

    Timer {
        id: componentSnapMaskAnimIsShowDelayToggler
        repeat: false
        running: false
        // >>> interval: from alias componentSnapMaskAnimRoot.timerDelay

        property int toShowDelay: 0
        property int toHideDelay: 0
        property bool targetIsShowVal: true

        onTriggered: {
            sideBarGroupItemMask.isShow = targetIsShowVal;
        }
    }

    Item {
        id: sideBarGroupItemContainer
        anchors.fill: parent
        anchors {
            leftMargin: 10
            rightMargin: 10
        }

        AikariComponentsAnimations.ComponentSnapMask {
            id: sideBarGroupItemMask
            maskColor: AikariConstants.ThemeStyle.defaultStyle.textColor
            maskZIndex: 2
            expectedLength: parent.width
            height: parent.height
            isShow: false
            anchors.left: parent.left

            duration: 250
            freeze: 75

            SideBarCommonItem {
                id: sideBarGroupItemInner
                // >>> icon
                // >>> title
                // >>> reusableStyleDefinition
                height: sideBarGroupItemContainer.height
                width: sideBarGroupItemContainer.width
                reusableStyleDefinition: QtObject {
                    property int width: sideBarGroupItemContainer.width
                    property int height: sideBarGroupItemContainer.height
                }

                // anchors.top: parent.top
                anchors.left: parent.left

                noMarginHorizontal: true
            }
        }
    }
}
