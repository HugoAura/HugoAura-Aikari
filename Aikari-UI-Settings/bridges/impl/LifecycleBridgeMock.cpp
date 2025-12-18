#include "./LifecycleBridge.h"

#define AIKARI_UI_MODULE_NAME "Backend / Lifecycle Bridge (Mock)"

#include <Aikari-UI-Settings-Private/macros/qLogger.h>
#include <QCoroTimer>
#include <chrono>
#include <coroutine>

using namespace Qt::Literals::StringLiterals;
using namespace std::chrono_literals;

namespace AikariUI::Settings::Bridges::Instance
{
    class LifecycleBridgePrivate
    {
       public:
        Q_DECLARE_PUBLIC(LifecycleBridge);

       private:
        QCoro::Task<> _runEstablishBackendConnectionMockSeq()
        {
            auto* parentThis = q_func();
            parentThis->backendConnStatistics.connectionProgressPercent = 5;
            parentThis->backendConnStatistics.curTaskText = u"Connecting..."_s;
            parentThis->backendConnStatistics.backlog.push_back(
                u"[INFO] Engage backend connection try"_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(200ms);
            parentThis->backendConnStatistics.connectionProgressPercent = 8;
            parentThis->backendConnStatistics.curTaskText =
                u"读取连接配置..."_s;
            parentThis->backendConnStatistics.backlog.push_back(
                u"[INFO] Accessing registry for gathering backend connection config"_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(125ms);
            parentThis->backendConnStatistics.backlog.push_back(
                u"[SUCCESS] Got connection config"_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(25ms);
            parentThis->backendConnStatistics.connectionProgressPercent = 20;
            parentThis->backendConnStatistics.curTaskText =
                u"建立命名管道连接..."_s;
            parentThis->backendConnStatistics.backlog.push_back(
                u"[INFO] Trying to establish named pipe connection..."_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(25ms);
            parentThis->backendConnStatistics.connectionProgressPercent = 27;
            parentThis->backendConnStatistics.backlog.push_back(
                u"[INFO] Connecting to pipe://{114514-1919810-ABCDEFG-0D000721-SUDORMRF}"_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(350ms);
            parentThis->backendConnStatistics.connectionProgressPercent = 35;
            parentThis->backendConnStatistics.curTaskText =
                u"正在验证会话..."_s;
            parentThis->backendConnStatistics.backlog.push_back(
                u"[INFO] Connected."_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(50ms);
            parentThis->backendConnStatistics.backlog.push_back(
                u"[INFO] Authenticating with one-shot token: 1a2b3c**************************"_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(120ms);
            parentThis->backendConnStatistics.connectionProgressPercent = 50;
            parentThis->backendConnStatistics.curTaskText = u"拉取配置..."_s;
            parentThis->backendConnStatistics.backlog.push_back(
                u"[SUCCESS] Authenticated, got client id: 1"_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(75ms);
            parentThis->backendConnStatistics.connectionProgressPercent = 62;
            parentThis->backendConnStatistics.curTaskText =
                u"正在拉取模块 launcher 配置..."_s;
            parentThis->backendConnStatistics.backlog.push_back(
                u"[INFO] Start config pull task"_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(50ms);
            parentThis->backendConnStatistics.backlog.push_back(
                u"[INFO] Pulling config for module \"launcher\"..."_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(125ms);
            parentThis->backendConnStatistics.backlog.push_back(
                u"[SUCCESS] Pulled config for module \"launcher\""_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(25ms);
            parentThis->backendConnStatistics.connectionProgressPercent = 75;
            parentThis->backendConnStatistics.curTaskText =
                u"正在拉取模块 pls 配置..."_s;
            parentThis->backendConnStatistics.backlog.push_back(
                u"[INFO] Pulling config for module \"pls\"..."_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(125ms);
            parentThis->backendConnStatistics.backlog.push_back(
                u"[SUCCESS] Pulled config for module \"pls\""_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(50ms);
            parentThis->backendConnStatistics.connectionProgressPercent = 84;
            parentThis->backendConnStatistics.curTaskText =
                u"初始化分析服务..."_s;
            parentThis->backendConnStatistics.backlog.push_back(
                u"[INFO] Initializing telemetry srv..."_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(150ms);
            parentThis->backendConnStatistics.backlog.push_back(
                u"[SUCCESS] Initialized telemetry srv"_s
            );
            co_await QCoro::sleepFor(25ms);
            parentThis->backendConnStatistics.connectionProgressPercent = 92;
            parentThis->backendConnStatistics.curTaskText = u"准备就绪"_s;
            parentThis->backendConnStatistics.backlog.push_back(
                u"[INFO] Aikari Settings UI is ready."_s
            );
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(50ms);
            parentThis->backendConnStatistics.connectionProgressPercent = 100;
            emit parentThis->onBackendConnStatisticsChanged();
            co_await QCoro::sleepFor(200ms);
            parentThis->setBackendConnStatus(
                AikariUI::Settings::Includes::Private::Types::Bridges::
                    Lifecycle::BackendConnectionStatus::Enum::CONNECTED
            );
        };

        LifecycleBridge* q_ptr;
    };

    LifecycleBridge::LifecycleBridge(QObject* parent)
        : ILifecycleBridge(parent), d_ptr(new LifecycleBridgePrivate)
    {
        d_ptr->q_ptr = this;
        this->backendConnStatistics = { .connectionProgressPercent = 0,
                                        .curTaskText = u"Now Loading..."_s,
                                        .errorMessage = u""_s,
                                        .backlog = {} };
        Q_LOG_DEBUG << "LifecycleBridge is constructed.";
    }

    LifecycleBridge::~LifecycleBridge()
    {
        Q_LOG_DEBUG << "LifecycleBridge has been deconstructed.";
    }

    LifecycleBridge* LifecycleBridge::create(
        QQmlEngine* qmlEngine, QJSEngine* jsEngine
    )
    {
        Q_LOG_DEBUG << "LifecycleBridge's create() is called.";
        Q_UNUSED(qmlEngine);
        Q_UNUSED(jsEngine);
        return LifecycleBridge::getInstance();
    }

    LifecycleBridge* LifecycleBridge::getInstance()
    {
        std::call_once(
            LifecycleBridge::_onceFlag,
            []()
            {
                if (LifecycleBridge::_self == nullptr)
                {
                    LifecycleBridge::_self = new LifecycleBridge();
                }
            }
        );
        return LifecycleBridge::_self;
    };

    void LifecycleBridge::establishBackendConnection()
    {
        auto* privImpl = d_func();
        this->setBackendConnStatus(
            AikariUI::Settings::Includes::Private::Types::Bridges::Lifecycle::
                BackendConnectionStatus::Enum::PENDING
        );
        QCoro::connect(
            privImpl->_runEstablishBackendConnectionMockSeq(),
            this,
            [](void)
            {
                Q_LOG_DEBUG << "Mock establishBackendConnection() done.";
            }
        );
    }

    void LifecycleBridge::closeBackendConnection()
    {
    }

    void LifecycleBridge::resetBackendConnectionProps()
    {
    }
}  // namespace AikariUI::Settings::Bridges::Instance
