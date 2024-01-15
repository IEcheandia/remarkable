#include <QTest>

#include "Poco/Activity.h"
#include "Poco/DateTime.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/Util/PropertyFileConfiguration.h"

#include "../include/viInspectionControl/VI_InspectionControl.h"

#include "event/inspectionCmd.proxy.h"
#include "event/inspection.proxy.h"
#include "event/sensor.proxy.h"
#include "event/ethercatOutputs.proxy.h"
#include "message/db.proxy.h"

#include "common/systemConfiguration.h"

using namespace precitec::interface;

class VI_InspectionControlTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();

private:
    QTemporaryDir m_dir;
};

void VI_InspectionControlTest::initTestCase()
{
    QVERIFY(m_dir.isValid());

    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());

    QDir{}.mkpath(m_dir.filePath(QStringLiteral("config")));

    const auto pathToSystemConfig = QFINDTESTDATA("../../wm_inst/config_templates/track_compact/SystemConfig.xml");
    QVERIFY(QFile::copy(pathToSystemConfig, m_dir.filePath(QStringLiteral("config")) + QStringLiteral("/SystemConfig.xml")));
}

void VI_InspectionControlTest::testCtor()
{
    TInspection<EventProxy>         inspectProxy_;
    TInspectionCmd<EventProxy>      inspectCmdProxy_;
    TSensor<EventProxy>             m_oSensorProxy;
    TEthercatOutputs<EventProxy>    m_oEthercatOutputsProxy;
    TS6K_InfoToProcesses<EventProxy> m_oS6K_InfoToProcessesProxy;
    TDb<MsgProxy> m_oDbProxy;

    precitec::ethercat::VI_InspectionControl oVI_InspectionControl(inspectProxy_, inspectCmdProxy_, m_oSensorProxy, m_oEthercatOutputsProxy, m_oS6K_InfoToProcessesProxy, m_oDbProxy);
    QCOMPARE(oVI_InspectionControl.m_oSumErrorLatchedAccu, std::atomic<bool>(false));
}

QTEST_GUILESS_MAIN(VI_InspectionControlTest)
#include "VI_InspectionControlTest.moc"
