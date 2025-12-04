import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QWindowKit as QWK
import AikariConstants as AikariConstants
import AikariRouter as AikariRouter
import AikariComponents.Infra

Window {
    id: mainWindowRoot
    width: 1400
    height: 800
    visible: true
    title: qsTr("Aikari")
    color: AikariConstants.ThemeStyle.defaultStyle.background

    Component.onCompleted: {
        qwkWindowAgent.setup(mainWindowRoot)
        qwkWindowAgent.setWindowAttribute("dark-mode", AikariConstants.ThemeStyle.defaultStyle.isDark)
    }

    QWK.WindowAgent {
        id: qwkWindowAgent
    }

    AppBar {
        id: mainAppBar
        Component.onCompleted: {
            qwkWindowAgent.setTitleBar(mainAppBar)
            mainWindowLoader.active = true
        }
    }

    Loader {
        id: mainWindowLoader
        height: parent.height - mainAppBar.height
        width: parent.width
        anchors.top: mainAppBar.bottom
        anchors.left: parent.left
        active: false
        source: AikariRouter.Router.curActivePage
        asynchronous: true
    }
}
