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

    readonly property QtObject sideBarCommonItemStyle: QtObject {
        readonly property int width: sideBarHomeRoot.width
        readonly property int height: 50
        readonly property var anchorLeft: sideBarHomeRoot.left
    }

    readonly property QtObject sideBarGroupRootStyle: QtObject {
        readonly property int expanderWidth: sideBarHomeRoot.width
        readonly property int expanderHeight: 50
        readonly property var expanderAnchorLeft: sideBarHomeRoot.left
        readonly property int childWidth: sideBarHomeRoot.width - 25
        readonly property int expandAnimDuration: 250
        readonly property int collapseAnimDuration: 500
    }

    readonly property QtObject sideBarGroupItemStyle: QtObject {
        readonly property int width: sideBarGroupRootStyle.childWidth
        readonly property int height: 50
        readonly property var anchorLeft: undefined
        readonly property var anchorRight: undefined
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
        id: sideBarGroupPLSDeviceSecurity
        reusableStyleDefinition: sideBarHomeRoot.sideBarGroupRootStyle
        groupIcon: MaterialSymbols.gppMaybe
        groupTitle: "设备安全"
        anchors.top: sideBarHrCommonAndFeatures.bottom
        anchors.topMargin: 5

        AikariComponentsAtom.SideBarGroupItem {
            id: sideBarGroupPLSDeviceSecurityEntryFreeze
            reusableStyleDefinition: sideBarHomeRoot.sideBarGroupItemStyle
            icon: MaterialSymbols.acUnit
            title: "冰点还原"
        }

        AikariComponentsAtom.SideBarGroupItem {
            id: sideBarGroupPLSDeviceSecurityEntrySoftware
            reusableStyleDefinition: sideBarHomeRoot.sideBarGroupItemStyle
            icon: MaterialSymbols.apps
            title: "软件管控"
        }

        AikariComponentsAtom.SideBarGroupItem {
            id: sideBarGroupPLSDeviceSecurityEntryNetwork
            reusableStyleDefinition: sideBarHomeRoot.sideBarGroupItemStyle
            icon: MaterialSymbols.bigtopUpdates
            title: "网络管控"
        }
    }

    AikariComponentsAtom.SideBarGroupContainer {
        id: sideBarGroupPLSDeviceManagement
        reusableStyleDefinition: sideBarHomeRoot.sideBarGroupRootStyle
        groupIcon: MaterialSymbols.devices
        groupTitle: "设备管理"
        anchors.top: sideBarGroupPLSDeviceSecurity.bottom

        AikariComponentsAtom.SideBarGroupItem {
            id: sideBarGroupPLSDeviceManagementEntryDeviceLock
            reusableStyleDefinition: sideBarHomeRoot.sideBarGroupItemStyle
            icon: MaterialSymbols.lock
            title: "锁屏与屏保"
        }
    }
}
