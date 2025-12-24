import QtQuick
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
        childHeight: int
        childAnchorLeft: AnchorLine
    }
    */
    readonly property real childMarginHorizontal: 10
    readonly property real childMarginVertical: 2.5

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
            property int width: reusableStyleDefinition.expanderWidth
            property int height: reusableStyleDefinition.expanderHeight
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
}
