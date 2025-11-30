import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import AikariConstants as AikariConstants

Window {
    id: mainWindowRoot
    width: 1400
    height: 800
    visible: true
    title: qsTr("Aikari")
    color: AikariConstants.ThemeStyle.defaultStyle.background
}
