import QtQuick
import AikariConstants as AikariConstants
import AikariComponents.Atom as AikariComponentsAtom
import "qrc:/constants/MaterialSymbolsFont.js" as MaterialSymbols

AikariComponentsAtom.SideBar {
    id: sideBarHomeRoot
    required property var viewRoot
    color: "transparent"
    expectedSideBarWidth: 350
    sideBarMaskColor: AikariConstants.ThemeStyle.defaultStyle.textColor
    anchors.left: viewRoot.left
    anchors.top: viewRoot.top
    height: viewRoot.height

    property QtObject sideBarCommonItemStyle: QtObject {
        property int width: sideBarHomeRoot.width
        property int height: 50
        property var anchorLeft: sideBarHomeRoot.left
    }

    property QtObject sideBarGroupItemStyle: QtObject {
        property int expanderWidth: sideBarHomeRoot.width
        property int expanderHeight: 50
        property var expanderAnchorLeft: sideBarHomeRoot.left
    }

    Item {
        id: sideBarTopMarginPlaceholder
        height: 5
    }

    AikariComponentsAtom.SideBarCommonItem {
        id: sideBarEntryDashboard
        reusableStyleDefinition: sideBarHomeRoot.sideBarCommonItemStyle
        icon: MaterialSymbols.home
        title: "仪表盘"
        anchors.top: sideBarTopMarginPlaceholder.bottom
    }

    AikariComponentsAtom.SideBarCommonItem {
        id: sideBarEntryGeneralSettings
        reusableStyleDefinition: sideBarHomeRoot.sideBarCommonItemStyle
        icon: MaterialSymbols.settings
        title: "通用设置"
        anchors.top: sideBarEntryDashboard.bottom
    }

    Rectangle {
        id: sideBarHrCommonAndFeatures
        width: sideBarHomeRoot.expectedSideBarWidth - 20
        height: 0.75
        anchors {
            horizontalCenter: sideBarHomeRoot.horizontalCenter
            top: sideBarEntryGeneralSettings.bottom
            topMargin: 5
        }
        color: AikariConstants.ThemeStyle.defaultStyle.textColorMoreOpacity
    }

    AikariComponentsAtom.SideBarGroupContainer {
        id: sideBarGroupPLSReportMocking
        reusableStyleDefinition: sideBarHomeRoot.sideBarGroupItemStyle
        groupIcon: MaterialSymbols.fileUploadOff
        groupTitle: "上报覆写"
        anchors.top: sideBarHrCommonAndFeatures.bottom
        anchors.topMargin: 5
    }
}
