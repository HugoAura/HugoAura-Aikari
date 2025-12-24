pragma Singleton
import QtQuick

QtObject {
    property FontLoader sairaLightLoader: FontLoader {
        source: "qrc:/assets/fonts/Saira-Light.ttf"
    }

    property FontLoader sairaRegularLoader: FontLoader {
        source: "qrc:/assets/fonts/Saira-Regular.ttf"
    }

    property FontLoader consolasLoader: FontLoader {
        source: "qrc:/assets/fonts/Consolas.ttf"
    }

    /* property FontLoader fluentIconsLoader: FontLoader {
        source: "qrc:/assets/fonts/IconFont-SegoeFluentIcons.ttf"
    } */
    property QtObject fluentIconsLoader: QtObject {
        property string name: "Microsoft Segoe Fluent Icons"
    }

    property QtObject materialSymbolsLoader: QtObject {
        property string name: "Material Symbols Sharp ExtraLight"
    }
}
