#include <QString>
#include <QDebug>
#include <QQmlContext>

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>

#include <QStyleHints>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);


    QQmlApplicationEngine engine;
    // Information for the engine, where the paths of the plugins and libs are
    const QString wmBaseDir{QString::fromUtf8(qgetenv("WM_BASE_DIR"))};
    engine.addImportPath(wmBaseDir + QStringLiteral("/lib/plugins/qml"));
    engine.addImportPath(QStringLiteral("./plugins/qml"));

    QStringList paths = engine.importPathList();
    // append path to default library
    // we need to append it as with prepand it would be used for things like QtQuickControls and pick up system installed Qt
    paths << QStringLiteral("/usr/lib/x86_64-linux-gnu/qt5/qml");
    engine.setImportPathList(paths);

    engine.load(QUrl(QStringLiteral("qrc:/resources/main.qml")));

    return app.exec();
}
