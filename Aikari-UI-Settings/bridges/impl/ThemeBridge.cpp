#include "./ThemeBridge.h"

#define AIKARI_UI_MODULE_NAME "Backend / Theme Bridge (Mock)"

#include <Aikari-UI-Settings-Private/macros/qLogger.h>

namespace AikariUI::Settings::Bridges::Instance
{
    ThemeBridge::ThemeBridge(QObject* parent) : IThemeBridge(parent)
    {
        Q_LOG_DEBUG << "ThemeBridge is constructed.";
    }

    ThemeBridge::~ThemeBridge()
    {
        Q_LOG_DEBUG << "ThemeBridge has been deconstructed.";
    }

    ThemeBridge* ThemeBridge::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
    {
        Q_LOG_DEBUG << "ThemeBridge's create() is called.";
        Q_UNUSED(qmlEngine);
        Q_UNUSED(jsEngine);
        return ThemeBridge::getInstance();
    }

    ThemeBridge* ThemeBridge::getInstance()
    {
        std::call_once(
            ThemeBridge::_onceFlag,
            []()
            {
                if (ThemeBridge::_self == nullptr)
                {
                    ThemeBridge::_self = new ThemeBridge();
                }
            }
        );
        return ThemeBridge::_self;
    };
}  // namespace AikariUI::Settings::Bridges::Instance
