#include <QQmlExtensionPlugin>
#include <QtQml/QtQml>

#include "availableTasksModel.h"
#include "availableTriggersModel.h"
#include "backupLocalDirectoryConfigurationManager.h"
#include "cronConfigurationManager.h"
#include "deleteBackupsConfigurationManager.h"
#include "schedulerModel.h"
#include "transferDirectoryConfigurationManager.h"
#include "triggerFilterByTaskModel.h"
#include "pathValidator.h"
#include "optionalPortValidator.h"

class SchedulerPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri);
};

void SchedulerPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(qstrcmp(uri, "precitec.gui.components.scheduler") == 0);
    qmlRegisterType<precitec::gui::components::scheduler::AvailableTasksModel>(uri, 1, 0, "AvailableTasksModel");
    qmlRegisterType<precitec::gui::components::scheduler::AvailableTriggersModel>(uri, 1, 0, "AvailableTriggersModel");
    qmlRegisterType<precitec::gui::components::scheduler::BackupLocalDirectoryConfigurationManager>(uri, 1, 0, "BackupLocalDirectoryConfigurationManager");
    qmlRegisterType<precitec::gui::components::scheduler::CronConfigurationManager>(uri, 1, 0, "CronConfigurationManager");
    qmlRegisterType<precitec::gui::components::scheduler::SchedulerModel>(uri, 1, 0, "SchedulerModel");
    qmlRegisterType<precitec::gui::components::scheduler::TransferDirectoryConfigurationManager>(uri, 1, 0, "TransferDirectoryConfigurationManager");
    qmlRegisterType<precitec::gui::components::scheduler::DeleteBackupsConfigurationManager>(uri, 1, 0, "DeleteBackupsConfigurationManager");
    qmlRegisterType<precitec::gui::components::scheduler::TriggerFilterByTaskModel>(uri, 1, 0, "TriggerFilterByTaskModel");
    qmlRegisterType<precitec::gui::components::scheduler::PathValidator>(uri, 1, 0, "PathValidator");
    qmlRegisterType<precitec::gui::components::scheduler::OptionalPortValidator>(uri, 1, 0, "OptionalPortValidator");
}

#include "main.moc"
