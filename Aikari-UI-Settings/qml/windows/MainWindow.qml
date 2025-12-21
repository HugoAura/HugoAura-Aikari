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

    color: AikariConstants.ThemeStyle.defaultStyle.background
    height: 900
    title: qsTr("Aikari")
    visible: true
    width: 1600

    Component.onCompleted: {
        qwkWindowAgent.setup(mainWindowRoot);
        qwkWindowAgent.setWindowAttribute("dark-mode", AikariConstants.ThemeStyle.defaultStyle.isDark);
        AikariConstants.ThemeStyle.environmentStates.rootWindow = mainWindowRoot;
    }

    QWK.WindowAgent {
        id: qwkWindowAgent
    }
    AppBar {
        id: mainAppBar

        parentWindow: mainWindowRoot
        z: 150

        Component.onCompleted: {
            qwkWindowAgent.setTitleBar(mainAppBar);
            mainWindowLoader.active = true;
        }
    }
    PageAnimAgent {
        id: mainWindowAnimAgent

        rootObject: mainWindowRoot
        z: 100
    }
    AikariRouter.Router {
        id: mainWindowRouter

        animAgentId: mainWindowAnimAgent
        curActivePage: "qrc:/AikariViews/SplashView.qml"
        currentLoader: mainWindowLoader
    }
    Loader {
        id: mainWindowLoader

        active: false
        anchors.left: mainWindowRoot.left
        anchors.top: mainAppBar.bottom
        asynchronous: true
        height: mainWindowRoot.height - mainAppBar.height
        source: mainWindowRouter.curActivePage
        width: mainWindowRoot.width
        z: 1
    }
}
