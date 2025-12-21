import QtQuick
import AikariComponents.Infra as AikariComponentsInfra
import AikariConstants as AikariConstants

Rectangle {
    id: homeViewRoot
    color: "transparent"
    height: parent.height
    width: parent.width
    anchors {
        top: parent.top
        left: parent.left
    }
    Component.onCompleted: {
        homeViewEntranceAnim.running = true;
    }

    AikariComponentsInfra.SideBarHome {
        id: homeSideBar
        viewRoot: homeViewRoot
    }

    SequentialAnimation {
        id: homeViewEntranceAnim
        running: false

        PauseAnimation {
            duration: 500
        }
        ScriptAction {
            script: {
                homeSideBar.sideBarIsShowBinding.when = true;
                AikariConstants.ThemeStyle.themeStates.mainAppBarShowSideBarSwitchBtn = true;
            }
        }
    }
}
