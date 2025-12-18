import QtQuick
import "../utils/connections.mjs" as AikariUtilsConnections

QtObject {
    required property var currentLoader
    property var animAgentId
    property url curActivePage: "qrc:/AikariViews/SplashView.qml"
    property url pendingActivePage

    function push(target) {
        if (
            currentLoader.item &&
            typeof currentLoader.item.aikariAnimationExit === "string" &&
            currentLoader.item.aikariAnimationExitProps
        ) {
            pendingActivePage = target;
            if (animAgentId) {
                animAgentId.startTargetAnimation(
                    currentLoader.item.aikariAnimationExit,
                    currentLoader.item.aikariAnimationExitProps,
                        () => {
                        curActivePage = pendingActivePage;
                    }
                );
            }
        } else {
            curActivePage = target;
        }
    }
}
