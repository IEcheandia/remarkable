#include <QTest>
#include <QSignalSpy>

#include "../src/laserControlModel.h"
#include "../src/abstractLaserControlModel.h"
#include "../src/productController.h"
#include "laserControlPreset.h"
#include "attribute.h"
#include "attributeModel.h"
#include "parameter.h"
#include "parameterSet.h"
#include "seam.h"
#include "product.h"
#include "productModel.h"

using precitec::gui::LaserControlModel;
using precitec::gui::AbstractLaserControlModel;
using precitec::gui::ProductController;
using precitec::storage::Attribute;
using precitec::storage::AttributeModel;
using precitec::storage::Parameter;
using precitec::storage::Seam;
using precitec::storage::Product;
using precitec::storage::ProductModel;
using precitec::storage::LaserControlPreset;

class LaserControlModelTest : public QObject
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
    void testSetEditing();
    void testSetAttributeModel();
    void testSetCurrentPreset();
    void testSetLaserControlPresetDir();
    void testSetProductController();
    void testFindAttribute();
    void testFindParameter();
    void testEnableDisablePresets();
    void testFindPreset();
    void testFindPresetIndex();
    void testCreateNewPreset();
    void testDeletePreset();
    void testUpdateHardwareParameters();
    void testLoad();
};

void LaserControlModelTest::testCtor()
{
    LaserControlModel model{this};
    QVERIFY(!model.productController());
    QVERIFY(!model.attributeModel());
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.currentPreset(), -1);
    QCOMPARE(model.isEditing(), false);
    QCOMPARE(model.laserControlPresetDir(), QString());
    QVERIFY(!model.channel2Enabled());
}

void LaserControlModelTest::testRoleNames()
{
    LaserControlModel model{this};
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 6);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("name"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("preset"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("uuid"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("channel1Power"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("channel2Power"));
    QCOMPARE(roleNames[Qt::UserRole + 4], QByteArrayLiteral("isCurrent"));
}

void LaserControlModelTest::testDisplayRole_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("name");

    QTest::newRow("0")  <<  0 << QStringLiteral("Default 1");
    QTest::newRow("1")  <<  1 << QStringLiteral("Default 2");
}

void LaserControlModelTest::testDisplayRole()
{
    LaserControlModel model{this};

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data().toString(), "name");
}

void LaserControlModelTest::testPreset_data()
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

void LaserControlModelTest::testPreset()
{
    LaserControlModel model{this};

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

void LaserControlModelTest::testUuid_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QUuid>("uuid");

    QTest::newRow("0")  <<  0 << QUuid("14f819a6-e946-4e55-98e7-727c8f8d7754");
    QTest::newRow("1")  <<  1 << QUuid("7d1a636b-5de7-47f9-9e03-a917dc3c2f1d");
}

void LaserControlModelTest::testUuid()
{
    LaserControlModel model{this};

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 1).toUuid(), "uuid");
}

void LaserControlModelTest::testSetEditing()
{
    LaserControlModel model{this};
    QSignalSpy editingChangedSpy(&model, &LaserControlModel::editingChanged);
    QVERIFY(editingChangedSpy.isValid());

    QCOMPARE(model.isEditing(), false);

    model.setEditing(false);
    QCOMPARE(editingChangedSpy.count(), 0);

    model.setEditing(true);
    QCOMPARE(model.isEditing(), true);
    QCOMPARE(editingChangedSpy.count(), 1);

    model.setEditing(true);
    QCOMPARE(editingChangedSpy.count(), 1);
}

void LaserControlModelTest::testSetAttributeModel()
{
    LaserControlModel model{this};
    QSignalSpy attributeModelChangedSpy(&model, &LaserControlModel::attributeModelChanged);
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

void LaserControlModelTest::testSetCurrentPreset()
{
    LaserControlModel model{this};
    QSignalSpy presetChangedSpy(&model, &LaserControlModel::currentPresetChanged);
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

void LaserControlModelTest::testSetLaserControlPresetDir()
{
    LaserControlModel model{this};
    QSignalSpy dirChangedSpy(&model, &LaserControlModel::laserControlPresetDirChanged);
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

void LaserControlModelTest::testSetProductController()
{
    LaserControlModel model{this};
    QSignalSpy productControllerChangedSpy(&model, &LaserControlModel::productControllerChanged);
    QVERIFY(productControllerChangedSpy.isValid());

    std::unique_ptr<ProductController> controller = std::make_unique<ProductController>();
    model.setProductController(controller.get());
    QCOMPARE(model.productController(), controller.get());
    QCOMPARE(productControllerChangedSpy.count(), 1);

    model.setProductController(controller.get());
    QCOMPARE(productControllerChangedSpy.count(), 1);

    controller.reset();
    QCOMPARE(model.productController(), nullptr);
    QCOMPARE(productControllerChangedSpy.count(), 2);
}

void LaserControlModelTest::testFindAttribute()
{
    LaserControlModel model{this};
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

void LaserControlModelTest::testFindParameter()
{
    LaserControlModel model{this};

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());
    s->createHardwareParameters();
    QVERIFY(s->hardwareParameters());

    AttributeModel am;
    QSignalSpy modelResetSpy(&am, &AttributeModel::modelReset);
    model.setAttributeModel(&am);
    QVERIFY(modelResetSpy.isValid());
    am.load(QFINDTESTDATA("testdata/LC_attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(am.rowCount(), 18);

    auto ps = s->hardwareParameters();

    auto param = model.findPresetParameter(ps, LaserControlPreset::Key::LC_Parameter_No3);
    QVERIFY(!param);

    auto param2 = ps->createParameter(QUuid::createUuid(), model.findPresetAttribute(LaserControlPreset::Key::LC_Parameter_No3), QUuid{});
    QVERIFY(param2);

    QCOMPARE(param2, model.findPresetParameter(ps, LaserControlPreset::Key::LC_Parameter_No3));
}

void LaserControlModelTest::testEnableDisablePresets()
{
    LaserControlModel model{this};
    QSignalSpy editingChangedSpy(&model, &LaserControlModel::editingChanged);
    QVERIFY(editingChangedSpy.isValid());

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    QCOMPARE(model.isEditing(), false);

    const auto preset1 = model.index(0, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(preset1);
    QCOMPARE(preset1->enabled(), true);

    const auto preset2 = model.index(1, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(preset2);
    QCOMPARE(preset2->enabled(), true);

    model.disablePresets();
    QCOMPARE(editingChangedSpy.count(), 1);

    QCOMPARE(preset1->enabled(), false);
    QCOMPARE(preset2->enabled(), false);
    QCOMPARE(model.isEditing(), true);

    model.enablePresets();
    QCOMPARE(editingChangedSpy.count(), 2);

    QCOMPARE(preset1->enabled(), true);
    QCOMPARE(preset2->enabled(), true);
    QCOMPARE(model.isEditing(), false);

    preset2->setState(LaserControlPreset::State::Edit);
    QCOMPARE(editingChangedSpy.count(), 3);

    QCOMPARE(preset1->enabled(), false);
    QCOMPARE(preset2->enabled(), true);
    QCOMPARE(model.isEditing(), true);

    preset2->setState(LaserControlPreset::State::Default);
    QCOMPARE(editingChangedSpy.count(), 4);

    QCOMPARE(preset1->enabled(), true);
    QCOMPARE(preset2->enabled(), true);
    QCOMPARE(model.isEditing(), false);
}

void LaserControlModelTest::testFindPreset()
{
    LaserControlModel model{this};

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    auto found = model.findPreset(QUuid("14f819a6-e946-4e55-98e7-727c8f8d7754"));
    QCOMPARE(found, true);

    auto notfound = model.findPreset(QUuid("14f819a6-e946-4e55-98e7-727c85437754"));
    QCOMPARE(notfound, false);
}

void LaserControlModelTest::testFindPresetIndex()
{
    LaserControlModel model{this};

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    auto preset1Index = model.findPresetIndex(QUuid("14f819a6-e946-4e55-98e7-727c8f8d7754"));
    QCOMPARE(preset1Index, 0);

    auto notfound = model.findPresetIndex(QUuid("14f819a6-e946-4e55-98e7-727c85437754"));
    QCOMPARE(notfound, -1);
}

void LaserControlModelTest::testCreateNewPreset()
{
    LaserControlModel model{this};
    QSignalSpy editingChangedSpy(&model, &LaserControlModel::editingChanged);
    QVERIFY(editingChangedSpy.isValid());
    QSignalSpy rowsInsertedSpy(&model, &LaserControlModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());
    QSignalSpy resetModelSpy(&model, &LaserControlModel::modelReset);
    QVERIFY(resetModelSpy.isValid());
    QSignalSpy presetChangedSpy(&model, &LaserControlModel::currentPresetChanged);
    QVERIFY(presetChangedSpy.isValid());

    QTemporaryDir dir;
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/laser_control_data/legacy_preset_1.json"), dir.filePath(QStringLiteral("legacy_preset_1.json"))));
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/laser_control_data/preset_2.json"), dir.filePath(QStringLiteral("preset2.json"))));

    model.setLaserControlPresetDir(dir.path());

    QCOMPARE(model.isEditing(), false);
    QCOMPARE(model.currentPreset(), -1);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(editingChangedSpy.count(), 0);
    QCOMPARE(resetModelSpy.count(), 2);
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(presetChangedSpy.count(), 0);

    const auto preset1 = model.index(0, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(preset1);
    QCOMPARE(preset1->enabled(), true);

    const auto preset2 = model.index(1, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(preset2);
    QCOMPARE(preset2->enabled(), true);

    model.createNewPreset();
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.isEditing(), true);
    QCOMPARE(editingChangedSpy.count(), 1);
    QCOMPARE(resetModelSpy.count(), 2);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(presetChangedSpy.count(), 1);

    const auto newPreset = model.index(2, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(newPreset);
    QCOMPARE(newPreset->enabled(), true);
    QCOMPARE(newPreset->name(), QStringLiteral("New Preset"));
    QCOMPARE(newPreset->state(), LaserControlPreset::State::New);
    QCOMPARE(preset1->enabled(), false);
    QCOMPARE(preset2->enabled(), false);

    QCOMPARE(model.currentPreset(), 2);
}

void LaserControlModelTest::testDeletePreset()
{
    LaserControlModel model{this};
    QSignalSpy editingChangedSpy(&model, &LaserControlModel::editingChanged);
    QVERIFY(editingChangedSpy.isValid());
    QSignalSpy rowsRemovedSpy(&model, &LaserControlModel::rowsRemoved);
    QVERIFY(rowsRemovedSpy.isValid());
    QSignalSpy presetChangedSpy(&model, &LaserControlModel::currentPresetChanged);
    QVERIFY(presetChangedSpy.isValid());

    QTemporaryDir dir;
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/laser_control_data/legacy_preset_1.json"), dir.filePath(QStringLiteral("legacy_preset_1.json"))));
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/laser_control_data/preset_2.json"), dir.filePath(QStringLiteral("preset2.json"))));
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/laser_control_data/legacy_preset_1.json"), dir.filePath(QStringLiteral("preset3.json"))));

    model.setLaserControlPresetDir(dir.path());

    QCOMPARE(model.isEditing(), false);
    QCOMPARE(model.currentPreset(), -1);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(editingChangedSpy.count(), 0);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(presetChangedSpy.count(), 0);

    const auto preset1 = model.index(0, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(preset1);
    QCOMPARE(preset1->enabled(), true);
    QCOMPARE(preset1->name(), QStringLiteral("Default 1"));

    const auto preset2 = model.index(1, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(preset2);
    QCOMPARE(preset2->enabled(), true);
    QCOMPARE(preset2->name(), QStringLiteral("Default 1"));

    const auto preset3 = model.index(2, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(preset3);
    QCOMPARE(preset3->enabled(), true);
    QCOMPARE(preset3->name(), QStringLiteral("Default 2"));

    model.setCurrentPreset(1);
    QCOMPARE(presetChangedSpy.count(), 1);

    preset2->setState(LaserControlPreset::State::Edit);
    QCOMPARE(editingChangedSpy.count(), 1);
    QCOMPARE(model.isEditing(), true);
    QCOMPARE(preset1->enabled(), false);
    QCOMPARE(preset2->enabled(), true);
    QCOMPARE(preset3->enabled(), false);

    model.deletePreset(nullptr);
    QCOMPARE(editingChangedSpy.count(), 1);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(presetChangedSpy.count(), 1);

    model.deletePreset(preset2);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(editingChangedSpy.count(), 2);
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(presetChangedSpy.count(), 2);
    QCOMPARE(model.currentPreset(), -1);

    const auto p1 = model.index(0, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(p1);
    QCOMPARE(p1->enabled(), true);
    QCOMPARE(p1->name(), QStringLiteral("Default 1"));

    const auto p2 = model.index(1, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(p2);
    QCOMPARE(p2->enabled(), true);
    QCOMPARE(p2->name(), QStringLiteral("Default 2"));

    const auto p3 = model.index(2, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(!p3);
}

void LaserControlModelTest::testUpdateHardwareParameters()
{
    LaserControlModel model{this};
    QSignalSpy modelResetSpy(&model, &LaserControlModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    auto productModel = new ProductModel{this};
    QVERIFY(productModel);
    QSignalSpy productModelResetSpy(productModel, &ProductModel::modelReset);
    QVERIFY(productModelResetSpy.isValid());

    QTemporaryDir dir;
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/product_data/product_1.json"), dir.filePath(QStringLiteral("product1.json"))));
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/product_data/product_2.json"), dir.filePath(QStringLiteral("product2.json"))));
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/product_data/product_3.json"), dir.filePath(QStringLiteral("product3.json"))));

    QVERIFY(!productModel->defaultProduct());
    productModel->loadProducts(dir.path());
    QCOMPARE(productModelResetSpy.count(), 1);
    QCOMPARE(productModel->rowCount(), 3);

    std::unique_ptr<ProductController> controller = std::make_unique<ProductController>();

    QVERIFY(controller);
    QSignalSpy controllerChangedSpy(controller.get(), &ProductController::hasChangesChanged);
    QVERIFY(controllerChangedSpy.isValid());
    controller->setProductModel(productModel);

    auto attributeModel = new AttributeModel{this};
    model.setAttributeModel(attributeModel);
    attributeModel->load(QFINDTESTDATA("testdata/LC_attributes.json"));
    QSignalSpy attributeModelResetSpy(attributeModel, &AttributeModel::modelReset);
    QVERIFY(attributeModelResetSpy.isValid());
    QVERIFY(attributeModelResetSpy.wait());
    QCOMPARE(attributeModel->rowCount(), 18);

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

    const auto preset1 = new LaserControlPreset(this);
    preset1->setPower({1, 2, 3, 4, 9, 10, 11, 12});
    preset1->setOffset({5, 6, 7, 8, 13, 14, 15, 16});

    const auto preset2 = new LaserControlPreset(this);
    preset2->setPower({11, 12, 13, 14, 19, 20, 21, 22});
    preset2->setOffset({15, 16, 17, 18, 23, 24, 25, 26});

    for (auto product : productModel->products())
    {
        const auto seams = product->allSeams();
        for (auto s : seams)
        {
            auto seam = s.value<precitec::storage::Seam*>();
            auto parameters = seam->hardwareParameters()->parameters();

            for (auto i = 0u; i < attUuids.size(); i++)
            {
                auto attribute = attributeModel->findAttribute(attUuids.at(i));
                auto it = std::find_if(parameters.begin(), parameters.end(), [attribute] (auto param) { return param->name() == attribute->name(); });
                QVERIFY(it != parameters.end());

                if (seam->laserControlPreset() == QUuid("7d1a636b-5de7-47f9-9e03-a917dc3c2f1d"))
                {
                    QCOMPARE((*it)->value().toInt(), preset1->getValue(LaserControlPreset::Key(i)));
                }
                if (seam->laserControlPreset() == QUuid("14f819a6-e946-4e55-98e7-727c8f8d7754"))
                {
                    QCOMPARE((*it)->value().toInt(), preset2->getValue(LaserControlPreset::Key(i)));
                }
            }

            auto send_data_attribute = attributeModel->findAttribute(QUuid{QByteArrayLiteral("268968BE-2EA0-45C5-A1E4-2E3B31F47EF2")});
            auto send_data_it = std::find_if(parameters.begin(), parameters.end(), [send_data_attribute] (auto param) { return param->name() == send_data_attribute->name(); });
            QVERIFY(send_data_it != parameters.end());
            QCOMPARE((*send_data_it)->value().toBool(), true);
        }
    }

    model.setProductController(controller.get());

    QTemporaryDir lc_dir;
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/laser_control_data/legacy_preset_1.json"), lc_dir.filePath(QStringLiteral("legacy_preset_1.json"))));
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/laser_control_data/preset_2.json"), lc_dir.filePath(QStringLiteral("preset2.json"))));

    model.setLaserControlPresetDir(lc_dir.path());
    QCOMPARE(modelResetSpy.count(), 2);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(controllerChangedSpy.count(), 0);

    const auto preset3 = model.index(0, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(preset3);

    const auto preset4 = model.index(1, 0).data(Qt::UserRole).value<LaserControlPreset*>();
    QVERIFY(preset4);

    model.updateHardwareParameters(preset3->uuid());
    QCOMPARE(controllerChangedSpy.count(), 2);

    productModel->loadProducts(dir.path());

    for (auto product : productModel->products())
    {
        const auto seams = product->allSeams();
        for (auto s : seams)
        {
            auto seam = s.value<precitec::storage::Seam*>();
            auto parameters = seam->hardwareParameters()->parameters();

            for(auto i = 0u; i < attUuids.size(); i++)
            {
                auto attribute = attributeModel->findAttribute(attUuids.at(i));
                auto it = std::find_if(parameters.begin(), parameters.end(), [attribute] (auto param) { return param->name() == attribute->name(); });
                QVERIFY(it != parameters.end());

                if (seam->laserControlPreset() == QUuid("7d1a636b-5de7-47f9-9e03-a917dc3c2f1d"))
                {
                    QCOMPARE((*it)->value().toInt(), preset1->getValue(LaserControlPreset::Key(i)));
                }
                if (seam->laserControlPreset() == QUuid("14f819a6-e946-4e55-98e7-727c8f8d7754"))
                {
                    QCOMPARE((*it)->value().toInt(), preset3->getValue(LaserControlPreset::Key(i)));
                }
            }
        }
    }

    model.updateHardwareParameters(preset4->uuid());
    QCOMPARE(controllerChangedSpy.count(), 4);

    productModel->loadProducts(dir.path());

    for (auto product : productModel->products())
    {
        const auto seams = product->allSeams();
        for (auto s : seams)
        {
            auto seam = s.value<precitec::storage::Seam*>();
            auto parameters = seam->hardwareParameters()->parameters();

            for(auto i = 0u; i < attUuids.size(); i++)
            {
                auto attribute = attributeModel->findAttribute(attUuids.at(i));
                auto it = std::find_if(parameters.begin(), parameters.end(), [attribute] (auto param) { return param->name() == attribute->name(); });
                QVERIFY(it != parameters.end());

                if (seam->laserControlPreset() == QUuid("7d1a636b-5de7-47f9-9e03-a917dc3c2f1d"))
                {
                    QCOMPARE((*it)->value().toInt(), preset4->getValue(LaserControlPreset::Key(i)));
                }
                if (seam->laserControlPreset() == QUuid("14f819a6-e946-4e55-98e7-727c8f8d7754"))
                {
                    QCOMPARE((*it)->value().toInt(), preset3->getValue(LaserControlPreset::Key(i)));
                }
            }
        }
    }
}

void LaserControlModelTest::testLoad()
{
    LaserControlModel model{this};
    QSignalSpy resetSpy(&model, &LaserControlModel::modelReset);
    QVERIFY(resetSpy.isValid());
    QSignalSpy rowsInsertedSpy(&model, &LaserControlModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());

    QCOMPARE(model.rowCount(), 0);

    const QString presetDir = QFINDTESTDATA("testdata/laser_control_data/");
    model.setLaserControlPresetDir(presetDir);

    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(resetSpy.count(), 2);
    QCOMPARE(rowsInsertedSpy.count(), 0);

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

QTEST_GUILESS_MAIN(LaserControlModelTest)
#include "laserControlModelTest.moc"

