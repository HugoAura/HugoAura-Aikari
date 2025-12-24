#include "./qmlFontInit.h"

#define AIKARI_UI_MODULE_NAME "Font Loader"

#include <Aikari-UI-Settings-Private/macros/qLogger.h>
#include <QFontDatabase>

namespace AikariUI::Settings::Infrastructure::QMLFontInit
{
    void initFont(/* QQmlApplicationEngine* qmlEnginePtr */)
    {
        QStringList fontLists = {
            QStringLiteral(
                ":/assets/fonts/IconFont-SegoeFluentIcons-Fixed.ttf"
            ),
            QStringLiteral(
                ":/assets/fonts/"
                "IconFont-MaterialSymbolsSharp-ExtraLight-Fixed.ttf"
            )
        };
        for (const QString& fontPath : fontLists)
        {
            int fontId = QFontDatabase::addApplicationFont(fontPath);
            if (fontId == -1)
            {
                Q_LOG_ERROR << "Failed loading font from: " << fontPath
                            << ", font path invalid.";
                continue;
            }
            QStringList fontFamilies =
                QFontDatabase::applicationFontFamilies(fontId);
#ifdef _DEBUG
            Q_LOG_DEBUG << "Font families for font " << fontId << ": "
                        << fontFamilies;
#endif
            for (const QString& fontFamily : fontFamilies)
            {
                QFont::insertSubstitution(fontFamily, fontFamily);
            }
        }
    };
}  // namespace AikariUI::Settings::Infrastructure::QMLFontInit
