#include <QGuiApplication>
#include <QIcon>
#include <QFont>
#include <QQuickStyle>
#include <QQmlApplicationEngine>

#include "module.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setFont(QStringLiteral("Noto Sans"));

    QIcon::setThemeName(QStringLiteral("precitec"));

    qmlRegisterType<precitec::gui::mockAxis::Module>("Precitec.MockAxis", 1, 0, "Module");

    QQmlApplicationEngine engine;
    QStringList paths = engine.importPathList();
    // append path to default library
    // we need to append it as with prepand it would be used for things like QtQuickControls and pick up system installed Qt
    paths << QStringLiteral("/usr/lib/x86_64-linux-gnu/qt5/qml");
    engine.setImportPathList(paths);

    QQuickStyle::setStyle(QLatin1String("/usr/lib/x86_64-linux-gnu/qt5/qml/precitecqtquickstyle/"));
    QQuickStyle::setFallbackStyle(QString());
    engine.load(QUrl(QStringLiteral("qrc:/resources/qml/main.qml")));

    return app.exec();
}
