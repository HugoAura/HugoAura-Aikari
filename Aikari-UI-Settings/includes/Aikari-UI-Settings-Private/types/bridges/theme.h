#pragma once

#include <QObject>
#include <QQmlEngine>

namespace AikariUI::Settings::Includes::Private::Types::Bridges::Theme
{
    Q_NAMESPACE
    QML_NAMED_ELEMENT(BridgeTypes_Theme)

    /* [TYPE] ENUM CONTAINER */
    struct ForceThemeMode
    {
        Q_GADGET
        QML_NAMED_ELEMENT(BridgeTypes_Theme_ForceThemeMode)
       public:
        enum class Enum
        {
            UNSET,
            LIGHT,
            DARK,
        };
        Q_ENUM(Enum)
    };
}  // namespace AikariUI::Settings::Includes::Private::Types::Bridges::Theme
