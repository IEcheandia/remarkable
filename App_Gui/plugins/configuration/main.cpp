#include <QQmlExtensionPlugin>
#include <QQmlEngine>

#include "etherCATConfigurationController.h"
#include "hardwareConfigurationBackupModel.h"
#include "shutdownChangeEntry.h"
#include "shutdownService.h"
#include "systemHardwareBackupHelper.h"
#include "upsModel.h"

#include <precitec/userLog.h>

class GeneralPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;
};

void GeneralPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<precitec::gui::HardwareConfigurationBackupModel>(uri, 1, 0, "HardwareConfigurationBackupModel");
    qmlRegisterType<precitec::gui::ShutdownService>(uri, 1, 0, "ShutdownService");
    qmlRegisterType<precitec::gui::SystemHardwareBackupHelper>(uri, 1, 0, "SystemHardwareBackupHelper");
    qmlRegisterType<precitec::gui::EtherCATConfigurationController>(uri, 1, 0, "EtherCATConfigurationController");
    qmlRegisterType<precitec::gui::UpsModel>(uri, 1, 0, "UpsModel");
    precitec::gui::components::userLog::UserLog::instance()->registerChange(precitec::gui::ShutdownChangeEntry::staticMetaObject);
}

#include "main.moc"
