#include "./LifecycleBridgeMock.h"

#include <Aikari-UI-Settings-Private/macros/qLogger.h>
#include <QCoroTimer>
#include <chrono>
#include <coroutine>

using namespace Qt::Literals::StringLiterals;
using namespace std::chrono_literals;

namespace AikariUI::Settings::Bridges::Impl::Mock
{
    LifecycleBridgeMock::LifecycleBridgeMock(QObject* parent)
        : ILifecycleBridge(parent)
    {
        this->backendConnStatistics = { .connectionProgressPercent = 0,
                                        .curTaskText = u"Now Loading..."_s,
                                        .errorMessage = u""_s,
                                        .backlog = {} };
    }

    QCoro::Task<> LifecycleBridgeMock::_runEstablishBackendConnectionMockSeq()
    {
        this->backendConnStatistics.connectionProgressPercent = 5;
        this->backendConnStatistics.curTaskText = u"Connecting..."_s;
        this->backendConnStatistics.backlog.push_back(
            u"[INFO] Engage backend connection try"_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(200ms);
        this->backendConnStatistics.connectionProgressPercent = 8;
        this->backendConnStatistics.curTaskText = u"读取连接配置..."_s;
        this->backendConnStatistics.backlog.push_back(
            u"[INFO] Accessing registry for gathering backend connection config"_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(125ms);
        this->backendConnStatistics.backlog.push_back(
            u"[SUCCESS] Got connection config"_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(25ms);
        this->backendConnStatistics.connectionProgressPercent = 20;
        this->backendConnStatistics.curTaskText = u"建立命名管道连接..."_s;
        this->backendConnStatistics.backlog.push_back(
            u"[INFO] Trying to establish named pipe connection..."_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(25ms);
        this->backendConnStatistics.connectionProgressPercent = 27;
        this->backendConnStatistics.backlog.push_back(
            u"[INFO] Connecting to pipe://{114514-1919810-ABCDEFG-0D000721-SUDORMRF}"_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(350ms);
        this->backendConnStatistics.connectionProgressPercent = 35;
        this->backendConnStatistics.curTaskText = u"正在验证会话..."_s;
        this->backendConnStatistics.backlog.push_back(u"[INFO] Connected."_s);
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(50ms);
        this->backendConnStatistics.backlog.push_back(
            u"[INFO] Authenticating with one-shot token: 1a2b3c**************************"_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(120ms);
        this->backendConnStatistics.connectionProgressPercent = 50;
        this->backendConnStatistics.curTaskText = u"拉取配置..."_s;
        this->backendConnStatistics.backlog.push_back(
            u"[SUCCESS] Authenticated, got client id: 1"_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(75ms);
        this->backendConnStatistics.connectionProgressPercent = 62;
        this->backendConnStatistics.curTaskText =
            u"正在拉取模块 launcher 配置..."_s;
        this->backendConnStatistics.backlog.push_back(
            u"[INFO] Start config pull task"_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(50ms);
        this->backendConnStatistics.backlog.push_back(
            u"[INFO] Pulling config for module \"launcher\"..."_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(125ms);
        this->backendConnStatistics.backlog.push_back(
            u"[SUCCESS] Pulled config for module \"launcher\""_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(25ms);
        this->backendConnStatistics.connectionProgressPercent = 75;
        this->backendConnStatistics.curTaskText = u"正在拉取模块 pls 配置..."_s;
        this->backendConnStatistics.backlog.push_back(
            u"[INFO] Pulling config for module \"pls\"..."_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(125ms);
        this->backendConnStatistics.backlog.push_back(
            u"[SUCCESS] Pulled config for module \"pls\""_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(50ms);
        this->backendConnStatistics.connectionProgressPercent = 84;
        this->backendConnStatistics.curTaskText = u"初始化分析服务..."_s;
        this->backendConnStatistics.backlog.push_back(
            u"[INFO] Initializing telemetry srv..."_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(150ms);
        this->backendConnStatistics.backlog.push_back(
            u"[SUCCESS] Initialized telemetry srv"_s
        );
        co_await QCoro::sleepFor(25ms);
        this->backendConnStatistics.connectionProgressPercent = 92;
        this->backendConnStatistics.curTaskText = u"准备就绪"_s;
        this->backendConnStatistics.backlog.push_back(
            u"[INFO] Aikari Settings UI is ready."_s
        );
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(50ms);
        this->backendConnStatistics.connectionProgressPercent = 100;
        emit onBackendConnStatisticsChanged();
        co_await QCoro::sleepFor(200ms);
        this->setBackendConnStatus(
            AikariUI::Settings::Includes::Private::Types::Bridges::Lifecycle::
                BackendConnectionStatus::Enum::CONNECTED
        );
    }

    void LifecycleBridgeMock::establishBackendConnection()
    {
        this->setBackendConnStatus(
            AikariUI::Settings::Includes::Private::Types::Bridges::Lifecycle::
                BackendConnectionStatus::Enum::PENDING
        );
        QCoro::connect(
            this->_runEstablishBackendConnectionMockSeq(),
            this,
            [](void)
            {
                Q_LOG_DEBUG << "Mock establishBackendConnection() done.";
            }
        );
    }

    void LifecycleBridgeMock::closeBackendConnection()
    {
    }

    void LifecycleBridgeMock::resetBackendConnectionProps()
    {
    }
}  // namespace AikariUI::Settings::Bridges::Impl::Mock
