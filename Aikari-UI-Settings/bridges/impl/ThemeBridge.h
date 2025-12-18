#pragma once

#include "bridges/virtual/IThemeBridge.h"

namespace AikariUI::Settings::Bridges::Instance
{
    class ThemeBridge
        : public AikariUI::Settings::Bridges::Virtual::IThemeBridge
    {
        Q_OBJECT;

        QML_SINGLETON;
        QML_NAMED_ELEMENT(BridgesImpl_Theme);

       public:
        static ThemeBridge* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);
        static ThemeBridge* getInstance();

        ThemeBridge(ThemeBridge const&) = delete;
        ThemeBridge& operator=(ThemeBridge const&) = delete;

       private:
        explicit ThemeBridge(QObject* parent = nullptr);
        ~ThemeBridge() override;

        inline static ThemeBridge* _self = nullptr;
        inline static std::once_flag _onceFlag;
    };
}  // namespace AikariUI::Settings::Bridges::Instance
