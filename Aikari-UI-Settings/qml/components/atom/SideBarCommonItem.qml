import QtQuick
import QtQuick.Effects
import AikariConstants as AikariConstants
import AikariComponents.Shapes as AikariComponentsShapes

Item {
    id: sideBarCommonItemContainer

    property alias icon: sideBarCommonItemRoot.icon // <<< SET
    property alias title: sideBarCommonItemRoot.title // <<< SET
    property var sideBarItemOnClicked: null // <<< SET
    required property var reusableStyleDefinition // <<< SET

    property bool noMarginHorizontal: false // <<< SET
    property bool noMarginVertical: false // <<< SET
    /*
    Accepts:
    {
    width: int
    height: int
    anchorLeft: AnchorLine | undefined
    anchorRight: AnchorLine | undefined
    }
    */

    property var belongsToGroup: null // <<< DELEGATE

    readonly property real marginHorizontal: noMarginHorizontal ? 0 : 10
    readonly property real marginVertical: noMarginVertical ? 0 : 2.5

    readonly property real iconPixelSize: height * 0.5
    readonly property real titlePixelSize: height * 0.325

    readonly property int colorAnimDuration: 150

    readonly property alias primaryColorReversable: sideBarCommonItemRoot.primaryColorReversable
    width: reusableStyleDefinition.width
    height: reusableStyleDefinition.height
    anchors.left: reusableStyleDefinition.anchorLeft
    anchors.right: reusableStyleDefinition.anchorRight
    visible: true

    AikariComponentsShapes.MaskClippedRectangle {
        id: sideBarCommonItemMask
        width: parent.width
        height: parent.height
        visible: false
        anchors {
            fill: parent
            topMargin: sideBarCommonItemContainer.marginVertical
            bottomMargin: sideBarCommonItemContainer.marginVertical
            leftMargin: sideBarCommonItemContainer.marginHorizontal
            rightMargin: sideBarCommonItemContainer.marginHorizontal
        }

        clipTriangleSize: 10
    }

    Rectangle {
        id: sideBarCommonItemRoot
        property var reusableStyleDefinition: sideBarCommonItemContainer.reusableStyleDefinition
        property alias icon: sideBarCommonItemIconText.text // <<< SET
        property alias title: sideBarCommonItemTitleText.text // <<< SET
        readonly property color primaryColor: AikariConstants.ThemeStyle.defaultStyle.textColor
        readonly property color reversedColor: AikariConstants.ThemeStyle.defaultStyle.background
        property bool isPressForceActive: false // Make sure the ColorAnimation alwaysRunToEnd
        property bool colorReversed: sideBarCommonItemTapHandler.pressed || isPressForceActive
        readonly property color primaryColorReversable: colorReversed ? reversedColor : primaryColor
        readonly property color reversedColorReversable: colorReversed ? primaryColor : reversedColor
        readonly property color bgColorReversable: colorReversed ? primaryColor : "transparent"
        color: sideBarCommonItemHoverHandler.hovered && !sideBarCommonItemTapHandler.pressed && !isPressForceActive
               ? AikariConstants.ThemeStyle.defaultStyle.textColorMoreOpacity : bgColorReversable

        width: parent.width
        height: parent.height
        anchors {
            fill: parent
            topMargin: sideBarCommonItemContainer.marginVertical
            bottomMargin: sideBarCommonItemContainer.marginVertical
            leftMargin: sideBarCommonItemContainer.marginHorizontal
            rightMargin: sideBarCommonItemContainer.marginHorizontal
        }

        visible: true

        Behavior on color {
            ColorAnimation {
                duration: sideBarCommonItemContainer.colorAnimDuration
                easing.type: Easing.InOutQuad
            }
        }

        Text {
            id: sideBarCommonItemIconText
            font.pixelSize: sideBarCommonItemContainer.iconPixelSize
            font.family: AikariConstants.FontLoaders.materialSymbolsLoader.name
            font.styleName: "Regular"
            // >>> text: from alias sideBarCommonItemRoot.icon
            color: sideBarCommonItemRoot.primaryColorReversable
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10

            Behavior on color {
                ColorAnimation {
                    duration: sideBarCommonItemContainer.colorAnimDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Text {
            id: sideBarCommonItemTitleText
            font.pixelSize: sideBarCommonItemContainer.titlePixelSize
            // >>> text: from alias sideBarCommonItemRoot.title
            color: sideBarCommonItemRoot.primaryColorReversable
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: sideBarCommonItemIconText.right
            anchors.leftMargin: 10

            Behavior on color {
                ColorAnimation {
                    duration: sideBarCommonItemContainer.colorAnimDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }

        HoverHandler {
            id: sideBarCommonItemHoverHandler
        }

        TapHandler {
            id: sideBarCommonItemTapHandler
            onTapped: {
                parent.isPressForceActive = true;
                sideBarCommonItemPressActionTimer.start();
                if (sideBarCommonItemContainer.sideBarItemOnClicked) {
                    sideBarCommonItemContainer.sideBarItemOnClicked.call(sideBarCommonItemContainer);
                }
            }
        }

        Timer {
            id: sideBarCommonItemPressActionTimer
            interval: 130
            repeat: false
            onTriggered: {
                parent.isPressForceActive = false;
            }
        }

        layer.enabled: true
        layer.effect: MultiEffect {
            maskEnabled: true
            maskSource: sideBarCommonItemMask
        }
    }
}
