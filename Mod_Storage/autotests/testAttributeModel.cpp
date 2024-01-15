#include <QTest>
#include <QSignalSpy>

#include "../src/attribute.h"
#include "../src/attributeModel.h"

using precitec::storage::Attribute;
using precitec::storage::AttributeModel;

class TestAttributeModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testLoadNonExistingFile();
    void testEmptyFile();
    void testNoAttributes();
    void testLoadAttributes_data();
    void testLoadAttributes();
    void testLoadSystemGraphAttributes();
    void testAttributesSaving();
};

void TestAttributeModel::initTestCase()
{
    qRegisterMetaType<precitec::storage::Attribute*>();
}

void TestAttributeModel::testCtor()
{
    AttributeModel model;
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(!model.findAttribute(QUuid::createUuid()));
}

void TestAttributeModel::testLoadNonExistingFile()
{
    AttributeModel model;
    QSignalSpy modelResetSpy{&model, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    model.load({});
    QVERIFY(!modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 0);
}

void TestAttributeModel::testEmptyFile()
{
    AttributeModel model;
    QSignalSpy modelResetSpy{&model, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    model.load(QFINDTESTDATA("testdata/attributes/empty.json"));
    QVERIFY(!modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 0);
}

void TestAttributeModel::testNoAttributes()
{
    AttributeModel model;
    QSignalSpy modelResetSpy{&model, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    model.load(QFINDTESTDATA("testdata/attributes/noAttributes.json"));
    QVERIFY(!modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 0);
}

void TestAttributeModel::testLoadAttributes_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("publicity");
    QTest::addColumn<int>("editListOrder");
    QTest::addColumn<QString>("contentName");
    QTest::addColumn<QString>("enumeration");
    QTest::addColumn<bool>("visible");
    QTest::addColumn<int>("userLevel");
    QTest::addColumn<QString>("toolTip");
    QTest::addColumn<QString>("description");
    QTest::addColumn<QString>("unit");
    QTest::addColumn<QVariant>("defaultValue");
    QTest::addColumn<QVariant>("minValue");
    QTest::addColumn<QVariant>("maxValue");
    QTest::addColumn<int>("maxLength");
    QTest::addColumn<bool>("mandatory");
    QTest::addColumn<QUuid>("variantId");
    QTest::addColumn<QUuid>("groupId");
    QTest::addColumn<int>("groupIndex");
    QTest::addColumn<int>("step");
    QTest::addColumn<precitec::storage::Parameter::DataType>("type");
    QTest::addColumn<int>("floatingPointPrecision");
    QTest::addColumn<QStringList>("fields");
    QTest::addColumn<QString>("fileLocation");
    QTest::addColumn<QStringList>("fileSuffixes");

    QTest::newRow("attribute1") << 0 << QStringLiteral("MinLimit") << true << 10 << QStringLiteral("Precitec.ProduktParameter.Content.MML4.MinLimit") << QString() << true
        << 0 << QStringLiteral("Precitec.ProduktParameter.Tooltip.MML4.MinLimit") << QStringLiteral("Precitec.ProduktParameter.Beschreibung.MML4.MinLimit")
        << QStringLiteral("Precitec.ProduktParameter.Unit.MML4.MinLimit") << QVariant(-100000) << QVariant(-100000) << QVariant(100000) << 0 << true
        << QUuid(QStringLiteral("F8F4E0A8-D259-40F9-B134-68AA24E0A06C")) << QUuid("") << 0 << 1 << precitec::storage::Parameter::DataType::Double << 3 << QStringList()
        << QString{} << QStringList{};

    QTest::newRow("attribute2") << 1 << QStringLiteral("SeamLeft") << true << 1 << QStringLiteral("Precitec.Filter.Attribute.SeamFindingCollector.Mode.Name")
        << QStringLiteral("Precitec.Filter.Attribute.SeamFindingCollector.Mode.Aufzaehlung") << true << 0
        << QStringLiteral("Precitec.Filter.Attribute.SeamFindingCollector.Mode.Tooltip") << QStringLiteral("Precitec.Filter.Attribute.SeamFindingCollector.Mode.Beschreibung")
        << QStringLiteral("Precitec.Filter.Attribute.SeamFindingCollector.Mode.Unit") << QVariant(0) << QVariant(0) << QVariant(1024) << 0 << true
        << QUuid(QStringLiteral("2B3C26CE-C6DB-4B7D-ABC5-603EA93338D5")) << QUuid("") << 0 << 1 << precitec::storage::Parameter::DataType::Integer << -1 << QStringList()
        << QString{} << QStringList{};

    QTest::newRow("attribute3") << 2 << QStringLiteral("Min") << true << 1 << QStringLiteral("Precitec.Filter.Attribute.LinearLut.Min.Name")
        << QStringLiteral("Precitec.Filter.Attribute.LinearLut.Min.Aufzaehlung") << true << 1
        << QStringLiteral("Precitec.Filter.Attribute.LinearLut.Min.Tooltip") << QStringLiteral("Precitec.Filter.Attribute.LinearLut.Min.Beschreibung")
        << QStringLiteral("Precitec.Filter.Attribute.LinearLut.Min.Unit") << QVariant(1) << QVariant(0) << QVariant(255) << 0 << true
        << QUuid(QStringLiteral("810680EB-91A1-4922-9360-DD3BCC190CB3")) << QUuid("FE745F44-D303-467F-BF28-1B4760F8262B") << 2 << 10 << precitec::storage::Parameter::DataType::Integer << -1 << QStringList()
        << QString{} << QStringList{};

    QTest::newRow("attribute4") << 3 << QStringLiteral("Mode") << false << 1 << QStringLiteral("Precitec.Filter.Attribute.LineTrackingQuality.Mode.Name")
        << QStringLiteral("Precitec.Filter.Attribute.LineTrackingQuality.Mode.Aufzaehlung") << false << 2
        << QStringLiteral("Precitec.Filter.Attribute.LineTrackingQuality.Mode.Tooltip") << QStringLiteral("Precitec.Filter.Attribute.LineTrackingQuality.Mode.Beschreibung")
        << QStringLiteral("Precitec.Filter.Attribute.LineTrackingQuality.Mode.Unit") << QVariant(0) << QVariant(0) << QVariant(10000) << 1 << false
        << QUuid(QStringLiteral("CD2DBC0D-E232-4C13-9698-24FBC3EFDF2A")) << QUuid("") << 0 << 1 << precitec::storage::Parameter::DataType::Enumeration << -1 << QStringList({QStringLiteral("Precitec.Filter.Attribute.Basefilter.Verbosity.enum3")})
        << QString{} << QStringList{};

    QTest::newRow("attribute5") << 4 << QStringLiteral("Tolerance") << true << 1 << QStringLiteral("Precitec.Filter.Attribute.NoSeamCheck.Tolerance.Name")
        << QStringLiteral("Precitec.Filter.Attribute.NoSeamCheck.Tolerance.Aufzaehlung") << true << 0
        << QStringLiteral("Precitec.Filter.Attribute.NoSeamCheck.Tolerance.Tooltip") << QStringLiteral("Precitec.Filter.Attribute.NoSeamCheck.Tolerance.Beschreibung")
        << QStringLiteral("Precitec.Filter.Attribute.NoSeamCheck.Tolerance.Unit") << QVariant(10) << QVariant(0) << QVariant(100) << 0 << true
        << QUuid(QStringLiteral("F9F3821C-CDC8-4B56-8E2F-084EBBF1D2A7")) << QUuid("") << 0 << 1 << precitec::storage::Parameter::DataType::Double << 6 << QStringList()
        << QString{} << QStringList{};

    QTest::newRow("attribute6") << 5 << QStringLiteral("Length") << true << 6 << QStringLiteral("Precitec.ProduktParameter.Content.MML4.Length")
        << QStringLiteral("") << true << 0 << QStringLiteral("Precitec.ProduktParameter.Tooltip.MML4.Length")
        << QStringLiteral("Precitec.ProduktParameter.Beschreibung.MML4.Length")<< QStringLiteral("Precitec.ProduktParameter.Unit.MML4.Length")
        << QVariant(10) << QVariant(0) << QVariant(10000000) << 0 << true << QUuid(QStringLiteral("F8F4E0A8-D259-40F9-B134-68AA24E0A06C")) << QUuid("") << 0 << 1
        << precitec::storage::Parameter::DataType::Double << 3 << QStringList()
        << QString{} << QStringList{};

    QTest::newRow("attribute7") << 6 << QStringLiteral("Max") << true << 1 << QStringLiteral("Precitec.ProduktParameter.Content.MML.Max")
        << QStringLiteral("") << true << 0
        << QStringLiteral("Precitec.ProduktParameter.Tooltip.MML.Max") << QStringLiteral("Precitec.ProduktParameter.Beschreibung.MML.Max")
        << QStringLiteral("Precitec.ProduktParameter.Unit.MML.Max") << QVariant(3) << QVariant(-100000) << QVariant(100000) << 0 << true
        << QUuid(QStringLiteral("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6")) << QUuid("") << 0 << 1 << precitec::storage::Parameter::DataType::Double << 3 << QStringList()
        << QString{} << QStringList{};

    QTest::newRow("attribute8") << 7 << QStringLiteral("MinLimit") << true << 10 << QStringLiteral("Precitec.ProduktParameter.Content.MML4.MinLimit")
        << QStringLiteral("") << true << 0 << QStringLiteral("Precitec.ProduktParameter.Tooltip.MML4.MinLimit")
        << QStringLiteral("Precitec.ProduktParameter.Beschreibung.MML4.MinLimit")<< QStringLiteral("Precitec.ProduktParameter.Unit.MML4.MinLimit")
        << QVariant(-100000) << QVariant(-100000) << QVariant(100000) << 0 << true << QUuid(QStringLiteral("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6")) << QUuid("")
        << 0 << 1 << precitec::storage::Parameter::DataType::Double << 3 << QStringList()
        << QString{} << QStringList{};

    QTest::newRow("attribute9") << 8 << QStringLiteral("Length") << true << 6 << QStringLiteral("Precitec.ProduktParameter.Content.MML4.Length")
        << QStringLiteral("") << true << 0
        << QStringLiteral("Precitec.ProduktParameter.Tooltip.MML4.Length") << QStringLiteral("Precitec.ProduktParameter.Beschreibung.MML4.Length")
        << QStringLiteral("Precitec.ProduktParameter.Unit.MML4.Length") << QVariant(10) << QVariant(0) << QVariant(10000000) << 0 << true
        << QUuid(QStringLiteral("37E21057-EFD4-4C18-A298-BE9F804C6C04")) << QUuid("") << 0 << 1 << precitec::storage::Parameter::DataType::Double << 3 << QStringList()
        << QString{} << QStringList{};

    QTest::newRow("attribute10") << 9 << QStringLiteral("Figure") << true << 7 << QStringLiteral("Precitec.ProduktParameter.Content.MML4.Figure")
        << QStringLiteral("") << true << 0
        << QStringLiteral("Precitec.ProduktParameter.Tooltip.MML4.Figure") << QStringLiteral("Precitec.ProduktParameter.Beschreibung.MML4.Figure")
        << QStringLiteral("Precitec.ProduktParameter.Unit.MML4.Figure") << QVariant(0) << QVariant(0) << QVariant(10000000) << 0 << true
        << QUuid(QStringLiteral("1bb2a9ee-b89f-46ee-94c4-eca3bacf7a93")) << QUuid("") << 0 << 1 << precitec::storage::Parameter::DataType::SeamFigure << -1 << QStringList()
        << QString{} << QStringList{};

    QTest::newRow("attribute11") << 10 << QStringLiteral("WobbleFigure") << true << 8 << QStringLiteral("Precitec.ProduktParameter.Content.MML4.WobbleFigure")
        << QStringLiteral("") << true << 0
        << QStringLiteral("Precitec.ProduktParameter.Tooltip.MML4.WobbleFigure") << QStringLiteral("Precitec.ProduktParameter.Beschreibung.MML4.WobbleFigure")
        << QStringLiteral("Precitec.ProduktParameter.Unit.MML4.WobbleFigure") << QVariant(0) << QVariant(0) << QVariant(10000000) << 0 << true
        << QUuid(QStringLiteral("b37c45ee-05bc-41c4-813d-358410da3d59")) << QUuid("") << 0 << 1 << precitec::storage::Parameter::DataType::WobbleFigure << -1 << QStringList()
        << QString{} << QStringList{};

    QTest::newRow("attribute12") << 11 << QStringLiteral("File") << true << 9 << QStringLiteral("Precitec.ProduktParameter.Content.MML4.File")
        << QStringLiteral("") << true << 0
        << QStringLiteral("Precitec.ProduktParameter.Tooltip.MML4.File") << QStringLiteral("Precitec.ProduktParameter.Beschreibung.MML4.File")
        << QStringLiteral("Precitec.ProduktParameter.Unit.MML4.File") << QVariant("") << QVariant("") << QVariant("") << 0 << true
        << QUuid(QStringLiteral("a17dc35e-a20a-4bee-bcd4-1d505759eed1")) << QUuid("") << 0 << 1 << precitec::storage::Parameter::DataType::File << -1 << QStringList()
        << QStringLiteral("config") << QStringList{QStringLiteral("bmp"), QStringLiteral("png")};
}

void TestAttributeModel::testLoadAttributes()
{
    AttributeModel model;
    QSignalSpy modelResetSpy{&model, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    model.load(QFINDTESTDATA("testdata/attributes/attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);

    QCOMPARE(model.rowCount(), 12);

    QFETCH(int, index);

    QCOMPARE(model.index(index, 0).isValid(), true);
    QCOMPARE(model.rowCount(model.index(index, 0)), 0);
    QCOMPARE(model.index(index, 0).data().isValid(), true);
    QCOMPARE(model.index(12, 0).data().isValid(), false);    
    
    auto attribute = model.index(index, 0).data().value<Attribute*>();
    QVERIFY(attribute);
    QTEST(attribute->name(), "name");
    QTEST(attribute->publicity(), "publicity");
    QTEST(attribute->editListOrder(), "editListOrder");
    QTEST(attribute->contentName(), "contentName");
    QTEST(attribute->enumeration(), "enumeration");
    QTEST(attribute->visible(), "visible");
    QTEST(attribute->userLevel(), "userLevel");
    QTEST(attribute->toolTip(), "toolTip");
    QTEST(attribute->description(), "description");
    QTEST(attribute->unit(), "unit");
    QTEST(attribute->defaultValue(), "defaultValue");
    QTEST(attribute->minValue(), "minValue");
    QTEST(attribute->maxValue(), "maxValue");
    QTEST(attribute->maxLength(), "maxLength");
    QTEST(attribute->mandatory(), "mandatory");
    QTEST(attribute->variantId(), "variantId");
    QTEST(attribute->groupId(), "groupId");
    QTEST(attribute->groupIndex(), "groupIndex");
    QTEST(attribute->step(), "step");
    QTEST(attribute->type(), "type");
    QTEST(attribute->floatingPointPrecision(), "floatingPointPrecision");
    QTEST(attribute->fields(), "fields");
    QTEST(attribute->fileInformation().location(), "fileLocation");
    QTEST(attribute->fileInformation().suffixes(), "fileSuffixes");
    QCOMPARE(model.findAttribute(attribute->uuid()), attribute);

    QSignalSpy attributeDeletedSpy{attribute, &QObject::destroyed};
    QVERIFY(attributeDeletedSpy.isValid());

    // random id still not found
    QVERIFY(model.findAttribute(QUuid::createUuid()) == nullptr);

    // loading another file now should clear
    model.load(QFINDTESTDATA("testdata/attributes/noAttributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 2);
    QCOMPARE(attributeDeletedSpy.count(), 1);
    QCOMPARE(model.rowCount(), 0);
}

void TestAttributeModel::testLoadSystemGraphAttributes()
{
    AttributeModel model;
    QSignalSpy modelResetSpy{&model, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    model.load(QFINDTESTDATA("../../wm_inst/system_graphs/attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);
}


void TestAttributeModel::testAttributesSaving()
{
    AttributeModel modelWrite;
    QSignalSpy modelResetSpy{&modelWrite, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    modelWrite.load(QFINDTESTDATA("../../wm_inst/system_graphs/keyValueAttributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);    
    
    QString path = QDir::currentPath() + "/NewKeyAttributes.json";
    QFile jsonFile(path);
    QVERIFY(jsonFile.open(QIODevice::WriteOnly | QIODevice::Text));
    modelWrite.toJsonFile(&jsonFile);
    QVERIFY(jsonFile.flush()); 
   
    AttributeModel modelRead;
    QSignalSpy modelResetSpy2{&modelRead, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy2.isValid());
    modelRead.load(QFINDTESTDATA(QDir::currentPath() + "/NewKeyAttributes.json"));
    QVERIFY(modelResetSpy2.wait());
    QCOMPARE(modelResetSpy2.count(), 1);
}

    
    

QTEST_GUILESS_MAIN(TestAttributeModel)
#include "testAttributeModel.moc"
