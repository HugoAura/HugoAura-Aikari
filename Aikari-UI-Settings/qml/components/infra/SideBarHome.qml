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

    property var curActiveSideBarItem: null
    property bool curActiveSideBarItemCollapsed: false
    property var curActiveGroup: null
    property bool expandAnimIsActive: false

    function onCommonSideBarItemClicked() {
        selectedElementHighlighter.marginHorizontal = this.noMarginHorizontal ? -2.5 : -12.5;
        sideBarHomeRoot.curActiveSideBarItem = this; // `this` will be bound to sideBarCommonItemContainer
        sideBarHomeRoot.curActiveGroup = this.belongsToGroup ? this.belongsToGroup : null;
        sideBarHomeRoot.curActiveSideBarItemCollapsed = false;
        /* console.log(this.title); */
        // push...
    }

    function onSideBarGroupRootItemClicked() {
        if (sideBarHomeRoot.curActiveGroup === this) {
            /* console.log("oops"); */
            selectedElementHighlighter.marginHorizontal = this.isExpanded ? -2.5 : -12.5;
            sideBarHomeRoot.curActiveSideBarItemCollapsed = !this.isExpanded;
        } else {}
    }

    function onSideBarGroupContainerExpandAnimBegin(isAboutToExpand) {
        expandAnimIsActive = true;
    }

    function onSideBarGroupContainerExpandAnimEnd(isAboutToExpand) {
        expandAnimIsActive = false;
    }

    AikariComponentsAtom.ElementFrameHorizontal {
        id: selectedElementHighlighter
        targetElement: sideBarHomeRoot.curActiveSideBarItemCollapsed && sideBarHomeRoot.curActiveGroup
                       ? sideBarHomeRoot.curActiveGroup.expanderItem : sideBarHomeRoot.curActiveSideBarItem
        showRectangleColorOverlay: !sideBarHomeRoot.curActiveSideBarItemCollapsed
        /* marginHorizontal: sideBarHomeRoot.curActiveSideBarItem.noMarginHorizontal ? 2.5 : -12.5 */
        marginHorizontal: 2.5
        posBindEnabled: sideBarHomeRoot.expandAnimIsActive
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
        sideBarItemOnClicked: sideBarHomeRoot.onCommonSideBarItemClicked
    }

    AikariComponentsAtom.SideBarCommonItem {
        id: sideBarEntryGeneralSettings
        reusableStyleDefinition: sideBarHomeRoot.sideBarCommonItemStyle
        icon: MaterialSymbols.settings
        title: "通用设置"
        anchors.top: sideBarEntryDashboard.bottom
        sideBarItemOnClicked: sideBarHomeRoot.onCommonSideBarItemClicked
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
        onExpanderClicked: sideBarHomeRoot.onSideBarGroupRootItemClicked
        onExpandAnimBegin: sideBarHomeRoot.onSideBarGroupContainerExpandAnimBegin
        onExpandAnimEnd: sideBarHomeRoot.onSideBarGroupContainerExpandAnimEnd

        AikariComponentsAtom.SideBarGroupItem {
            id: sideBarGroupPLSDeviceSecurityEntryFreeze
            reusableStyleDefinition: sideBarHomeRoot.sideBarGroupItemStyle
            icon: MaterialSymbols.acUnit
            title: "冰点还原"
            sideBarItemOnClicked: sideBarHomeRoot.onCommonSideBarItemClicked
        }

        AikariComponentsAtom.SideBarGroupItem {
            id: sideBarGroupPLSDeviceSecurityEntrySoftware
            reusableStyleDefinition: sideBarHomeRoot.sideBarGroupItemStyle
            icon: MaterialSymbols.apps
            title: "软件管控"
            sideBarItemOnClicked: sideBarHomeRoot.onCommonSideBarItemClicked
        }

        AikariComponentsAtom.SideBarGroupItem {
            id: sideBarGroupPLSDeviceSecurityEntryNetwork
            reusableStyleDefinition: sideBarHomeRoot.sideBarGroupItemStyle
            icon: MaterialSymbols.bigtopUpdates
            title: "网络管控"
            sideBarItemOnClicked: sideBarHomeRoot.onCommonSideBarItemClicked
        }
    }

    AikariComponentsAtom.SideBarGroupContainer {
        id: sideBarGroupPLSDeviceManagement
        reusableStyleDefinition: sideBarHomeRoot.sideBarGroupRootStyle
        groupIcon: MaterialSymbols.devices
        groupTitle: "设备管理"
        anchors.top: sideBarGroupPLSDeviceSecurity.bottom
        onExpanderClicked: sideBarHomeRoot.onSideBarGroupRootItemClicked
        onExpandAnimBegin: sideBarHomeRoot.onSideBarGroupContainerExpandAnimBegin
        onExpandAnimEnd: sideBarHomeRoot.onSideBarGroupContainerExpandAnimEnd

        AikariComponentsAtom.SideBarGroupItem {
            id: sideBarGroupPLSDeviceManagementEntryDeviceLock
            reusableStyleDefinition: sideBarHomeRoot.sideBarGroupItemStyle
            icon: MaterialSymbols.lock
            title: "锁屏与屏保"
            sideBarItemOnClicked: sideBarHomeRoot.onCommonSideBarItemClicked
        }
    }
}
