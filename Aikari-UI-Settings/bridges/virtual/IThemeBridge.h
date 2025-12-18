#pragma once
#include <Aikari-UI-Settings-Private/types/bridges/theme.h>
#include <QObject>

namespace AikariUI::Settings::Bridges::Virtual
{
    class IThemeBridge : public QObject
    {
        Q_OBJECT;

        Q_PROPERTY(
            AikariUI::Settings::Includes::Private::Types::Bridges::Theme::
                ForceThemeMode::Enum forceThemeMode READ getForceThemeMode
                    NOTIFY onForceThemeModeChanged
        );

       public:
        explicit IThemeBridge(QObject* parent = nullptr) : QObject(parent) {};

        virtual AikariUI::Settings::Includes::Private::Types::Bridges::Theme::
            ForceThemeMode::Enum
            getForceThemeMode()
        {
            return this->forceThemeMode;
        };

        virtual void setForceThemeMode(
            AikariUI::Settings::Includes::Private::Types::Bridges::Theme::
                ForceThemeMode::Enum targetThemeMode
        )
        {
            this->forceThemeMode = targetThemeMode;
            emit onForceThemeModeChanged();
        };

       signals:
        void onForceThemeModeChanged();

       protected:
        AikariUI::Settings::Includes::Private::Types::Bridges::Theme::
            ForceThemeMode::Enum forceThemeMode = AikariUI::Settings::Includes::
                Private::Types::Bridges::Theme::ForceThemeMode::Enum::UNSET;
    };
}  // namespace AikariUI::Settings::Bridges::Virtual
