#pragma once

#include <QObject>
#include <QQmlEngine>

namespace AikariUI::Settings::Includes::Private::Types::Bridges::Lifecycle
{
    Q_NAMESPACE
    QML_NAMED_ELEMENT(BridgeTypes_Lifecycle)

    /* [TYPE] ENUM CONTAINER */
    struct BackendConnectionStatus
    {
        Q_GADGET
        QML_NAMED_ELEMENT(BridgeTypes_Lifecycle_BackendConnectionStatus)
       public:
        enum class Enum
        {
            NOT_CONNECTED,
            FAILED,
            PENDING,
            CONNECTED
        };
        Q_ENUM(Enum)
    };

    /* [TYPE] CXX STRUCT */
    struct BackendConnectionStatistics
    {
        Q_GADGET
        QML_VALUE_TYPE(BridgeTypes_Lifecycle_BackendConnectionStatistics)

        Q_PROPERTY(
            int connectionProgressPercent MEMBER connectionProgressPercent
        )
        Q_PROPERTY(QString curTaskText MEMBER curTaskText)
        Q_PROPERTY(QString errorMessage MEMBER errorMessage)
        Q_PROPERTY(QVector<QString> backlog MEMBER backlog)
       public:
        int connectionProgressPercent;
        QString curTaskText;
        QString errorMessage;
        QVector<QString> backlog;

        bool operator==(const BackendConnectionStatistics& other) const =
            default;
        bool operator!=(const BackendConnectionStatistics& other) const =
            default;
    };
}  // namespace AikariUI::Settings::Includes::Private::Types::Bridges::Lifecycle
