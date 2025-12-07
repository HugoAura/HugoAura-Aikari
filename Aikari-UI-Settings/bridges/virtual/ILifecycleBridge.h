#pragma once
#include <Aikari-UI-Settings-Private/types/bridges/lifecycle.h>
#include <QObject>

namespace AikariUI::Settings::Bridges::Virtual
{
    class ILifecycleBridge : public QObject
    {
        Q_OBJECT;

        Q_PROPERTY(
            AikariUI::Settings::Includes::Private::Types::Bridges::Lifecycle::
                BackendConnectionStatus::Enum backendConnStatus READ
                    getBackendConnStatus WRITE setBackendConnStatus NOTIFY
                        onBackendConnStatusChanged
        );
        Q_PROPERTY(
            AikariUI::Settings::Includes::Private::Types::Bridges::Lifecycle::
                BackendConnectionStatistics backendConnStatistics READ
                    getBackendConnStatistics WRITE setBackendConnStatistics
                        NOTIFY onBackendConnStatisticsChanged
        );

       public:
        explicit ILifecycleBridge(QObject* parent = nullptr)
            : QObject(parent) {};

        Q_INVOKABLE virtual void establishBackendConnection() = 0;
        Q_INVOKABLE virtual void closeBackendConnection() = 0;

        Q_INVOKABLE virtual void resetBackendConnectionProps() = 0;

        // >>> BEGIN OF Q_PROPERTY GETTER / SETTER FNS >>> //

        virtual AikariUI::Settings::Includes::Private::Types::Bridges::
            Lifecycle::BackendConnectionStatus::Enum
            getBackendConnStatus()
        {
            return this->backendConnStatus;
        };

        virtual void setBackendConnStatus(
            const AikariUI::Settings::Includes::Private::Types::Bridges::
                Lifecycle::BackendConnectionStatus::Enum& newVal
        )
        {
            if (newVal == this->backendConnStatus)
                return;
            this->backendConnStatus = newVal;
            emit onBackendConnStatusChanged();
        };

        virtual const AikariUI::Settings::Includes::Private::Types::Bridges::
            Lifecycle::BackendConnectionStatistics&
            getBackendConnStatistics()
        {
            return this->backendConnStatistics;
        };

        virtual void setBackendConnStatistics(
            const AikariUI::Settings::Includes::Private::Types::Bridges::
                Lifecycle::BackendConnectionStatistics& newVal
        )
        {
            if (newVal == this->backendConnStatistics)
                return;
            this->backendConnStatistics = newVal;
            emit onBackendConnStatisticsChanged();
        };

        // <<< END OF Q_PROPERTY GETTER / SETTER FNS <<< //

       signals:
        void onBackendConnStatusChanged();
        void onBackendConnStatisticsChanged();

       protected:
        AikariUI::Settings::Includes::Private::Types::Bridges::Lifecycle::
            BackendConnectionStatus::Enum backendConnStatus =
                AikariUI::Settings::Includes::Private::Types::Bridges::
                    Lifecycle::BackendConnectionStatus::Enum::NOT_CONNECTED;

        AikariUI::Settings::Includes::Private::Types::Bridges::Lifecycle::
            BackendConnectionStatistics backendConnStatistics;
    };
}  // namespace AikariUI::Settings::Bridges::Virtual
