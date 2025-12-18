#pragma once

#include <QCoroTask>
#include <QScopedPointer>

#include "bridges/virtual/ILifecycleBridge.h"

namespace AikariUI::Settings::Bridges::Instance
{
    class LifecycleBridgePrivate;

    class LifecycleBridge
        : public AikariUI::Settings::Bridges::Virtual::ILifecycleBridge
    {
        Q_OBJECT;

        QML_SINGLETON;
        QML_NAMED_ELEMENT(BridgesImpl_Lifecycle);

       public:
        static LifecycleBridge* create(
            QQmlEngine* qmlEngine, QJSEngine* jsEngine
        );
        static LifecycleBridge* getInstance();

        void establishBackendConnection() override;
        void closeBackendConnection() override;
        void resetBackendConnectionProps() override;

        LifecycleBridge(LifecycleBridge const&) = delete;
        LifecycleBridge& operator=(LifecycleBridge const&) = delete;

       protected:
        Q_DECLARE_PRIVATE(LifecycleBridge);

       private:
        explicit LifecycleBridge(QObject* parent = nullptr);
        ~LifecycleBridge() override;

        QScopedPointer<LifecycleBridgePrivate> d_ptr;
        inline static LifecycleBridge* _self = nullptr;
        inline static std::once_flag _onceFlag;
    };
}  // namespace AikariUI::Settings::Bridges::Instance
