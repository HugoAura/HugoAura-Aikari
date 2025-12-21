import QtQuick
import AikariConstants as AikariConstants

Rectangle {
    id: sideBarCommonItemRoot
    required property var reusableStyleDefinition // <<< SET
    property alias icon: sideBarCommonItemIconText.text // <<< SET
    property alias title: sideBarCommonItemTitleText.text // <<< SET
    readonly property color primaryColor: AikariConstants.ThemeStyle.defaultStyle.textColor
    readonly property color reversedColor: AikariConstants.ThemeStyle.defaultStyle.background
    property bool colorReversed: false
    readonly property color primaryColorReversable: colorReversed ? reversedColor : primaryColor
    readonly property color reversedColorReversable: colorReversed ? primaryColor : reversedColor
    color: reversedColorReversable

    /*
    Accepts:
    {
    width: int
    height: int
    anchorLeft: AnchorLine
    }
    */
    width: reusableStyleDefinition.width
    height: reusableStyleDefinition.height
    anchors.left: reusableStyleDefinition.anchorLeft

    Text {
        id: sideBarCommonItemIconText
        font.family: AikariConstants.FontLoaders.materialSymbolsLoader.name
        font.pixelSize: parent.height * 0.45
        font.weight: Font.Light
        textFormat: Text.PlainText
        // >>> text: from alias sideBarCommonItemRoot.icon
        color: sideBarCommonItemRoot.primaryColorReversable
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 10
    }

    Text {
        id: sideBarCommonItemTitleText
        font.pixelSize: parent.height * 0.45
        // >>> text: from alias sideBarCommonItemRoot.title
        color: sideBarCommonItemRoot.primaryColorReversable
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: sideBarCommonItemIconText.right
        anchors.leftMargin: 10
    }
}
