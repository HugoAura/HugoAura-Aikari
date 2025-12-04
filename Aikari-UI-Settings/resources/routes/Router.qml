pragma Singleton
import QtQuick

QtObject {
    property url curActivePage: "qrc:/AikariViews/SplashView.qml"

    function push(target) {
        this.curActivePage = target
    }
}
