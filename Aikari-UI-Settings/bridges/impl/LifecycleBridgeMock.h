#pragma once

#include <QCoroTask>

#include "bridges/virtual/ILifecycleBridge.h"

namespace AikariUI::Settings::Bridges::Impl::Mock
{
    class LifecycleBridgeMock
        : public AikariUI::Settings::Bridges::Virtual::ILifecycleBridge
    {
        Q_OBJECT;

        QML_SINGLETON;
        QML_NAMED_ELEMENT(BridgesImpl_Lifecycle);

       public:
        explicit LifecycleBridgeMock(QObject* parent = nullptr);

        void establishBackendConnection() override;
        void closeBackendConnection() override;
        void resetBackendConnectionProps() override;

       private:
        QCoro::Task<> _runEstablishBackendConnectionMockSeq();
    };
}  // namespace AikariUI::Settings::Bridges::Impl::Mock
