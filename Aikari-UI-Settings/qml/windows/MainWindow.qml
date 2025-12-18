import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QWindowKit as QWK
import AikariConstants as AikariConstants
import AikariRouter as AikariRouter
import AikariComponents.Infra
import AikariComponents.Animations

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
        z: 150
        Component.onCompleted: {
            qwkWindowAgent.setTitleBar(mainAppBar)
            mainWindowLoader.active = true
        }
    }

    PageAnimAgent {
        id: mainWindowAnimAgent
        rootObject: mainWindowRoot
    }

    AikariRouter.Router {
        id: mainWindowRouter
        animAgentId: mainWindowAnimAgent
        currentLoader: mainWindowLoader
        curActivePage: "qrc:/AikariViews/SplashView.qml"
    }

    Loader {
        id: mainWindowLoader
        height: mainWindowRoot.height - mainAppBar.height
        width: mainWindowRoot.width
        anchors.top: mainAppBar.bottom
        anchors.left: mainWindowRoot.left
        active: false
        source: mainWindowRouter.curActivePage
        asynchronous: true
    }
}
