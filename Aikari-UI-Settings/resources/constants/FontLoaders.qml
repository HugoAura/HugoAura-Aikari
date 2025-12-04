pragma Singleton
import QtQuick

QtObject {
    property FontLoader sairaLightLoader: FontLoader
    {
        source: "qrc:/assets/fonts/Saira-Light.ttf"
    }

    property FontLoader sairaRegularLoader: FontLoader
    {
        source: "qrc:/assets/fonts/Saira-Regular.ttf"
    }
}
