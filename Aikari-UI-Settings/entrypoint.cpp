#define AIKARI_UI_MODULE_NAME "Main"

#include <Aikari-UI-Settings-Private/macros/qLogger.h>
#include <Aikari-UI-Settings-Public/constants/entrypoint.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QWKQuick/qwkquickglobal.h>
#include <QtQuick/QQuickWindow>

#include "./infrastructure/cliParse.h"

int main(int argCount, char* argVec[])
{
    /*
    while (!IsDebuggerPresent())
    {
        Sleep(1000);
    }
    */
#ifdef _DEBUG
    qputenv("QT_WIN_DEBUG_CONSOLE", "attach");
#endif
    auto cliOpts = AikariUI::Settings::Infrastructure::CliParse::parseCliOpts(
        argCount, argVec
    );
    if (cliOpts.fastFail)
        return 0;

    QGuiApplication globalQtApp(argCount, argVec);

    QQuickWindow::setDefaultAlphaBuffer(true);

    QQmlApplicationEngine globalQmlEngine;

    globalQmlEngine.addImportPath(
        QCoreApplication::applicationDirPath() + "/qml"
    );
    globalQmlEngine.addImportPath("qrc:/");

    Q_LOG_DEBUG << "Import Paths:" << globalQmlEngine.importPathList();

    QWK::registerTypes(&globalQmlEngine);

    const QUrl mainWinUrl(QStringLiteral("qrc:/windows/MainWindow.qml"));
    QObject::connect(
        &globalQmlEngine,
        &QQmlApplicationEngine::objectCreated,
        &globalQtApp,
        [mainWinUrl](QObject* obj, const QUrl& objUrl)
        {
            if (!obj && (objUrl == mainWinUrl))
            {
                QCoreApplication::exit(
                    AikariUI::Settings::Includes::Public::Constants::
                        Entrypoint::EXIT_CODES::MAIN_QML_FAILED_TO_LOAD
                );  // ec: -1
            }
        },
        Qt::QueuedConnection
    );

    globalQmlEngine.load(mainWinUrl);

    return QGuiApplication::exec();
};
