#include <QQmlExtensionPlugin>
#include <QtQml/QtQml>
#include "analogInController.h"
#include "analogOutController.h"
#include "digitalSlaveModel.h"
#include "gatewayFilterModel.h"
#include "gatewayModel.h"
#include "slaveInfoModel.h"
#include "slaveInfoFilterModel.h"
#include "recordedSignalAnalyzerController.h"
#include "viConfigModel.h"
#include "viConfigService.h"

class EtherCATPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri);
};

void EtherCATPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(qstrcmp(uri, "precitec.gui.components.ethercat") == 0);
    qmlRegisterType<precitec::gui::components::ethercat::AnalogInController>(uri, 1, 0, "AnalogInController");
    qmlRegisterType<precitec::gui::components::ethercat::AnalogOutController>(uri, 1, 0, "AnalogOutController");
    qmlRegisterType<precitec::gui::components::ethercat::DigitalSlaveModel>(uri, 1, 0, "DigitalSlaveModel");
    qmlRegisterType<precitec::gui::components::ethercat::GatewayModel>(uri, 1, 0, "GatewayModel");
    qmlRegisterType<precitec::gui::components::ethercat::GatewayFilterModel>(uri, 1, 0, "GatewayFilterModel");
    qmlRegisterType<precitec::gui::components::ethercat::RecordedSignalAnalyzerController>(uri, 1, 0, "RecordedSignalAnalyzerController");
    qmlRegisterType<precitec::gui::components::ethercat::SlaveInfoFilterModel>(uri, 1, 0, "SlaveInfoFilterModel");
    qmlRegisterType<precitec::gui::components::ethercat::SlaveInfoModel>(uri, 1, 0, "SlaveInfoModel");
    qmlRegisterType<precitec::gui::components::ethercat::ViConfigModel>(uri, 1, 0, "ViConfigModel");
    qmlRegisterType<precitec::gui::components::ethercat::ViConfigService>(uri, 1, 0, "ViConfigService");
}

#include "main.moc"
