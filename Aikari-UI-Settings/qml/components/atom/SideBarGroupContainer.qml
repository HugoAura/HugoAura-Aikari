import QtQuick
import QtQuick.Layouts
import "qrc:/constants/MaterialSymbolsFont.js" as MaterialSymbols
import AikariConstants as AikariConstants

Item {
    id: sideBarGroupContainerRoot

    property alias groupIcon: sideBarGroupExpanderItem.icon // <<< SET
    property alias groupTitle: sideBarGroupExpanderItem.title // <<< SET
    required property var reusableStyleDefinition // <<< SET
    /*
    Accepts:
    {
        expanderWidth: int
        expanderHeight: int
        expanderAnchorLeft: AnchorLine
        childWidth: int
        expandAnimDuration: int
        collapseAnimDuration: int
    }
    */
    readonly property real childMarginHorizontal: 10
    readonly property real childMarginVertical: 2.5

    default property alias innerContent: sideBarGroupContainerInnerItems.data

    property bool isExpanded: false // <<< State

    width: reusableStyleDefinition.expanderWidth
    height: childrenRect.height
    anchors {
        left: reusableStyleDefinition.expanderAnchorLeft
    }
    visible: true

    function onGroupToggleExpand() {
        sideBarGroupContainerRoot.isExpanded = !sideBarGroupContainerRoot.isExpanded
    }

    SideBarCommonItem {
        id: sideBarGroupExpanderItem
        reusableStyleDefinition: QtObject {
            property int width: sideBarGroupContainerRoot.reusableStyleDefinition.expanderWidth
            property int height: sideBarGroupContainerRoot.reusableStyleDefinition.expanderHeight
            property var anchorLeft: sideBarGroupContainerRoot.left
        }
        // >>> icon: from alias sideBarGroupContainerRoot.groupIcon
        // >>> title: from alias sideBarGroupContainerRoot.groupTitle
        sideBarItemOnClicked: () => sideBarGroupContainerRoot.onGroupToggleExpand()

        Text {
            id: sideBarGroupExpanderChevronHint
            font.pixelSize: sideBarGroupExpanderItem.iconPixelSize
            font.family: AikariConstants.FontLoaders.materialSymbolsLoader.name
            font.styleName: "Regular"
            text: MaterialSymbols.keyboardArrowDown
            color: sideBarGroupExpanderItem.primaryColorReversable
            anchors {
                verticalCenter: sideBarGroupExpanderItem.verticalCenter
                right: sideBarGroupExpanderItem.right
                rightMargin: 20
            }
            rotation: sideBarGroupContainerRoot.isExpanded ? 180 : 0

            Behavior on color {
                ColorAnimation {
                    duration: sideBarGroupExpanderItem.colorAnimDuration
                    easing.type: Easing.InOutQuad
                }
            }

            Behavior on rotation {
                RotationAnimation {
                    duration: 300
                    direction: RotationAnimation.Clockwise
                    easing.type: Easing.Bezier
                    easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
                }
            }
        }
    }

    Item {
        id: sideBarGroupContainerInnerItemsAnimFrame
        width: sideBarGroupContainerRoot.reusableStyleDefinition.childWidth
        anchors.right: parent.right
        anchors.top: sideBarGroupExpanderItem.bottom

        clip: true
        readonly property bool isExpanded: sideBarGroupContainerRoot.isExpanded

        states: [
            State {
                name: "collapsed"
                PropertyChanges {
                    target: sideBarGroupContainerInnerItemsAnimFrame
                    height: 0
                }
            },
            State {
                name: "expanded"
                PropertyChanges {
                    target: sideBarGroupContainerInnerItemsAnimFrame
                    height: sideBarGroupContainerInnerItems.height
                }
            }
        ]

        transitions: [
            Transition {
                to: "expanded"
                NumberAnimation {
                    properties: "height"
                    duration: sideBarGroupContainerRoot.reusableStyleDefinition.expandAnimDuration
                    easing.type: Easing.Bezier
                    easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
                }
            },
            Transition {
                to: "collapsed"
                NumberAnimation {
                    properties: "height"
                    duration: sideBarGroupContainerRoot.reusableStyleDefinition.collapseAnimDuration
                    easing.type: Easing.Bezier
                    easing.bezierCurve: AikariConstants.ThemeStyle.cubicBeziers.snapTo
                }
            }
        ]

        onIsExpandedChanged: {
            sideBarGroupContainerInnerItemsAnimFrame.state = isExpanded ? "expanded" : "collapsed";
        }

        Column {
            id: sideBarGroupContainerInnerItems
            width: parent.width
            // height: childrenRect.height
            readonly property bool isExpanded: sideBarGroupContainerRoot.isExpanded

            Component.onCompleted: {
                const diff = 100;
                const childrenLength = sideBarGroupContainerInnerItems.children.length;
                for (let i = 0; i < childrenLength; i++) {
                    const child = sideBarGroupContainerInnerItems.children[i];
                    child.animTimerShowDelay = diff * i;
                    // child.animTimerHideDelay = (childrenLength - 1 - i) * diff;
                    child.animTimerHideDelay = 0;
                }
            }

            onIsExpandedChanged: {
                for (const child of sideBarGroupContainerInnerItems.children) {
                    child.triggerTimer(isExpanded);
                }
            }
        }
    }
}
