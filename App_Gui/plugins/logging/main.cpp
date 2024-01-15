#include <QQmlExtensionPlugin>
#include <QtQml/QtQml>

#include "logFilterModel.h"
#include "logModel.h"

class LoggerPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri);
};

void LoggerPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(qstrcmp(uri,"precitec.gui.components.logging") == 0);
    qmlRegisterType<precitec::gui::components::logging::LogFilterModel>("precitec.gui.components.logging", 1, 0, "LogFilterModel");
    qmlRegisterType<precitec::gui::components::logging::LogModel>("precitec.gui.components.logging", 1, 0, "LogModel");
}

#include "main.moc"
