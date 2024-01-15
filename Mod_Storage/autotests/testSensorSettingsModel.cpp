#include <QTest>
#include <QSignalSpy>

#include "../src/resultSetting.h"
#include "../src/sensorSettingsModel.h"

using precitec::storage::ResultSetting;
using precitec::storage::SensorSettingsModel;

static const int NUMBER_OF_SENSORS = 55;

class TestSensorSettingsModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testConfigurationDirecory();
    void testLoad();
    void testLoadNoFile();
    void testSave();
    void testUpdateValue();
};


void TestSensorSettingsModel::testCtor()
{
    SensorSettingsModel model;
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.configurationDirectory().isEmpty());
}

void TestSensorSettingsModel::testRoleNames()
{
    SensorSettingsModel model;
    const auto roles = model.roleNames();
    QCOMPARE(roles.size(), 14);
    QCOMPARE(roles.value(Qt::DisplayRole), QByteArrayLiteral("enumType"));
    QCOMPARE(roles.value(Qt::UserRole), QByteArrayLiteral("uuid"));
    QCOMPARE(roles.value(Qt::UserRole + 1), QByteArrayLiteral("name"));
    QCOMPARE(roles.value(Qt::UserRole + 2), QByteArrayLiteral("plotterNumber"));
    QCOMPARE(roles.value(Qt::UserRole + 3), QByteArrayLiteral("plottable"));
    QCOMPARE(roles.value(Qt::UserRole + 4), QByteArrayLiteral("min"));
    QCOMPARE(roles.value(Qt::UserRole + 5), QByteArrayLiteral("max"));
    QCOMPARE(roles.value(Qt::UserRole + 6), QByteArrayLiteral("lineColor"));
    QCOMPARE(roles.value(Qt::UserRole + 7), QByteArrayLiteral("visibleItem"));
    QCOMPARE(roles.value(Qt::UserRole + 8), QByteArrayLiteral("visualization"));
    QCOMPARE(roles.value(Qt::UserRole + 9), QByteArrayLiteral("disabled"));
    QCOMPARE(roles.value(Qt::UserRole + 10), QByteArrayLiteral("hue"));
    QCOMPARE(roles.value(Qt::UserRole + 11), QByteArrayLiteral("saturation"));
    QCOMPARE(roles.value(Qt::UserRole + 12), QByteArrayLiteral("lightness"));
}

void TestSensorSettingsModel::testConfigurationDirecory()
{
    SensorSettingsModel model;
    QSignalSpy configurationDirectoryChangedSpy(&model, &SensorSettingsModel::configurationDirectoryChanged);
    QVERIFY(configurationDirectoryChangedSpy.isValid());

    model.setConfigurationDirectory(QStringLiteral(""));
    QCOMPARE(configurationDirectoryChangedSpy.count(), 0);

    model.setConfigurationDirectory(QStringLiteral("someDir"));
    QCOMPARE(model.configurationDirectory(), QStringLiteral("someDir"));
    QCOMPARE(configurationDirectoryChangedSpy.count(), 1);

    model.setConfigurationDirectory(QStringLiteral("someDir"));
    QCOMPARE(configurationDirectoryChangedSpy.count(), 1);
}

void TestSensorSettingsModel::testLoad()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/products/sensorConfig.json"), dir.filePath(QStringLiteral("sensorConfig.json"))));

    SensorSettingsModel model;
    QSignalSpy modelResetSpy(&model, &QAbstractItemModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    QCOMPARE(model.rowCount(), 0);

    model.setConfigurationDirectory(dir.path());
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.rowCount(), NUMBER_OF_SENSORS);
    QCOMPARE(model.m_sensorItems.at(0)->name(), QStringLiteral("My X Axis position"));

    QVERIFY(QFile::remove(dir.filePath(QStringLiteral("sensorConfig.json"))));
}

void TestSensorSettingsModel::testLoadNoFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    SensorSettingsModel model;
    QSignalSpy modelResetSpy(&model, &QAbstractItemModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    QCOMPARE(model.rowCount(), 0);

    model.setConfigurationDirectory(dir.path());
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.rowCount(), NUMBER_OF_SENSORS);
    QCOMPARE(model.m_sensorItems.at(0)->name(), QStringLiteral("X Axis position"));

    QVERIFY(QFile::remove(dir.filePath(QStringLiteral("sensorConfig.json"))));
}

void TestSensorSettingsModel::testSave()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/products/sensorConfig.json"), dir.filePath(QStringLiteral("sensorConfig.json"))));

    SensorSettingsModel model;
    QSignalSpy modelResetSpy(&model, &QAbstractItemModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    QCOMPARE(model.rowCount(), 0);

    model.setConfigurationDirectory(dir.path());
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.rowCount(), NUMBER_OF_SENSORS);
    QCOMPARE(model.m_sensorItems.at(0)->name(), QStringLiteral("My X Axis position"));

    model.checkAndAddItem(0, QStringLiteral("Name"), QColor("white"));
    QCOMPARE(model.rowCount(), NUMBER_OF_SENSORS + 1);

    SensorSettingsModel model2;
    QSignalSpy model2ResetSpy(&model2, &QAbstractItemModel::modelReset);
    QVERIFY(model2ResetSpy.isValid());
    model2.setConfigurationDirectory(dir.path());
    QCOMPARE(model2ResetSpy.count(), 1);
    QCOMPARE(model2.rowCount(), NUMBER_OF_SENSORS + 1);

    QVERIFY(QFile::remove(dir.filePath(QStringLiteral("sensorConfig.json"))));
}

void TestSensorSettingsModel::testUpdateValue()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/products/sensorConfig.json"), dir.filePath(QStringLiteral("sensorConfig.json"))));

    SensorSettingsModel model;
    QSignalSpy dataChangedSpy{&model, &SensorSettingsModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    model.setConfigurationDirectory(dir.path());

    QCOMPARE(model.rowCount(), NUMBER_OF_SENSORS);

    QCOMPARE(model.m_sensorItems.at(0)->name(), QStringLiteral("My X Axis position"));
    QVERIFY(model.m_sensorItems.at(4));
    QCOMPARE(model.m_sensorItems.at(4)->name(), QStringLiteral("Glas Dirty"));
    model.updateValue(model.index(0), QStringLiteral("new text for test"), ResultSetting::Type::Name);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.last().at(0).toModelIndex(), model.index(0));
    QCOMPARE(dataChangedSpy.last().at(1).toModelIndex(), model.index(0));
    model.updateValue(model.index(4), QStringLiteral("new text for violation"), ResultSetting::Type::Name);
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.last().at(0).toModelIndex(), model.index(4));
    QCOMPARE(dataChangedSpy.last().at(1).toModelIndex(), model.index(4));
    QVERIFY(model.m_sensorItems.at(0));
    QCOMPARE(model.m_sensorItems.at(0)->name(), QStringLiteral("new text for test"));
    QVERIFY(model.m_sensorItems.at(4));
    QCOMPARE(model.m_sensorItems.at(4)->name(), QStringLiteral("new text for violation"));

    QVERIFY(QFile::remove(dir.filePath(QStringLiteral("sensorConfig.json"))));
}

QTEST_GUILESS_MAIN(TestSensorSettingsModel)
#include "testSensorSettingsModel.moc"


