import QtQuick

Item {
    required property var rootObject
    id: agentSelf
    width: rootObject.width
    height: rootObject.height
    anchors {
        top: rootObject.top
        left: rootObject.left
    }
    property QtObject animState: QtObject
    {
        property bool isAnimRunning: false
        property var curActiveAnim
        property var curAnimCallback
    }

    PageSwipeHorizontal {
        id: mountedPageSwipeHorizontal
        visible: false
    }

    function getAnimIdByString(animationNameStr) {
        switch (animationNameStr) {
            case "PageSwipeHorizontal":
                return mountedPageSwipeHorizontal;
        }
    }

    function startTargetAnimation(animationName, properties, onSwitchPageCallback) {
        if (agentSelf.animState.isAnimRunning) {
            agentSelf.animState.curActiveAnim.reset();
        }
        agentSelf.animState.curActiveAnim = getAnimIdByString(animationName);
        agentSelf.animState.isAnimRunning = true;
        agentSelf.animState.curAnimCallback = onSwitchPageCallback;
        agentSelf.animState.curActiveAnim.start(properties);
    }

    function onAnimCompleted() {
        agentSelf.animState.isAnimRunning = false;
        agentSelf.animState.curAnimCallback();
    }
}
