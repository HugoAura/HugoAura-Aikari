import QtQuick

Rectangle {
    id: pBarContainer
    property alias fillColor: pBarInner.color
    property alias backgroundColor: pBarContainer.color
    height: 2
    property real percent: 0

    Rectangle {
        id: pBarInner
        height: parent.height
        width: pBarContainer.width * (pBarContainer.percent / 100)

        Behavior on width {
            SmoothedAnimation {
                velocity: 330
                reversingMode: SmoothedAnimation.Eased
            }
        }
    }
}
