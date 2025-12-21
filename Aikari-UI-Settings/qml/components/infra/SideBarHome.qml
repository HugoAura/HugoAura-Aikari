import QtQuick
import AikariConstants as AikariConstants
import AikariComponents.Atom as AikariComponentsAtom
import "qrc:/constants/IconFont.js" as MaterialSymbols

AikariComponentsAtom.SideBar {
    id: sideBarHomeRoot
    required property var viewRoot
    color: "transparent"
    expectedSideBarWidth: 380
    sideBarMaskColor: AikariConstants.ThemeStyle.defaultStyle.textColor
    anchors.left: viewRoot.left
    anchors.top: viewRoot.top
    height: viewRoot.height

    property QtObject sideBarCommonItemStyle: QtObject {
        property int width: sideBarHomeRoot.width
        property int height: 55
        property var anchorLeft: sideBarHomeRoot.left
    }

    AikariComponentsAtom.SideBarCommonItem {
        reusableStyleDefinition: sideBarHomeRoot.sideBarCommonItemStyle
        icon: MaterialSymbols.home
        title: "仪表盘"
        Component.onCompleted: {
            console.log(MaterialSymbols.home);
        }
    }
}
