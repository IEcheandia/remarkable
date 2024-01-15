#include <QTest>
#include <QSignalSpy>

#include "../src/laserControlPreset.h"
#include "precitec/dataSet.h"

using precitec::storage::LaserControlPreset;
using precitec::gui::components::plotter::DataSet;

class LaserControlPresetTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testGetValue();
    void testSetName();
    void testPower_data();
    void testPower();
    void testOffset_data();
    void testOffset();
    void testSetEnabled();
    void testSetChange();
    void testSetState();
    void testSetFilePath();
    void testSetValue();
    void testLoad();
    void testDelete();
    void testSave();
    void testRestore();
    void testComputeSamples();
};

void LaserControlPresetTest::testCtor()
{
    LaserControlPreset preset{this};
    QVERIFY(!preset.uuid().isNull());
    QVERIFY(preset.channel1PowerDataSet());
    QCOMPARE(preset.channel1PowerDataSet()->color(), QColor("magenta"));
    QCOMPARE(preset.channel1PowerDataSet()->drawingMode(), DataSet::DrawingMode::Line);
    QCOMPARE(preset.channel1PowerDataSet()->drawingOrder(), DataSet::DrawingOrder::OnBottom);
    QCOMPARE(preset.channel1PowerDataSet()->sampleCount(), 7);
    QCOMPARE(preset.name(), QStringLiteral(""));
    QCOMPARE(preset.power(0), 0);
    QCOMPARE(preset.power(1), 0);
    QCOMPARE(preset.power(2), 0);
    QCOMPARE(preset.power(3), 0);
    QCOMPARE(preset.offset(0), 0);
    QCOMPARE(preset.offset(1), 0);
    QCOMPARE(preset.offset(2), 0);
    QCOMPARE(preset.offset(3), 0);
    QCOMPARE(preset.enabled(), true);
    QCOMPARE(preset.state(), LaserControlPreset::State::Default);
    QCOMPARE(preset.hasChanges(), false);
    QCOMPARE(preset.filePath(), QString());

    auto count = 0;
    for (auto sample : preset.channel1PowerDataSet()->samples())
    {
        QCOMPARE(sample , QVector2D(25 * count, -1));
        count++;
    }
}

void LaserControlPresetTest::testSetName()
{
    LaserControlPreset preset{this};
    QSignalSpy nameChangedSpy(&preset, &LaserControlPreset::nameChanged);
    QVERIFY(nameChangedSpy.isValid());

    QSignalSpy changedSpy(&preset, &LaserControlPreset::markAsChanged);
    QVERIFY(changedSpy.isValid());

    QCOMPARE(preset.name(), QStringLiteral(""));

    preset.setName(QStringLiteral(""));
    QCOMPARE(nameChangedSpy.count(), 0);
    QCOMPARE(preset.hasChanges(), false);
    QCOMPARE(changedSpy.count(), 0);

    preset.setName(QStringLiteral("Profile 1"));
    QCOMPARE(preset.name(), QStringLiteral("Profile 1"));
    QCOMPARE(nameChangedSpy.count(), 1);
    QCOMPARE(preset.hasChanges(), true);
    QCOMPARE(changedSpy.count(), 1);

    preset.setName(QStringLiteral("Profile 1"));
    QCOMPARE(nameChangedSpy.count(), 1);
    QCOMPARE(changedSpy.count(), 1);
}

void LaserControlPresetTest::testPower_data()
{
    QTest::addColumn<int>("presetNumber");

    QTest::newRow("0")  <<  0;
    QTest::newRow("1")  <<  1;
    QTest::newRow("2")  <<  2;
    QTest::newRow("3")  <<  3;
}

void LaserControlPresetTest::testPower()
{
    LaserControlPreset preset{this};
    QSignalSpy powerChangedSpy(&preset, &LaserControlPreset::powerChanged);
    QVERIFY(powerChangedSpy.isValid());

    QSignalSpy changedSpy(&preset, &LaserControlPreset::markAsChanged);
    QVERIFY(changedSpy.isValid());

    QFETCH(int, presetNumber);

    QCOMPARE(preset.power(presetNumber), 0);

    preset.setPower(presetNumber, 0);
    QCOMPARE(powerChangedSpy.count(), 0);
    QCOMPARE(preset.hasChanges(), false);
    QCOMPARE(changedSpy.count(), 0);

    preset.setPower(presetNumber, 5);
    QCOMPARE(preset.power(presetNumber), 5);
    QCOMPARE(powerChangedSpy.count(), 1);
    QCOMPARE(preset.hasChanges(), true);
    QCOMPARE(changedSpy.count(), 1);

    preset.setPower(presetNumber, 5);
    QCOMPARE(powerChangedSpy.count(), 1);
    QCOMPARE(changedSpy.count(), 1);

    preset.setPower(presetNumber, -3);
    QCOMPARE(preset.power(0), 0);
    QCOMPARE(powerChangedSpy.count(), 2);
    QCOMPARE(changedSpy.count(), 1);

    preset.setPower(presetNumber, 120);
    QCOMPARE(preset.power(presetNumber), 100);
    QCOMPARE(powerChangedSpy.count(), 3);
    QCOMPARE(changedSpy.count(), 1);

    while (!powerChangedSpy.empty())
    {
        auto arguments = powerChangedSpy.takeFirst();
        QCOMPARE(arguments.at(0).toUInt(), presetNumber);
    }
}

void LaserControlPresetTest::testOffset_data()
{
    QTest::addColumn<int>("presetNumber");

    QTest::newRow("0")  <<  0;
    QTest::newRow("1")  <<  1;
    QTest::newRow("2")  <<  2;
    QTest::newRow("3")  <<  3;
}

void LaserControlPresetTest::testOffset()
{
    LaserControlPreset preset{this};
    QSignalSpy offsetChangedSpy(&preset, &LaserControlPreset::offsetChanged);
    QVERIFY(offsetChangedSpy.isValid());

    QSignalSpy changedSpy(&preset, &LaserControlPreset::markAsChanged);
    QVERIFY(changedSpy.isValid());

    QFETCH(int, presetNumber);

QCOMPARE(preset.offset(presetNumber), 0);

    preset.setOffset(presetNumber, 0);
    QCOMPARE(offsetChangedSpy.count(), 0);
    QCOMPARE(preset.hasChanges(), false);
    QCOMPARE(changedSpy.count(), 0);

    preset.setOffset(presetNumber, 5);
    QCOMPARE(preset.offset(presetNumber), 5);
    QCOMPARE(offsetChangedSpy.count(), 1);
    QCOMPARE(preset.hasChanges(), true);
    QCOMPARE(changedSpy.count(), 1);

    preset.setOffset(presetNumber, 5);
    QCOMPARE(offsetChangedSpy.count(), 1);
    QCOMPARE(changedSpy.count(), 1);

    preset.setOffset(presetNumber, -3);
    QCOMPARE(preset.offset(presetNumber), 0);
    QCOMPARE(offsetChangedSpy.count(), 2);
    QCOMPARE(changedSpy.count(), 1);

    preset.setOffset(presetNumber, 120);
    QCOMPARE(preset.offset(presetNumber), 100);
    QCOMPARE(offsetChangedSpy.count(), 3);
    QCOMPARE(changedSpy.count(), 1);

    for (auto i = 0; i <= offsetChangedSpy.count(); i++)
    {
        auto arguments = offsetChangedSpy.takeFirst();
        QCOMPARE(arguments.at(0).toUInt(), presetNumber);
    }
}

void LaserControlPresetTest::testSetEnabled()
{
    LaserControlPreset preset{this};
    QSignalSpy enabledChangedSpy(&preset, &LaserControlPreset::enabledChanged);
    QVERIFY(enabledChangedSpy.isValid());

    QCOMPARE(preset.enabled(), true);
    QCOMPARE(preset.channel1PowerDataSet()->color(), QColor("magenta"));

    preset.setEnabled(true);
    QCOMPARE(enabledChangedSpy.count(), 0);

    preset.setState(LaserControlPreset::State::Edit);
    preset.setEnabled(false);
    QCOMPARE(enabledChangedSpy.count(), 0);

    preset.setState(LaserControlPreset::State::New);
    preset.setEnabled(false);
    QCOMPARE(enabledChangedSpy.count(), 0);

    preset.setState(LaserControlPreset::State::Default);
    preset.setEnabled(false);
    QCOMPARE(preset.enabled(), false);
    QCOMPARE(enabledChangedSpy.count(), 1);
    QCOMPARE(preset.channel1PowerDataSet()->color(), QColor("gray"));
}

void LaserControlPresetTest::testSetChange()
{
    LaserControlPreset preset{this};
    QSignalSpy changedSpy(&preset, &LaserControlPreset::markAsChanged);
    QVERIFY(changedSpy.isValid());

    QCOMPARE(preset.hasChanges(), false);

    preset.setChange(false);
    QCOMPARE(changedSpy.count(), 0);

    preset.setChange(true);
    QCOMPARE(preset.hasChanges(), true);
    QCOMPARE(changedSpy.count(), 1);

    preset.setChange(true);
    QCOMPARE(changedSpy.count(), 1);
}

void LaserControlPresetTest::testSetState()
{
    LaserControlPreset preset{this};
    QSignalSpy stateChangedSpy(&preset, &LaserControlPreset::stateChanged);
    QVERIFY(stateChangedSpy.isValid());
    QSignalSpy editStartedSpy(&preset, &LaserControlPreset::editStarted);
    QVERIFY(editStartedSpy.isValid());
    QSignalSpy editStoppedSpy(&preset, &LaserControlPreset::editStopped);
    QVERIFY(editStoppedSpy.isValid());

    QCOMPARE(preset.state(), LaserControlPreset::State::Default);

    preset.setState(LaserControlPreset::State::Default);
    QCOMPARE(stateChangedSpy.count(), 0);
    QCOMPARE(editStartedSpy.count(), 0);
    QCOMPARE(editStoppedSpy.count(), 0);

    preset.setState(LaserControlPreset::State::Edit);
    QCOMPARE(preset.state(), LaserControlPreset::State::Edit);
    QCOMPARE(stateChangedSpy.count(), 1);
    QCOMPARE(editStartedSpy.count(), 1);
    QCOMPARE(editStoppedSpy.count(), 0);

    preset.setState(LaserControlPreset::State::Default);
    QCOMPARE(stateChangedSpy.count(), 2);
    QCOMPARE(editStartedSpy.count(), 1);
    QCOMPARE(editStoppedSpy.count(), 1);

    preset.setState(LaserControlPreset::State::New);
    QCOMPARE(preset.state(), LaserControlPreset::State::New);
    QCOMPARE(stateChangedSpy.count(), 3);
    QCOMPARE(editStartedSpy.count(), 2);
    QCOMPARE(editStoppedSpy.count(), 1);

    preset.setState(LaserControlPreset::State::Default);
    QCOMPARE(stateChangedSpy.count(), 4);
    QCOMPARE(editStartedSpy.count(), 2);
    QCOMPARE(editStoppedSpy.count(), 2);
}

void LaserControlPresetTest::testSetFilePath()
{
    LaserControlPreset preset{this};
    QCOMPARE(preset.filePath(), QString());

    preset.setFilePath(QStringLiteral("myFolder"));
    QCOMPARE(preset.filePath(), QStringLiteral("myFolder"));
}

void LaserControlPresetTest::testGetValue()
{
    LaserControlPreset preset{this};
    preset.setPower({1, 2, 3, 4, 9, 10, 11, 12});
    preset.setOffset({5, 6, 7, 8, 13, 14, 15, 16});

    for (int i = 0; i < 16; i++)
    {
        QCOMPARE(preset.getValue(LaserControlPreset::Key(i)), i + 1);
    }
}

void LaserControlPresetTest::testSetValue()
{
    LaserControlPreset preset{this};
    QSignalSpy changedSpy(&preset, &LaserControlPreset::markAsChanged);
    QVERIFY(changedSpy.isValid());

    QCOMPARE(changedSpy.count(), 0);

    for (auto i = 0; i < 16; i++)
    {
        preset.setValue(LaserControlPreset::Key(i), i);
        QCOMPARE(preset.getValue(LaserControlPreset::Key(i)), i);
    }

    QCOMPARE(changedSpy.count(), 1);
}

void LaserControlPresetTest::testLoad()
{
    const QString presetDir = QFINDTESTDATA("testdata/laser_control/preset_1.json");

    const auto nullPreset = LaserControlPreset::load(QString(), this);
    QVERIFY(!nullPreset);

    const auto preset = LaserControlPreset::load(presetDir, this);
    QVERIFY(preset);
    QCOMPARE(preset->uuid(), QUuid("14f819a6-e946-4e55-98e7-727c8f8d7754"));
    QCOMPARE(preset->name(), QStringLiteral("Default 1"));
    QCOMPARE(preset->filePath(), presetDir);
    QCOMPARE(preset->power(0), 53);
    QCOMPARE(preset->power(1), 60);
    QCOMPARE(preset->power(2), 20);
    QCOMPARE(preset->power(3), 60);
    QCOMPARE(preset->offset(0), 25);
    QCOMPARE(preset->offset(1), 100);
    QCOMPARE(preset->offset(2), 0);
    QCOMPARE(preset->offset(3), 100);
    QCOMPARE(preset->hasChanges(), false);
}

void LaserControlPresetTest::testDelete()
{
    QTemporaryDir dir;
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/laser_control/preset_1.json"), dir.filePath(QStringLiteral("preset.json"))));

    QVERIFY(LaserControlPreset::load(dir.filePath(QStringLiteral("preset.json")), this));

    QVERIFY(LaserControlPreset::deleteFile(dir.filePath(QStringLiteral("preset.json"))));

    QVERIFY(!LaserControlPreset::load(dir.filePath(QStringLiteral("preset.json")), this));
}

void LaserControlPresetTest::testSave()
{
    auto preset = LaserControlPreset::load(QFINDTESTDATA("testdata/laser_control/preset_1.json"), this);
    QVERIFY(preset);

    QTemporaryDir dir;
    preset->save(dir.filePath(QStringLiteral("preset.json")));

    auto savedFile = LaserControlPreset::load(dir.filePath(QStringLiteral("preset.json")), this);
    QVERIFY(savedFile);

    QCOMPARE(preset->uuid(), savedFile->uuid());
    QCOMPARE(preset->name(), savedFile->name());
    QCOMPARE(preset->power(0), savedFile->power(0));
    QCOMPARE(preset->power(1), savedFile->power(1));
    QCOMPARE(preset->power(2), savedFile->power(2));
    QCOMPARE(preset->power(3), savedFile->power(3));
    QCOMPARE(preset->offset(0), savedFile->offset(0));
    QCOMPARE(preset->offset(1), savedFile->offset(1));
    QCOMPARE(preset->offset(2), savedFile->offset(2));
    QCOMPARE(preset->offset(3), savedFile->offset(3));
    QCOMPARE(preset->filePath(), savedFile->filePath());
    QCOMPARE(preset->state(), LaserControlPreset::State::Default);
    QCOMPARE(preset->hasChanges(), false);
    QCOMPARE(savedFile->state(), LaserControlPreset::State::Default);
    QCOMPARE(savedFile->hasChanges(), false);
}

void LaserControlPresetTest::testRestore()
{
    const QString presetDir = QFINDTESTDATA("testdata/laser_control/preset_1.json");

    const auto preset = LaserControlPreset::load(presetDir, this);
    QVERIFY(preset);

    QCOMPARE(preset->uuid(), QUuid("14f819a6-e946-4e55-98e7-727c8f8d7754"));
    QCOMPARE(preset->name(), QStringLiteral("Default 1"));
    QCOMPARE(preset->filePath(), presetDir);
    QCOMPARE(preset->power(0), 53);
    QCOMPARE(preset->power(1), 60);
    QCOMPARE(preset->power(2), 20);
    QCOMPARE(preset->power(3), 60);
    QCOMPARE(preset->offset(0), 25);
    QCOMPARE(preset->offset(1), 100);
    QCOMPARE(preset->offset(2), 0);
    QCOMPARE(preset->offset(3), 100);
    QCOMPARE(preset->hasChanges(), false);

    preset->setName(QStringLiteral("testProfile"));
    preset->setPower(0, 1);
    preset->setPower(1, 2);
    preset->setPower(2, 3);
    preset->setPower(3, 4);
    preset->setOffset(0, 5);
    preset->setOffset(1, 6);
    preset->setOffset(2, 7);
    preset->setOffset(3, 8);

    QCOMPARE(preset->name(), QStringLiteral("testProfile"));
    QCOMPARE(preset->power(0), 1);
    QCOMPARE(preset->power(1), 2);
    QCOMPARE(preset->power(2), 3);
    QCOMPARE(preset->power(3), 4);
    QCOMPARE(preset->offset(0), 5);
    QCOMPARE(preset->offset(1), 6);
    QCOMPARE(preset->offset(2), 7);
    QCOMPARE(preset->offset(3), 8);
    QCOMPARE(preset->hasChanges(), true);

    preset->restore();

    QCOMPARE(preset->uuid(), QUuid("14f819a6-e946-4e55-98e7-727c8f8d7754"));
    QCOMPARE(preset->name(), QStringLiteral("Default 1"));
    QCOMPARE(preset->filePath(), presetDir);
    QCOMPARE(preset->power(0), 53);
    QCOMPARE(preset->power(1), 60);
    QCOMPARE(preset->power(2), 20);
    QCOMPARE(preset->power(3), 60);
    QCOMPARE(preset->offset(0), 25);
    QCOMPARE(preset->offset(1), 100);
    QCOMPARE(preset->offset(2), 0);
    QCOMPARE(preset->offset(3), 100);
    QCOMPARE(preset->hasChanges(), false);
}

void LaserControlPresetTest::testComputeSamples()
{
    LaserControlPreset preset{this};
    QVERIFY(!preset.uuid().isNull());
    QVERIFY(preset.channel1PowerDataSet());
    QCOMPARE(preset.channel1PowerDataSet()->color(), QColor("magenta"));
    QCOMPARE(preset.channel1PowerDataSet()->drawingMode(), DataSet::DrawingMode::Line);
    QCOMPARE(preset.channel1PowerDataSet()->drawingOrder(), DataSet::DrawingOrder::OnBottom);

    auto count = 0;
    for (auto sample : preset.channel1PowerDataSet()->samples())
    {
        QCOMPARE(sample , QVector2D(25.0f * count, -1.0f));
        count++;
    }

    preset.setPower(0, 20);
    preset.setPower(1, 40);
    preset.setPower(2, 60);
    preset.setPower(3, 80);

    QCOMPARE(preset.power(0), 20);
    QCOMPARE(preset.power(1), 40);
    QCOMPARE(preset.power(2), 60);
    QCOMPARE(preset.power(3), 80);

    std::vector<float> values = {1.6f, 0.4f, 0.8f, 1.2f};

    count = 0;
    for (auto sample : preset.channel1PowerDataSet()->samples())
    {
        QVERIFY(qFuzzyCompare(float(sample.x()), 25 * count));
        QVERIFY(qFuzzyCompare(float(sample.y()), values.at(count % 4) - 1.0f));
        count++;
    }
}

QTEST_GUILESS_MAIN(LaserControlPresetTest)
#include "testLaserControlPreset.moc"
