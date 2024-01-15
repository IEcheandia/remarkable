#include <QTest>
#include <QSignalSpy>

#include "../src/laserControlMeasureModel.h"
#include "laserControlPreset.h"
#include "attribute.h"
#include "attributeModel.h"
#include "parameter.h"
#include "parameterSet.h"
#include "seam.h"
#include "seamSeries.h"
#include "product.h"

using precitec::gui::LaserControlMeasureModel;
using precitec::gui::AbstractLaserControlModel;
using precitec::storage::Attribute;
using precitec::storage::AttributeModel;
using precitec::storage::Parameter;
using precitec::storage::ParameterSet;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;
using precitec::storage::Product;
using precitec::storage::LaserControlPreset;

class LaserControlMeasureModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testDisplayRole_data();
    void testDisplayRole();
    void testPreset_data();
    void testPreset();
    void testUuid_data();
    void testUuid();
    void testSetSeam();
    void testSetSeamSeries();
    void testSetAttributeModel();
    void testSetCurrentPreset();
    void testSetLaserControlPresetDir();
    void testFindAttribute();
    void testFindParameter();
    void testUpdateHardwareParameters();
    void testFindPreset();
    void testFindPresetTwoChannels();
    void testCreateCurrentPreset();
    void testRemoveCurrentPreset();
    void testLoad();
    void testPresetEnabled();
    void testDelay();
    void testChannelTwo();
};

void LaserControlMeasureModelTest::testCtor()
{
    LaserControlMeasureModel model{this};
    QVERIFY(!model.measureTask());
    QVERIFY(!model.attributeModel());
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.currentPreset(), -1);
    QCOMPARE(model.laserControlPresetDir(), QString());
    QVERIFY(!model.channel2Enabled());
}

void LaserControlMeasureModelTest::testRoleNames()
{
    LaserControlMeasureModel model{this};
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 6);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("name"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("preset"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("uuid"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("channel1Power"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("channel2Power"));
    QCOMPARE(roleNames[Qt::UserRole + 4], QByteArrayLiteral("isCurrent"));
}

void LaserControlMeasureModelTest::testDisplayRole_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("name");

    QTest::newRow("0")  <<  0 << QStringLiteral("Default 1");
    QTest::newRow("1")  <<  1 << QStringLiteral("Default 2");
}

void LaserControlMeasureModelTest::testDisplayRole()
{
    LaserControlMeasureModel model{this};

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data().toString(), "name");
}

void LaserControlMeasureModelTest::testPreset_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("channel 1 power1");
    QTest::addColumn<int>("channel 1 power2");
    QTest::addColumn<int>("channel 1 power3");
    QTest::addColumn<int>("channel 1 power4");
    QTest::addColumn<int>("channel 1 offset1");
    QTest::addColumn<int>("channel 1 offset2");
    QTest::addColumn<int>("channel 1 offset3");
    QTest::addColumn<int>("channel 1 offset4");
    QTest::addColumn<int>("channel 2 power1");
    QTest::addColumn<int>("channel 2 power2");
    QTest::addColumn<int>("channel 2 power3");
    QTest::addColumn<int>("channel 2 power4");
    QTest::addColumn<int>("channel 2 offset1");
    QTest::addColumn<int>("channel 2 offset2");
    QTest::addColumn<int>("channel 2 offset3");
    QTest::addColumn<int>("channel 2 offset4");

    QTest::newRow("0") << 0 << 53 << 60 << 20 << 60 << 25 << 100 << 0 << 100 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;
    QTest::newRow("1") << 1 << 55 << 65 << 20 << 65 << 25 << 100 << 0 << 100 << 29 << 55 << 17 << 13 << 24 << 13 << 75 << 6;
}

void LaserControlMeasureModelTest::testPreset()
{
    LaserControlMeasureModel model{this};

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->power(0), "channel 1 power1");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->power(1), "channel 1 power2");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->power(2), "channel 1 power3");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->power(3), "channel 1 power4");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->offset(0), "channel 1 offset1");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->offset(1), "channel 1 offset2");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->offset(2), "channel 1 offset3");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->offset(3), "channel 1 offset4");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->power(4), "channel 2 power1");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->power(5), "channel 2 power2");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->power(6), "channel 2 power3");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->power(7), "channel 2 power4");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->offset(4), "channel 2 offset1");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->offset(5), "channel 2 offset2");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->offset(6), "channel 2 offset3");
    QTEST(index.data(Qt::UserRole).value<LaserControlPreset*>()->offset(7), "channel 2 offset4");
}

void LaserControlMeasureModelTest::testUuid_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QUuid>("uuid");

    QTest::newRow("0")  <<  0 << QUuid("14f819a6-e946-4e55-98e7-727c8f8d7754");
    QTest::newRow("1")  <<  1 << QUuid("7d1a636b-5de7-47f9-9e03-a917dc3c2f1d");
}

void LaserControlMeasureModelTest::testUuid()
{
    LaserControlMeasureModel model{this};

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 1).toUuid(), "uuid");
}

void LaserControlMeasureModelTest::testSetSeam()
{
    LaserControlMeasureModel model{this};

    QSignalSpy seamChangedSpy(&model, &LaserControlMeasureModel::measureTaskChanged);
    QVERIFY(seamChangedSpy.isValid());

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());

    model.setMeasureTask(s);
    QCOMPARE(model.measureTask(), s);
    QCOMPARE(seamChangedSpy.count(), 1);

    model.setMeasureTask(s);
    QCOMPARE(seamChangedSpy.count(), 1);

    auto ps = model.getParameterSet();
    QVERIFY(ps);
    QCOMPARE(s->hardwareParameters(), ps);
    QCOMPARE(model.getParameterSet(), ps);

    s->deleteLater();
    QVERIFY(seamChangedSpy.wait());
    QCOMPARE(model.measureTask(), nullptr);
    QCOMPARE(seamChangedSpy.count(), 2);
}

void LaserControlMeasureModelTest::testSetSeamSeries()
{
    LaserControlMeasureModel model{this};

    QSignalSpy seamChangedSpy(&model, &LaserControlMeasureModel::measureTaskChanged);
    QVERIFY(seamChangedSpy.isValid());

    Product p{QUuid::createUuid()};
    auto s = p.createSeamSeries();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());

    model.setMeasureTask(s);
    QCOMPARE(model.measureTask(), s);
    QCOMPARE(seamChangedSpy.count(), 1);

    model.setMeasureTask(s);
    QCOMPARE(seamChangedSpy.count(), 1);

    auto ps = model.getParameterSet();
    QVERIFY(ps);
    QCOMPARE(s->hardwareParameters(), ps);
    QCOMPARE(model.getParameterSet(), ps);

    s->deleteLater();
    QVERIFY(seamChangedSpy.wait());
    QCOMPARE(model.measureTask(), nullptr);
    QCOMPARE(seamChangedSpy.count(), 2);
}

void LaserControlMeasureModelTest::testSetAttributeModel()
{
    LaserControlMeasureModel model{this};
    QSignalSpy attributeModelChangedSpy(&model, &LaserControlMeasureModel::attributeModelChanged);
    QVERIFY(attributeModelChangedSpy.isValid());

    std::unique_ptr<AttributeModel> am = std::make_unique<AttributeModel>();
    model.setAttributeModel(am.get());
    QCOMPARE(model.attributeModel(), am.get());
    QCOMPARE(attributeModelChangedSpy.count(), 1);

    model.setAttributeModel(am.get());
    QCOMPARE(attributeModelChangedSpy.count(), 1);

    am.reset();
    QCOMPARE(model.attributeModel(), nullptr);
    QCOMPARE(attributeModelChangedSpy.count(), 2);
}

void LaserControlMeasureModelTest::testSetCurrentPreset()
{
    LaserControlMeasureModel model{this};
    QSignalSpy presetChangedSpy(&model, &LaserControlMeasureModel::currentPresetChanged);
    QVERIFY(presetChangedSpy.isValid());

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    QCOMPARE(model.currentPreset(), -1);

    model.setCurrentPreset(-1);
    QCOMPARE(presetChangedSpy.count(), 0);

    model.setCurrentPreset(5);
    QCOMPARE(model.currentPreset(), -1);
    QCOMPARE(presetChangedSpy.count(), 0);

    model.setCurrentPreset(1);
    QCOMPARE(model.currentPreset(), 1);
    QCOMPARE(presetChangedSpy.count(), 1);

    model.setCurrentPreset(1);
    QCOMPARE(presetChangedSpy.count(), 1);
}

void LaserControlMeasureModelTest::testSetLaserControlPresetDir()
{
    LaserControlMeasureModel model{this};
    QSignalSpy dirChangedSpy(&model, &LaserControlMeasureModel::laserControlPresetDirChanged);
    QVERIFY(dirChangedSpy.isValid());

    QCOMPARE(model.laserControlPresetDir(), QString());

    model.setLaserControlPresetDir(QStringLiteral(""));
    QCOMPARE(dirChangedSpy.count(), 0);

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);
    QCOMPARE(model.laserControlPresetDir(), presetDir);
    QCOMPARE(dirChangedSpy.count(), 1);

    model.setLaserControlPresetDir(presetDir);
    QCOMPARE(dirChangedSpy.count(), 1);
}

void LaserControlMeasureModelTest::testFindAttribute()
{
    LaserControlMeasureModel model{this};
    model.setCurrentPreset(2);

    AttributeModel am;
    QSignalSpy modelResetSpy(&am, &AttributeModel::modelReset);
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/LC_attributes.json"));

    QVERIFY(modelResetSpy.isValid());
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(am.rowCount(), 18);

    auto attributeLC_Parameter_No3 = model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No3);
    QVERIFY(attributeLC_Parameter_No3);
    QCOMPARE(attributeLC_Parameter_No3->name(), QStringLiteral("LC_Parameter_No3"));
}

void LaserControlMeasureModelTest::testFindParameter()
{
    LaserControlMeasureModel model{this};

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());

    model.setMeasureTask(s);
    model.setCurrentPreset(2);

    AttributeModel am;
    QSignalSpy modelResetSpy(&am, &AttributeModel::modelReset);
    model.setAttributeModel(&am);
    QVERIFY(modelResetSpy.isValid());
    am.load(QFINDTESTDATA("testdata/LC_attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(am.rowCount(), 18);

    auto param = model.findPresetParameter(model.getParameterSet(), LaserControlPreset::Key::LC_Parameter_No3);
    QVERIFY(!param);

    auto ps = model.getParameterSet();

    auto param2 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No3), QUuid{});
    QVERIFY(param2);

    QCOMPARE(param2, model.findPresetParameter(model.getParameterSet(), LaserControlPreset::Key::LC_Parameter_No3));
}

void LaserControlMeasureModelTest::testUpdateHardwareParameters()
{
    LaserControlMeasureModel model{this};

    QSignalSpy markAsChangedSpy(&model, &LaserControlMeasureModel::markAsChanged);
    QVERIFY(markAsChangedSpy.isValid());

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());

    model.setMeasureTask(s);
    QCOMPARE(markAsChangedSpy.count(), 0);

    AttributeModel am;
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/LC_attributes.json"));
    QSignalSpy modelResetSpy(&am, &AttributeModel::modelReset);
    QVERIFY(modelResetSpy.isValid());
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(am.rowCount(), 18);
    QCOMPARE(markAsChangedSpy.count(), 0);

    model.setPresetEnabled(true);
    QCOMPARE(markAsChangedSpy.count(), 1);
    model.setCurrentPreset(1);
    QCOMPARE(markAsChangedSpy.count(), 2);

    model.setCurrentPreset(1);
    QCOMPARE(markAsChangedSpy.count(), 2);

    QVERIFY(s->hardwareParameters());
    auto& parameters = s->hardwareParameters()->parameters();
    QCOMPARE(parameters.size(), 17);

    std::vector<QUuid> attUuids = {
        QUuid{QByteArrayLiteral("16DFEBA1-0A86-40C5-856D-D8949BEE803B")},
        QUuid{QByteArrayLiteral("DC32C906-0731-445A-A92E-4D222E6A2F34")},
        QUuid{QByteArrayLiteral("3ACDE9A3-8217-4225-9105-67DF1F8F5BA8")},
        QUuid{QByteArrayLiteral("9A59DA4D-1C41-43AA-BCA6-657587C3D038")},
        QUuid{QByteArrayLiteral("0E1E3F32-5A7F-4A4A-A957-91F685F1EA23")},
        QUuid{QByteArrayLiteral("B10FDAB7-5A76-4EA6-9F07-2E3259FCB1FB")},
        QUuid{QByteArrayLiteral("82FD87BF-B3EC-4B0F-855E-A21F480EB04B")},
        QUuid{QByteArrayLiteral("D403DDCF-4182-46FB-A779-F43D28966167")},
        QUuid{QByteArrayLiteral("DD52C706-ECA4-4012-8893-E35C9350F46B")},
        QUuid{QByteArrayLiteral("E4396066-7FEC-4756-82EB-DADABE44D5DD")},
        QUuid{QByteArrayLiteral("406A93EE-D9E7-44D5-979C-391FB47347AA")},
        QUuid{QByteArrayLiteral("6773212B-3C55-4A6F-984B-926996FDAFA7")},
        QUuid{QByteArrayLiteral("F001DBFF-EB99-4B4C-8705-6BDFBD38E1F5")},
        QUuid{QByteArrayLiteral("FE413097-4738-46F5-A3A1-2941D958A90C")},
        QUuid{QByteArrayLiteral("4E77FD69-4B78-4BAD-81C2-12CFA6F79F8D")},
        QUuid{QByteArrayLiteral("237638AB-BAF6-4952-A793-AB3BB5AB091A")}
    };

    auto preset = new LaserControlPreset(this);
    preset->setPower({55, 65, 20, 65, 29, 55, 17, 13});
    preset->setOffset({25, 100, 0, 100, 24, 13, 75, 6});

    for(auto i = 0u; i < attUuids.size(); i++)
    {
        auto attribute = am.findAttribute(attUuids.at(i));
        auto it = std::find_if(parameters.begin(), parameters.end(), [attribute] (auto param) { return param->name() == attribute->name(); });
        QVERIFY(it != parameters.end());
        QCOMPARE((*it)->value().toInt(), preset->getValue(LaserControlPreset::Key(i)));
    }

    auto send_data_attribute = am.findAttribute(QUuid{QByteArrayLiteral("268968BE-2EA0-45C5-A1E4-2E3B31F47EF2")});
    auto send_data_it = std::find_if(parameters.begin(), parameters.end(), [send_data_attribute] (auto param) { return param->name() == send_data_attribute->name(); });
    QVERIFY(send_data_it != parameters.end());
    QCOMPARE((*send_data_it)->value().toBool(), true);

    model.setChannel2Enabled(true);

    auto twoChannelParameters = s->hardwareParameters()->parameters();
    QCOMPARE(twoChannelParameters.size(), 17);

    for(auto i = 0u; i < attUuids.size(); i++)
    {
        auto attribute = am.findAttribute(attUuids.at(i));
        auto it = std::find_if(twoChannelParameters.begin(), twoChannelParameters.end(), [attribute] (auto param) { return param->name() == attribute->name(); });
        QVERIFY(it != twoChannelParameters.end());
        QCOMPARE((*it)->value().toInt(), preset->getValue(LaserControlPreset::Key(i)));
    }
}

void LaserControlMeasureModelTest::testFindPreset()
{
    LaserControlMeasureModel model{this};
    QSignalSpy presetChangedSpy(&model, &LaserControlMeasureModel::currentPresetChanged);
    QVERIFY(presetChangedSpy.isValid());

    QSignalSpy rowsInsertedSpy(&model, &LaserControlMeasureModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());

    model.setMeasureTask(s);
    QCOMPARE(presetChangedSpy.count(), 0);

    AttributeModel am;
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/LC_attributes.json"));
    QSignalSpy modelResetSpy(&am, &AttributeModel::modelReset);
    QVERIFY(modelResetSpy.isValid());
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(am.rowCount(), 18);

    QCOMPARE(model.currentPreset(), -1);

    auto param = model.findPresetParameter(model.getParameterSet(), LaserControlPreset::Key::LC_Parameter_No3);
    QVERIFY(!param);

    auto ps = model.getParameterSet();

    auto param0 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No3), QUuid{});
    QVERIFY(param0);
    param0->setValue(55);

    auto param1 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No4), QUuid{});
    QVERIFY(param1);
    param1->setValue(65);

    auto param2 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No5), QUuid{});
    QVERIFY(param2);
    param2->setValue(20);

    auto param3 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No6), QUuid{});
    QVERIFY(param3);
    param3->setValue(65);

    auto param4 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No11), QUuid{});
    QVERIFY(param4);
    param4->setValue(25);

    auto param5 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No12), QUuid{});
    QVERIFY(param5);
    param5->setValue(100);

    auto param6 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No13), QUuid{});
    QVERIFY(param6);
    param6->setValue(0);

    auto param7 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No14), QUuid{});
    QVERIFY(param7);
    param7->setValue(100);

    model.init();
    QCOMPARE(presetChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.currentPreset(), 0);
    QCOMPARE(model.currentPresetItem()->name(), QStringLiteral("Unsaved Configuration"));

    s->setLaserControlPreset("7d1a636b-5de7-47f9-9e03-a917dc3c2f1d");
    model.init();
    QCOMPARE(presetChangedSpy.count(), 3);
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.currentPreset(), 1);
    QCOMPARE(model.currentPresetItem()->name(), QStringLiteral("Default 2"));

    param0->setValue(15);
    model.init();
    QCOMPARE(presetChangedSpy.count(), 5);
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.currentPreset(), 0);
    QCOMPARE(model.currentPresetItem()->name(), QStringLiteral("Unsaved Configuration"));
}

void LaserControlMeasureModelTest::testFindPresetTwoChannels()
{
    LaserControlMeasureModel model{this};
    QSignalSpy presetChangedSpy(&model, &LaserControlMeasureModel::currentPresetChanged);
    QVERIFY(presetChangedSpy.isValid());

    QSignalSpy rowsInsertedSpy(&model, &LaserControlMeasureModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());

    model.setChannel2Enabled(true);
    model.setMeasureTask(s);
    QCOMPARE(presetChangedSpy.count(), 0);

    AttributeModel am;
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/LC_attributes.json"));
    QSignalSpy modelResetSpy(&am, &AttributeModel::modelReset);
    QVERIFY(modelResetSpy.isValid());
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(am.rowCount(), 18);

    QCOMPARE(model.currentPreset(), -1);

    auto param = model.findPresetParameter(model.getParameterSet(), LaserControlPreset::Key::LC_Parameter_No3);
    QVERIFY(!param);

    auto ps = model.getParameterSet();

    auto param0 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No3), QUuid{});
    QVERIFY(param0);
    param0->setValue(55);

    auto param1 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No4), QUuid{});
    QVERIFY(param1);
    param1->setValue(65);

    auto param2 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No5), QUuid{});
    QVERIFY(param2);
    param2->setValue(20);

    auto param3 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No6), QUuid{});
    QVERIFY(param3);
    param3->setValue(65);

    auto param4 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No11), QUuid{});
    QVERIFY(param4);
    param4->setValue(25);

    auto param5 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No12), QUuid{});
    QVERIFY(param5);
    param5->setValue(100);

    auto param6 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No13), QUuid{});
    QVERIFY(param6);
    param6->setValue(0);

    auto param7 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No14), QUuid{});
    QVERIFY(param7);
    param7->setValue(100);

    model.init();
    QCOMPARE(presetChangedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.currentPreset(), -1);

    auto param8 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No16), QUuid{});
    QVERIFY(param8);
    param8->setValue(29);

    auto param9 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No17), QUuid{});
    QVERIFY(param9);
    param9->setValue(55);

    auto param10 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No18), QUuid{});
    QVERIFY(param10);
    param10->setValue(17);

    auto param11 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No19), QUuid{});
    QVERIFY(param11);
    param11->setValue(13);

    auto param12 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No24), QUuid{});
    QVERIFY(param12);
    param12->setValue(24);

    auto param13 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No25), QUuid{});
    QVERIFY(param13);
    param13->setValue(13);

    auto param14 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No26), QUuid{});
    QVERIFY(param14);
    param14->setValue(75);

    auto param15 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No27), QUuid{});
    QVERIFY(param15);
    param15->setValue(6);

    model.init();
    QCOMPARE(presetChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.currentPreset(), 0);
    QCOMPARE(model.currentPresetItem()->name(), QStringLiteral("Unsaved Configuration"));

    s->setLaserControlPreset("7d1a636b-5de7-47f9-9e03-a917dc3c2f1d");
    model.init();
    QCOMPARE(presetChangedSpy.count(), 3);
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.currentPreset(), 1);
    QCOMPARE(model.currentPresetItem()->name(), QStringLiteral("Default 2"));

    param0->setValue(15);
    model.init();
    QCOMPARE(presetChangedSpy.count(), 5);
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.currentPreset(), 0);
    QCOMPARE(model.currentPresetItem()->name(), QStringLiteral("Unsaved Configuration"));
}

void LaserControlMeasureModelTest::testCreateCurrentPreset()
{
    LaserControlMeasureModel model{this};
    QSignalSpy presetChangedSpy(&model, &LaserControlMeasureModel::currentPresetChanged);
    QVERIFY(presetChangedSpy.isValid());

    QSignalSpy rowsInsertedSpy(&model, &LaserControlMeasureModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());

    model.setMeasureTask(s);
    QCOMPARE(presetChangedSpy.count(), 0);

    AttributeModel am;
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/LC_attributes.json"));
    QSignalSpy modelResetSpy(&am, &AttributeModel::modelReset);
    QVERIFY(modelResetSpy.isValid());
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(am.rowCount(), 18);

    QCOMPARE(model.currentPreset(), -1);

    auto ps = model.getParameterSet();

    auto param0 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No3), QUuid{});
    QVERIFY(param0);
    param0->setValue(55);

    auto param1 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No4), QUuid{});
    QVERIFY(param1);
    param1->setValue(65);

    auto param2 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No5), QUuid{});
    QVERIFY(param2);
    param2->setValue(20);

    auto param3 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No6), QUuid{});
    QVERIFY(param3);
    param3->setValue(65);

    auto param4 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No11), QUuid{});
    QVERIFY(param4);
    param4->setValue(25);

    auto param5 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No12), QUuid{});
    QVERIFY(param5);
    param5->setValue(100);

    auto param6 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No13), QUuid{});
    QVERIFY(param6);
    param6->setValue(0);

    auto param7 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No14), QUuid{});
    QVERIFY(param7);
    param7->setValue(100);

    model.createCurrentPreset();
    QCOMPARE(presetChangedSpy.count(), 1);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(model.currentPreset(), 0);

    const auto current = model.index(0, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(current);
    QCOMPARE(current->name(), QStringLiteral("Unsaved Configuration"));
    QCOMPARE(current->power(0), 55);
    QCOMPARE(current->power(1), 65);
    QCOMPARE(current->power(2), 20);
    QCOMPARE(current->power(3), 65);
    QCOMPARE(current->offset(0), 25);
    QCOMPARE(current->offset(1), 100);
    QCOMPARE(current->offset(2), 0);
    QCOMPARE(current->offset(3), 100);
}

void LaserControlMeasureModelTest::testRemoveCurrentPreset()
{
    LaserControlMeasureModel model{this};
    QSignalSpy presetChangedSpy(&model, &LaserControlMeasureModel::currentPresetChanged);
    QVERIFY(presetChangedSpy.isValid());

    QSignalSpy rowsInsertedSpy(&model, &LaserControlMeasureModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());

    QSignalSpy rowsRemovedSpy(&model, &LaserControlMeasureModel::rowsRemoved);
    QVERIFY(rowsRemovedSpy.isValid());

    QSignalSpy modelResetSpy(&model, &LaserControlMeasureModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    QCOMPARE(rowsInsertedSpy.count(), 0);
    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);
    QCOMPARE(modelResetSpy.count(), 2);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());

    model.setMeasureTask(s);
    QCOMPARE(presetChangedSpy.count(), 0);

    AttributeModel am;
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/LC_attributes.json"));
    QSignalSpy atModelResetSpy(&am, &AttributeModel::modelReset);
    QVERIFY(atModelResetSpy.isValid());
    QVERIFY(atModelResetSpy.wait());
    QCOMPARE(am.rowCount(), 18);
    QCOMPARE(model.currentPreset(), -1);
    QCOMPARE(model.rowCount(), 2);

    model.removeCurrentPreset();

    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(rowsRemovedSpy.count(), 0);

    auto ps = model.getParameterSet();

    auto param0 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No3), QUuid{});
    QVERIFY(param0);
    param0->setValue(15);

    auto param1 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No4), QUuid{});
    QVERIFY(param1);
    param1->setValue(65);

    auto param2 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No5), QUuid{});
    QVERIFY(param2);
    param2->setValue(20);

    auto param3 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No6), QUuid{});
    QVERIFY(param3);
    param3->setValue(65);

    auto param4 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No11), QUuid{});
    QVERIFY(param4);
    param4->setValue(25);

    auto param5 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No12), QUuid{});
    QVERIFY(param5);
    param5->setValue(100);

    auto param6 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No13), QUuid{});
    QVERIFY(param6);
    param6->setValue(0);

    auto param7 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No14), QUuid{});
    QVERIFY(param7);
    param7->setValue(100);

    model.init();
    QCOMPARE(presetChangedSpy.count(), 1);
    QCOMPARE(model.currentPreset(), 0);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(rowsRemovedSpy.count(), 0);

    model.removeCurrentPreset();
    QCOMPARE(model.currentPreset(), 0);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(rowsRemovedSpy.count(), 1);
}

void LaserControlMeasureModelTest::testLoad()
{
    LaserControlMeasureModel model{this};
    QSignalSpy resetSpy(&model, &LaserControlMeasureModel::modelReset);
    QVERIFY(resetSpy.isValid());
    QSignalSpy rowsInsertedSpy(&model, &LaserControlMeasureModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());
    QSignalSpy loadingFinishedSpy(&model, &LaserControlMeasureModel::loadingFinished);
    QVERIFY(loadingFinishedSpy.isValid());

    QCOMPARE(model.rowCount(), 0);

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(resetSpy.count(), 2);
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(loadingFinishedSpy.count(), 1);

    const auto preset1 = model.index(0, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(preset1);
    QCOMPARE(preset1->uuid(), QUuid("14f819a6-e946-4e55-98e7-727c8f8d7754"));
    QCOMPARE(preset1->name(), QStringLiteral("Default 1"));
    QCOMPARE(preset1->power(0), 53);
    QCOMPARE(preset1->power(1), 60);
    QCOMPARE(preset1->power(2), 20);
    QCOMPARE(preset1->power(3), 60);
    QCOMPARE(preset1->offset(0), 25);
    QCOMPARE(preset1->offset(1), 100);
    QCOMPARE(preset1->offset(2), 0);
    QCOMPARE(preset1->offset(3), 100);

    const auto preset2 = model.index(1, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(preset2);
    QCOMPARE(preset2->uuid(), QUuid("7d1a636b-5de7-47f9-9e03-a917dc3c2f1d"));
    QCOMPARE(preset2->name(), QStringLiteral("Default 2"));
    QCOMPARE(preset2->power(0), 55);
    QCOMPARE(preset2->power(1), 65);
    QCOMPARE(preset2->power(2), 20);
    QCOMPARE(preset2->power(3), 65);
    QCOMPARE(preset2->offset(0), 25);
    QCOMPARE(preset2->offset(1), 100);
    QCOMPARE(preset2->offset(2), 0);
    QCOMPARE(preset2->offset(3), 100);
}

void LaserControlMeasureModelTest::testPresetEnabled()
{
    LaserControlMeasureModel model{this};

    QSignalSpy presetEnabledChangedSpy(&model, &LaserControlMeasureModel::presetEnabledChanged);
    QVERIFY(presetEnabledChangedSpy.isValid());

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    Product p{QUuid::createUuid()};
    auto s = p.createSeamSeries();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());

    model.setMeasureTask(s);
    QCOMPARE(model.presetEnabled(), false);
    QCOMPARE(presetEnabledChangedSpy.count(), 0);

    AttributeModel am;
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/LC_attributes.json"));
    QSignalSpy modelResetSpy(&am, &AttributeModel::modelReset);
    QVERIFY(modelResetSpy.isValid());
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(am.rowCount(), 18);

    model.setCurrentPreset(1);
    QCOMPARE(model.currentPreset(), 1);
    QCOMPARE(s->laserControlPreset(), QUuid());
    QVERIFY(s->hardwareParameters());
    auto parameters = s->hardwareParameters()->parameters();
    QCOMPARE(parameters.size(), 0);

    model.setPresetEnabled(true);
    QCOMPARE(model.presetEnabled(), true);
    QCOMPARE(presetEnabledChangedSpy.count(), 1);

    parameters = s->hardwareParameters()->parameters();
    QCOMPARE(parameters.size(), 17);

    std::vector<QUuid> attUuids = {
        QUuid{QByteArrayLiteral("16DFEBA1-0A86-40C5-856D-D8949BEE803B")},
        QUuid{QByteArrayLiteral("DC32C906-0731-445A-A92E-4D222E6A2F34")},
        QUuid{QByteArrayLiteral("3ACDE9A3-8217-4225-9105-67DF1F8F5BA8")},
        QUuid{QByteArrayLiteral("9A59DA4D-1C41-43AA-BCA6-657587C3D038")},
        QUuid{QByteArrayLiteral("0E1E3F32-5A7F-4A4A-A957-91F685F1EA23")},
        QUuid{QByteArrayLiteral("B10FDAB7-5A76-4EA6-9F07-2E3259FCB1FB")},
        QUuid{QByteArrayLiteral("82FD87BF-B3EC-4B0F-855E-A21F480EB04B")},
        QUuid{QByteArrayLiteral("D403DDCF-4182-46FB-A779-F43D28966167")},
        QUuid{QByteArrayLiteral("DD52C706-ECA4-4012-8893-E35C9350F46B")},
        QUuid{QByteArrayLiteral("E4396066-7FEC-4756-82EB-DADABE44D5DD")},
        QUuid{QByteArrayLiteral("406A93EE-D9E7-44D5-979C-391FB47347AA")},
        QUuid{QByteArrayLiteral("6773212B-3C55-4A6F-984B-926996FDAFA7")},
        QUuid{QByteArrayLiteral("F001DBFF-EB99-4B4C-8705-6BDFBD38E1F5")},
        QUuid{QByteArrayLiteral("FE413097-4738-46F5-A3A1-2941D958A90C")},
        QUuid{QByteArrayLiteral("4E77FD69-4B78-4BAD-81C2-12CFA6F79F8D")},
        QUuid{QByteArrayLiteral("237638AB-BAF6-4952-A793-AB3BB5AB091A")}
    };

    auto preset = new LaserControlPreset(this);
    preset->setPower({55, 65, 20, 65, 29, 55, 17, 13});
    preset->setOffset({25, 100, 0, 100, 24, 13, 75, 6});

    for(auto i = 0u; i < attUuids.size(); i++)
    {
        auto attribute = am.findAttribute(attUuids.at(i));
        auto it = std::find_if(parameters.begin(), parameters.end(), [attribute] (auto param) { return param->name() == attribute->name(); });
        QVERIFY(it != parameters.end());
        QCOMPARE((*it)->value().toInt(), preset->getValue(LaserControlPreset::Key(i)));
    }

    auto send_data_attribute = am.findAttribute(QUuid{QByteArrayLiteral("268968BE-2EA0-45C5-A1E4-2E3B31F47EF2")});
    auto send_data_it = std::find_if(parameters.begin(), parameters.end(), [send_data_attribute] (auto param) { return param->name() == send_data_attribute->name(); });
    QVERIFY(send_data_it != parameters.end());
    QCOMPARE((*send_data_it)->value().toBool(), true);

    auto s2 = p.createSeam();
    QVERIFY(s2);
    QVERIFY(!s2->hardwareParameters());

    model.setMeasureTask(s2);
    QCOMPARE(model.presetEnabled(), false);
    QCOMPARE(presetEnabledChangedSpy.count(), 2);

    model.setMeasureTask(s);
    QCOMPARE(model.presetEnabled(), true);
    QCOMPARE(presetEnabledChangedSpy.count(), 3);
    parameters = s->hardwareParameters()->parameters();
    QCOMPARE(parameters.size(), 17);

    model.setPresetEnabled(false);
    QCOMPARE(model.presetEnabled(), false);
    QCOMPARE(presetEnabledChangedSpy.count(), 4);
    parameters = s->hardwareParameters()->parameters();
    QCOMPARE(parameters.size(), 0);
}

void LaserControlMeasureModelTest::testDelay()
{
    LaserControlMeasureModel model{this};

    QSignalSpy delayEnabledChangedSpy(&model, &LaserControlMeasureModel::delayEnabledChanged);
    QVERIFY(delayEnabledChangedSpy.isValid());

    Product p{QUuid::createUuid()};
    auto s = p.createSeamSeries();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());

    model.setMeasureTask(s);
    QCOMPARE(model.delayEnabled(), false);
    QCOMPARE(delayEnabledChangedSpy.count(), 0);

    AttributeModel am;
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/LC_attributes.json"));
    QSignalSpy modelResetSpy(&am, &AttributeModel::modelReset);
    QVERIFY(modelResetSpy.isValid());
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(am.rowCount(), 18);

    model.setDelay(11);
    QCOMPARE(model.delay(), 11);
    QVERIFY(s->hardwareParameters());
    auto parameters = s->hardwareParameters()->parameters();
    QCOMPARE(parameters.size(), 0);

    model.setDelayEnabled(true);
    QCOMPARE(model.delayEnabled(), true);
    QCOMPARE(delayEnabledChangedSpy.count(), 1);

    parameters = s->hardwareParameters()->parameters();
    QCOMPARE(parameters.size(), 2);

    std::vector<QUuid> attUuids = {
        QUuid{QByteArrayLiteral("8AD9BE12-4026-45BF-B106-5CC487EB8B1D")},
        QUuid{QByteArrayLiteral("268968BE-2EA0-45C5-A1E4-2E3B31F47EF2")}
    };

    auto attribute0 = am.findAttribute(attUuids.at(0));
    auto it_0 = std::find_if(parameters.begin(), parameters.end(), [attribute0] (auto param) { return param->name() == attribute0->name(); });
    QVERIFY(it_0 != parameters.end());
    QCOMPARE((*it_0)->value().toInt(), 11);

    auto attribute1 = am.findAttribute(attUuids.at(1));
    auto it_1 = std::find_if(parameters.begin(), parameters.end(), [attribute1] (auto param) { return param->name() == attribute1->name(); });
    QVERIFY(it_1 != parameters.end());
    QCOMPARE((*it_1)->value().toBool(), true);

    auto s2 = p.createSeam();
    QVERIFY(s2);
    QVERIFY(!s2->hardwareParameters());

    model.setMeasureTask(s2);
    QCOMPARE(model.delayEnabled(), false);
    QCOMPARE(delayEnabledChangedSpy.count(), 2);

    model.setMeasureTask(s);
    QCOMPARE(model.delayEnabled(), true);
    QCOMPARE(delayEnabledChangedSpy.count(), 3);
    parameters = s->hardwareParameters()->parameters();
    QCOMPARE(parameters.size(), 2);

    model.setDelayEnabled(false);
    QCOMPARE(model.delayEnabled(), false);
    QCOMPARE(delayEnabledChangedSpy.count(), 4);
    parameters = s->hardwareParameters()->parameters();
    QCOMPARE(parameters.size(), 0);
}

void LaserControlMeasureModelTest::testChannelTwo()
{
    LaserControlMeasureModel model{this};

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    QSignalSpy channel2EnabledChangedSpy(&model, &LaserControlMeasureModel::channel2EnabledChanged);
    QVERIFY(channel2EnabledChangedSpy.isValid());

    QVERIFY(!model.channel2Enabled());

    model.setChannel2Enabled(false);
    QCOMPARE(channel2EnabledChangedSpy.count(), 0);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());

    model.setMeasureTask(s);

    AttributeModel am;
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/LC_attributes.json"));
    QSignalSpy modelResetSpy(&am, &AttributeModel::modelReset);
    QVERIFY(modelResetSpy.isValid());
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(am.rowCount(), 18);

    model.setPresetEnabled(true);
    model.setCurrentPreset(1);

    QVERIFY(s->hardwareParameters());
    QCOMPARE(s->hardwareParameters()->parameters().size(), 17);

    model.setChannel2Enabled(true);
    QCOMPARE(model.channel2Enabled(), true);
    QCOMPARE(channel2EnabledChangedSpy.count(), 1);

    model.setChannel2Enabled(true);
    QCOMPARE(channel2EnabledChangedSpy.count(), 1);

    QCOMPARE(s->hardwareParameters()->parameters().size(), 17);

    model.setChannel2Enabled(false);
    QCOMPARE(channel2EnabledChangedSpy.count(), 2);

    QCOMPARE(s->hardwareParameters()->parameters().size(), 17);
}

QTEST_GUILESS_MAIN(LaserControlMeasureModelTest)
#include "laserControlMeasureModelTest.moc"
