#include <QTest>
#include <QSignalSpy>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrentRun>
#include <QTimer>

#include "../viConfigModel.h"
#include "../viConfigService.h"
#include "../../../src/serviceToGuiServer.h"

#include <precitec/dataSet.h>

using precitec::gui::components::ethercat::ViConfigModel;
using precitec::gui::components::ethercat::ViConfigService;
using precitec::gui::components::plotter::DataSet;
using precitec::gui::ServiceToGuiServer;

class ViConfigModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testSetViConfig();
    void testDefaultDataSetColor();
    void testToggleEnabled();
    void testSetColor();
    void testSetService();
    void testModelData();
    void testModelDataSouvis6000();
    void testSetStorageDir();
    void testRecording();
};

void ViConfigModelTest::testCtor()
{
    ViConfigModel model;
    QVERIFY(model.viConfig() == nullptr);
    QVERIFY(model.service() == nullptr);
    QCOMPARE(model.defaultDataSetColor(), Qt::black);
    QCOMPARE(model.dataSets().isEmpty(), true);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.isRecording(), false);
    QCOMPARE(model.isProcessing(), false);
    QCOMPARE(model.storageDir(), QString());
    QCOMPARE(model.areItemsEnabled(), false);
}

void ViConfigModelTest::testRoleNames()
{
    ViConfigModel model;
    auto roles = model.roleNames();
    QCOMPARE(roles.size(), 3);
    QCOMPARE(roles[Qt::DisplayRole], QByteArrayLiteral("name"));
    QCOMPARE(roles[Qt::UserRole], QByteArrayLiteral("enabled"));
    QCOMPARE(roles[Qt::UserRole + 1], QByteArrayLiteral("color"));
}

void ViConfigModelTest::testSetViConfig()
{
    ViConfigModel model;
    QSignalSpy modelResetSpy{&model, &ViConfigModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QSignalSpy viConfigServiceChangedSpy{&model, &ViConfigModel::viConfigChanged};
    QVERIFY(viConfigServiceChangedSpy.isValid());

    auto service = std::make_unique<ViConfigService>();
    model.setViConfig(service.get());
    QCOMPARE(model.viConfig(), service.get());
    QCOMPARE(viConfigServiceChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 0);

    // setting same should not change
    model.setViConfig(service.get());
    QCOMPARE(viConfigServiceChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 0);

    // but setting the test data should emit modelReset
    QString path = QFINDTESTDATA("../../../../wm_inst/config_templates");
    QVERIFY(!path.isEmpty());
    service->setConfigurationDir(path);
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);
    QVERIFY(model.rowCount() != 0);

    for (int i = 0; i < model.rowCount(); i++)
    {
        QCOMPARE(model.index(i, 0).data(Qt::UserRole).toBool(), false);
        QCOMPARE(model.index(i, 0).data(Qt::UserRole + 1).value<QColor>(), Qt::black);
    }

    // unset ViConfigService
    service.reset();
    QCOMPARE(modelResetSpy.count(), 2);
    QCOMPARE(model.rowCount(), 0);
}

void ViConfigModelTest::testDefaultDataSetColor()
{
    ViConfigModel model;
    QSignalSpy modelResetSpy{&model, &ViConfigModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    ViConfigService viConfig;
    QString path = QFINDTESTDATA("../../../../wm_inst/config_templates");
    QVERIFY(!path.isEmpty());
    viConfig.setConfigurationDir(path);

    model.setViConfig(&viConfig);
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);
    QVERIFY(model.rowCount() != 0);

    for (int i = 0; i < model.rowCount(); i++)
    {
        QCOMPARE(model.index(i, 0).data(Qt::UserRole + 1).value<QColor>(), Qt::black);
    }
    // change to other color
    QSignalSpy dataChangedSpy{&model, &QAbstractItemModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());
    QSignalSpy defaultColorChangedSpy{&model, &ViConfigModel::defaultDataSetColorChanged};
    QVERIFY(defaultColorChangedSpy.isValid());

    // change color
    QCOMPARE(model.defaultDataSetColor(), Qt::black);
    model.setDefaultDataSetColor(Qt::red);
    QCOMPARE(model.defaultDataSetColor(), Qt::red);
    QCOMPARE(defaultColorChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), model.index(model.rowCount() -1, 0));
    QCOMPARE(dataChangedSpy.first().at(2).value<QVector<int>>(), QVector<int>() << Qt::UserRole +1);

    for (int i = 0; i < model.rowCount(); i++)
    {
        QCOMPARE(model.index(i, 0).data(Qt::UserRole + 1).value<QColor>(), Qt::red);
    }

    // setting same color again shouldn't change
    model.setDefaultDataSetColor(Qt::red);
    QCOMPARE(model.defaultDataSetColor(), Qt::red);
    QCOMPARE(defaultColorChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);
}

void ViConfigModelTest::testToggleEnabled()
{
    ViConfigModel model;
    QSignalSpy modelResetSpy{&model, &ViConfigModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    ViConfigService viConfig;
    QString path = QFINDTESTDATA("../../../../wm_inst/config_templates");
    QVERIFY(!path.isEmpty());
    viConfig.setConfigurationDir(path);

    model.setViConfig(&viConfig);
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);
    QVERIFY(model.rowCount() != 0);

    QSignalSpy dataChangedSpy{&model, &QAbstractItemModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());
    QSignalSpy dataSetsChangedSpy{&model, &ViConfigModel::dataSetsChanged};
    QVERIFY(dataSetsChangedSpy.isValid());
    QCOMPARE(model.areItemsEnabled(), false);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toBool(), false);
    model.toggleEnabled(model.index(0, 0));
    QCOMPARE(model.areItemsEnabled(), true);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.first().at(2).value<QVector<int>>(), QVector<int>() << Qt::UserRole);
    QCOMPARE(dataSetsChangedSpy.count(), 1);

    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toBool(), true);
    auto datasets = model.dataSets();
    QCOMPARE(datasets.count(), 1);
    auto dataset = datasets.first().value<DataSet*>();
    QCOMPARE(dataset->name(), model.index(0, 0).data());

    // toggle again
    QSignalSpy destroyedSpy{dataset, &QObject::destroyed};
    QVERIFY(destroyedSpy.isValid());
    model.toggleEnabled(model.index(0, 0));
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.last().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.last().at(1).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.last().at(2).value<QVector<int>>(), QVector<int>() << Qt::UserRole);
    QCOMPARE(dataSetsChangedSpy.count(), 2);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toBool(), false);
    QCOMPARE(model.dataSets().count(), 0);
    QCOMPARE(model.areItemsEnabled(), false);

    QVERIFY(destroyedSpy.wait());

    // and once more
    model.toggleEnabled(model.index(0, 0));
    QCOMPARE(dataChangedSpy.count(), 3);
    QCOMPARE(model.dataSets().count(), 1);
    dataset = model.dataSets().first().value<DataSet*>();
    QVERIFY(dataset);
    QCOMPARE(dataSetsChangedSpy.count(), 3);

    // when unsetting the viconfig it should result in the DataSet getting destroyed
    QSignalSpy destroyedSpy2{dataset, &QObject::destroyed};
    QVERIFY(destroyedSpy2.isValid());
    model.setViConfig(nullptr);
    QVERIFY(destroyedSpy2.wait());
    QCOMPARE(dataSetsChangedSpy.count(), 4);
}

void ViConfigModelTest::testSetColor()
{
    ViConfigModel model;
    QSignalSpy modelResetSpy{&model, &ViConfigModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    ViConfigService viConfig;
    QString path = QFINDTESTDATA("../../../../wm_inst/config_templates");
    QVERIFY(!path.isEmpty());
    viConfig.setConfigurationDir(path);

    model.setViConfig(&viConfig);
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);
    QVERIFY(model.rowCount() != 0);

    QSignalSpy dataChangedSpy{&model, &QAbstractItemModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());
    // no DataSet yet
    model.setColor(model.index(0, 0), Qt::red);
    QVERIFY(dataChangedSpy.isEmpty());
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 1).value<QColor>(), Qt::black);

    model.toggleEnabled(model.index(0, 0));
    QCOMPARE(dataChangedSpy.count(), 1);
    model.setColor(model.index(0, 0), Qt::red);
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.last().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.last().at(1).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.last().at(2).value<QVector<int>>(), QVector<int>() << Qt::UserRole+1);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 1).value<QColor>(), Qt::red);
}

void ViConfigModelTest::testSetService()
{
    ViConfigModel model;
    QVERIFY(model.service() == nullptr);
    QSignalSpy serviceChangedSpy{&model, &ViConfigModel::serviceChanged};
    QVERIFY(serviceChangedSpy.isValid());

    auto service = std::make_unique<ServiceToGuiServer>(precitec::ServiceFromGuiProxy{});
    model.setService(service.get());
    QCOMPARE(model.service(), service.get());
    QCOMPARE(serviceChangedSpy.count(), 1);

    // setting again should not change
    model.setService(service.get());
    QCOMPARE(serviceChangedSpy.count(), 1);

    // but deleting should change
    service.reset();
    QCOMPARE(serviceChangedSpy.count(), 2);
    QVERIFY(model.service() == nullptr);
}

void ViConfigModelTest::testModelData()
{
    ViConfigModel model;
    QSignalSpy modelResetSpy{&model, &ViConfigModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    ViConfigService viConfig;
    QString path = QFINDTESTDATA("testdata");
    QVERIFY(!path.isEmpty());
    viConfig.setConfigurationDir(path);

    model.setViConfig(&viConfig);
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.rowCount(), 7);

    QCOMPARE(model.index(0, 0).data().toString(), QStringLiteral("Part OK (OK=1)"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+2).value<ViConfigService::SignalType>(), ViConfigService::SignalType::Output);
    QCOMPARE(model.index(1, 0).data().toString(), QStringLiteral("Part No"));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole+2).value<ViConfigService::SignalType>(), ViConfigService::SignalType::Output);

    QCOMPARE(model.index(2, 0).data().toString(), QStringLiteral("Inspection Cycle Active"));
    QCOMPARE(model.index(2, 0).data(Qt::UserRole+2).value<ViConfigService::SignalType>(), ViConfigService::SignalType::Input);
    QCOMPARE(model.index(3, 0).data().toString(), QStringLiteral("Take-Over Seam Series"));
    QCOMPARE(model.index(3, 0).data(Qt::UserRole+2).value<ViConfigService::SignalType>(), ViConfigService::SignalType::Input);
    QCOMPARE(model.index(4, 0).data().toString(), QStringLiteral("Option: input value"));
    QCOMPARE(model.index(4, 0).data(Qt::UserRole+2).value<ViConfigService::SignalType>(), ViConfigService::SignalType::Input);
    QCOMPARE(model.index(5, 0).data().toString(), QStringLiteral("Seam Number"));
    QCOMPARE(model.index(5, 0).data(Qt::UserRole+2).value<ViConfigService::SignalType>(), ViConfigService::SignalType::Input);
    QCOMPARE(model.index(6, 0).data().toString(), QStringLiteral("Calibration Finished"));
    QCOMPARE(model.index(6, 0).data(Qt::UserRole+2).value<ViConfigService::SignalType>(), ViConfigService::SignalType::Output);

    // let's enable DataSets for all
    for (int i = 0; i < 5; i++)
    {
        model.toggleEnabled(model.index(i, 0));
    }
    model.toggleEnabled(model.index(6, 0));
    auto dataSets = model.dataSets();
    QCOMPARE(dataSets.count(), 6);
    QCOMPARE(dataSets.at(0).value<DataSet*>()->name(), QStringLiteral("Part OK (OK=1)"));
    QCOMPARE(dataSets.at(1).value<DataSet*>()->name(), QStringLiteral("Part No"));
    QCOMPARE(dataSets.at(2).value<DataSet*>()->name(), QStringLiteral("Inspection Cycle Active"));
    QCOMPARE(dataSets.at(3).value<DataSet*>()->name(), QStringLiteral("Take-Over Seam Series"));
    QCOMPARE(dataSets.at(4).value<DataSet*>()->name(), QStringLiteral("Option: input value"));
    QCOMPARE(dataSets.at(5).value<DataSet*>()->name(), QStringLiteral("Calibration Finished"));

    // add service
    ServiceToGuiServer service{{}};
    model.setService(&service);
    EC_T_GET_SLAVE_INFO infos[2];
    infos[0].dwVendorId = 27;
    infos[0].dwProductCode = 61;
    infos[0].wCfgPhyAddress = 0;
    infos[0].dwPdSizeIn = 20 * 8;
    infos[0].dwPdSizeOut = 20 * 8;
    infos[0].dwPdOffsIn = 0;
    infos[0].dwPdOffsOut = 20 * 8;
    infos[1].dwVendorId = 27;
    infos[1].dwProductCode = 61;
    infos[1].wCfgPhyAddress = 30;
    infos[1].dwPdSizeIn = 20 * 8;
    infos[1].dwPdSizeOut = 20 * 8;
    infos[1].dwPdOffsIn = 40 * 8;
    infos[1].dwPdOffsOut = 60 * 8;
    precitec::interface::SlaveInfo info{2};
    info.FillBuffer(infos);
    QSignalSpy slaveInfoChangedSpy{&service, &ServiceToGuiServer::slaveInfoChanged};
    QVERIFY(slaveInfoChangedSpy.isValid());
    QtConcurrent::run([&] {service.SlaveInfoECAT(2, info);});
    QVERIFY(slaveInfoChangedSpy.wait());

    // now send some data
    QSignalSpy dataSetChanged{dataSets.first().value<DataSet*>(), &DataSet::samplesChanged};
    QByteArray overallData{80, 0};
    overallData.data()[0] = 3;
    overallData.data()[20] = 2;
    overallData.data()[27] = 0xA;
    reinterpret_cast<uint32_t*>(overallData.data())[8] = 123456789;
    reinterpret_cast<uint16_t*>(overallData.data())[24] = 1234;
    std::vector<precitec::interface::ProcessData> processData{{80, overallData.constData()}};
    QtConcurrent::run([&] {service.ProcessImage(processData, processData);});
    QVERIFY(dataSetChanged.wait());

    QCOMPARE(dataSets.at(0).value<DataSet*>()->sampleCount(), 1);
    auto samples = dataSets.at(0).value<DataSet*>()->samples();
    QCOMPARE(samples.front(), QVector2D(0, 1));
    samples = dataSets.at(1).value<DataSet*>()->samples();
    QCOMPARE(samples.front(), QVector2D(0, 123456789));
    samples = dataSets.at(2).value<DataSet*>()->samples();
    QCOMPARE(samples.front(), QVector2D(0, 1));
    samples = dataSets.at(3).value<DataSet*>()->samples();
    QCOMPARE(samples.front(), QVector2D(0, 1));
    samples = dataSets.at(4).value<DataSet*>()->samples();
    QCOMPARE(samples.front(), QVector2D(0, 1234));
    samples = dataSets.at(5).value<DataSet*>()->samples();
    QCOMPARE(samples.front(), QVector2D(0, 5));
}

void ViConfigModelTest::testModelDataSouvis6000()
{
    ViConfigModel model;
    QSignalSpy modelResetSpy{&model, &ViConfigModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    ViConfigService viConfig;
    QString path = QFINDTESTDATA("testdata/souvis6000");
    QVERIFY(!path.isEmpty());
    viConfig.setConfigurationDir(path);

    model.setViConfig(&viConfig);
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.rowCount(), 1);

    QCOMPARE(model.index(0, 0).data().toString(), QStringLiteral("Product Number"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole+2).value<ViConfigService::SignalType>(), ViConfigService::SignalType::Input);

    model.toggleEnabled(model.index(0, 0));
    auto dataSets = model.dataSets();
    QCOMPARE(dataSets.count(), 1);
    QCOMPARE(dataSets.at(0).value<DataSet*>()->name(), QStringLiteral("Product Number"));

    // add service
    ServiceToGuiServer service{{}};
    model.setService(&service);
    EC_T_GET_SLAVE_INFO infos[2];
    infos[0].dwVendorId = 27;
    infos[0].dwProductCode = 61;
    infos[0].wCfgPhyAddress = 0;
    infos[0].dwPdSizeIn = 20 * 8;
    infos[0].dwPdSizeOut = 20 * 8;
    infos[0].dwPdOffsIn = 0;
    infos[0].dwPdOffsOut = 20 * 8;
    infos[1].dwVendorId = 27;
    infos[1].dwProductCode = 61;
    infos[1].wCfgPhyAddress = 30;
    infos[1].dwPdSizeIn = 20 * 8;
    infos[1].dwPdSizeOut = 20 * 8;
    infos[1].dwPdOffsIn = 40 * 8;
    infos[1].dwPdOffsOut = 60 * 8;
    precitec::interface::SlaveInfo info{2};
    info.FillBuffer(infos);
    QSignalSpy slaveInfoChangedSpy{&service, &ServiceToGuiServer::slaveInfoChanged};
    QVERIFY(slaveInfoChangedSpy.isValid());
    QtConcurrent::run([&] {service.SlaveInfoECAT(2, info);});
    QVERIFY(slaveInfoChangedSpy.wait());

    // now send some data
    QSignalSpy dataSetChanged{dataSets.first().value<DataSet*>(), &DataSet::samplesChanged};
    QByteArray overallData{80, 0};
    uint32_t value=12345;
    std::memcpy(overallData.data() + 2, &value, sizeof value);
    std::vector<precitec::interface::ProcessData> processData{{80, overallData.constData()}};
    QtConcurrent::run([&] {service.ProcessImage(processData, processData);});
    QVERIFY(dataSetChanged.wait());

    QCOMPARE(dataSets.at(0).value<DataSet*>()->sampleCount(), 1);
    auto samples = dataSets.at(0).value<DataSet*>()->samples();
    QCOMPARE(samples.front(), QVector2D(0, 12345));
}

void ViConfigModelTest::testSetStorageDir()
{
    ViConfigModel model;
    QCOMPARE(model.storageDir(), QString());
    QSignalSpy storageDirChangedSpy{&model, &ViConfigModel::storageDirChanged};
    QVERIFY(storageDirChangedSpy.isValid());

    model.setStorageDir(QStringLiteral("foo"));
    QCOMPARE(storageDirChangedSpy.count(), 1);
    QCOMPARE(model.storageDir(), QStringLiteral("foo"));

    // setting same should not change
    model.setStorageDir(QStringLiteral("foo"));
    QCOMPARE(storageDirChangedSpy.count(), 1);
}

void ViConfigModelTest::testRecording()
{
    ViConfigModel model;
    QSignalSpy modelResetSpy{&model, &ViConfigModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    ViConfigService viConfig;
    QString path = QFINDTESTDATA("testdata");
    QVERIFY(!path.isEmpty());
    viConfig.setConfigurationDir(path);

    model.setViConfig(&viConfig);
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.rowCount(), 7);
    model.toggleEnabled(model.index(0, 0));
    model.toggleEnabled(model.index(1, 0));
    model.toggleEnabled(model.index(2, 0));
    model.toggleEnabled(model.index(5, 0));

    // set colors
    model.setColor(model.index(0, 0), Qt::red);
    model.setColor(model.index(1, 0), Qt::green);
    model.setColor(model.index(2, 0), Qt::blue);

    // add service
    ServiceToGuiServer service{{}};
    model.setService(&service);
    EC_T_GET_SLAVE_INFO infos[2];
    infos[0].dwVendorId = 27;
    infos[0].dwProductCode = 61;
    infos[0].wCfgPhyAddress = 0;
    infos[0].dwPdSizeIn = 20 * 8;
    infos[0].dwPdSizeOut = 20 * 8;
    infos[0].dwPdOffsIn = 0;
    infos[0].dwPdOffsOut = 20 * 8;
    infos[1].dwVendorId = 27;
    infos[1].dwProductCode = 61;
    infos[1].wCfgPhyAddress = 30;
    infos[1].dwPdSizeIn = 20 * 8;
    infos[1].dwPdSizeOut = 20 * 8;
    infos[1].dwPdOffsIn = 40 * 8;
    infos[1].dwPdOffsOut = 60 * 8;
    precitec::interface::SlaveInfo info{2};
    info.FillBuffer(infos);
    QSignalSpy slaveInfoChangedSpy{&service, &ServiceToGuiServer::slaveInfoChanged};
    QVERIFY(slaveInfoChangedSpy.isValid());
    QtConcurrent::run([&] {service.SlaveInfoECAT(2, info);});
    QVERIFY(slaveInfoChangedSpy.wait());

    // add storage dir
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    model.setStorageDir(dir.path());

    QSignalSpy recordingChangedSpy{&model, &ViConfigModel::recordingChanged};
    QVERIFY(recordingChangedSpy.isValid());
    QSignalSpy processingChangedSpy{&model, &ViConfigModel::processingChanged};
    QVERIFY(processingChangedSpy.isValid());

    model.startRecording();
    QCOMPARE(recordingChangedSpy.count(), 1);
    QCOMPARE(model.isRecording(), true);

    // find the timer
    auto timer = model.findChild<QTimer*>();
    QVERIFY(timer);
    QVERIFY(timer->isActive());

    QByteArray overallData{80, 0};
    overallData.data()[0] = 3;
    overallData.data()[2] = 3;
    overallData.data()[20] = 2;
    reinterpret_cast<uint32_t*>(overallData.data())[8] = 123456789;
    reinterpret_cast<uint16_t*>(overallData.data())[24] = 1234;
    std::vector<precitec::interface::ProcessData> processData{
        {80, overallData.constData()},
        {80, QByteArray{80, 0}.constData()},
        {80, overallData.constData()},
        {80, QByteArray{80, 0}.constData()},
        {80, overallData.constData()}};
    QtConcurrent::run([&] {service.ProcessImage(processData, processData);});

    const auto current = QDateTime::currentDateTime();
    timer->setInterval(std::chrono::milliseconds{10});
    QCOMPARE(processingChangedSpy.count(), 0);
    QVERIFY(recordingChangedSpy.wait());
    QCOMPARE(recordingChangedSpy.count(), 2);
    QCOMPARE(processingChangedSpy.count(), 2);

    QFile jsonFile{dir.filePath(QStringLiteral("signalAnalyzer.json"))};
    QVERIFY(jsonFile.exists());
    QVERIFY(jsonFile.open(QIODevice::ReadOnly));

    auto doc = QJsonDocument::fromJson(QCborValue::fromCbor(qUncompress(jsonFile.readAll())).toByteArray());
    QCOMPARE(doc.isEmpty(), false);
    auto rootObject = doc.object();
    QCOMPARE(rootObject.count(), 2);
    QVERIFY(rootObject.contains(QLatin1String("date")));
    QVERIFY(rootObject.contains(QLatin1String("samples")));

    const auto date = QDateTime::fromString(rootObject.value(QLatin1String("date")).toString(), Qt::ISODate);
    QVERIFY(current.secsTo(date) <= 1 && current.secsTo(date) >= 0);

    const auto samples = rootObject.value(QLatin1String("samples")).toArray();
    QCOMPARE(samples.count(), 4);
    auto dataSetJson = samples.at(0).toObject();
    QCOMPARE(dataSetJson.value(QLatin1String("name")).toString(), QStringLiteral("Part OK (OK=1)"));
    auto color = dataSetJson.value(QLatin1String("color")).toObject();
    QCOMPARE(color.value(QLatin1String("r")).toInt(), 255);
    QCOMPARE(color.value(QLatin1String("g")).toInt(), 0);
    QCOMPARE(color.value(QLatin1String("b")).toInt(), 0);
    auto dataSetSamples = dataSetJson.value(QLatin1String("samples")).toArray();
    QCOMPARE(dataSetSamples.count(), 5);
    QCOMPARE(dataSetSamples.at(0).toDouble(), 1.0);
    QCOMPARE(dataSetSamples.at(1).toDouble(), 0.0);
    QCOMPARE(dataSetSamples.at(2).toDouble(), 1.0);
    QCOMPARE(dataSetSamples.at(3).toDouble(), 0.0);
    QCOMPARE(dataSetSamples.at(4).toDouble(), 1.0);

    dataSetJson = samples.at(1).toObject();
    QCOMPARE(dataSetJson.value(QLatin1String("name")).toString(), QStringLiteral("Part No"));
    color = dataSetJson.value(QLatin1String("color")).toObject();
    QCOMPARE(color.value(QLatin1String("r")).toInt(), 0);
    QCOMPARE(color.value(QLatin1String("g")).toInt(), 255);
    QCOMPARE(color.value(QLatin1String("b")).toInt(), 0);
    dataSetSamples = dataSetJson.value(QLatin1String("samples")).toArray();
    QCOMPARE(dataSetSamples.count(), 5);
    QCOMPARE(dataSetSamples.at(0).toDouble(), 123456789.0f);
    QCOMPARE(dataSetSamples.at(1).toDouble(), 0.0f);
    QCOMPARE(dataSetSamples.at(2).toDouble(), 123456789.0f);
    QCOMPARE(dataSetSamples.at(3).toDouble(), 0.0f);
    QCOMPARE(dataSetSamples.at(4).toDouble(), 123456789.0f);

    dataSetJson = samples.at(2).toObject();
    QCOMPARE(dataSetJson.value(QLatin1String("name")).toString(), QStringLiteral("Inspection Cycle Active"));
    color = dataSetJson.value(QLatin1String("color")).toObject();
    QCOMPARE(color.value(QLatin1String("r")).toInt(), 0);
    QCOMPARE(color.value(QLatin1String("g")).toInt(), 0);
    QCOMPARE(color.value(QLatin1String("b")).toInt(), 255);
    dataSetSamples = dataSetJson.value(QLatin1String("samples")).toArray();
    QCOMPARE(dataSetSamples.count(), 5);
    QCOMPARE(dataSetSamples.at(0).toDouble(), 1.0);
    QCOMPARE(dataSetSamples.at(1).toDouble(), 0.0);
    QCOMPARE(dataSetSamples.at(2).toDouble(), 1.0);
    QCOMPARE(dataSetSamples.at(3).toDouble(), 0.0);
    QCOMPARE(dataSetSamples.at(4).toDouble(), 1.0);

    dataSetJson = samples.at(3).toObject();
    QCOMPARE(dataSetJson.value(QLatin1String("name")).toString(), QStringLiteral("Seam Number"));
    dataSetSamples = dataSetJson.value(QLatin1String("samples")).toArray();
    QCOMPARE(dataSetSamples.count(), 5);
    QCOMPARE(dataSetSamples.at(0).toDouble(), 3.0);
    QCOMPARE(dataSetSamples.at(1).toDouble(), 0.0);
    QCOMPARE(dataSetSamples.at(2).toDouble(), 3.0);
    QCOMPARE(dataSetSamples.at(3).toDouble(), 0.0);
    QCOMPARE(dataSetSamples.at(4).toDouble(), 3.0);
}

QTEST_GUILESS_MAIN(ViConfigModelTest)
#include "viConfigModelTest.moc"
