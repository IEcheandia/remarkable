#include <QTest>
#include <QSignalSpy>

#include "../src/hardwareParameterProductModel.h"
#include "../src/hardwareParameters.h"
#include "attribute.h"
#include "attributeModel.h"
#include "parameter.h"
#include "product.h"

using precitec::gui::HardwareParameterProductModel;
using precitec::gui::HardwareParameters;
using precitec::storage::Attribute;
using precitec::storage::AttributeModel;
using precitec::storage::Parameter;
using precitec::storage::Product;

class HardwareParameterProductModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testDisplayRole_data();
    void testDisplayRole();
    void testKey_data();
    void testKey();
    void testSetProduct();
    void testSetAttributeModel();
    void testFindAttribute();
    void testEnable();
    void testUpdateParameter();
    void testTranslations_data();
    void testTranslations();

private:
    QTemporaryDir m_dir;
};

void HardwareParameterProductModelTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void HardwareParameterProductModelTest::testCtor()
{
    HardwareParameterProductModel model;
    QVERIFY(!model.product());
    QVERIFY(!model.attributeModel());
    QCOMPARE(model.rowCount(), int(HardwareParameters::Key::InvalidKey));

    for (int i = 0; i < model.rowCount(); i++)
    {
        const auto index = model.index(i, 0);
        QVERIFY(index.isValid());
        QCOMPARE(index.data(Qt::UserRole + 1).toBool(), false);
        QVERIFY(!index.data(Qt::UserRole + 2).value<Attribute*>());
        QVERIFY(!index.data(Qt::UserRole + 3).value<Parameter*>());
    }
}

void HardwareParameterProductModelTest::testDisplayRole_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("name");

    QTest::newRow("0")  <<  0 << QStringLiteral("Start position");
    QTest::newRow("1")  <<  1 << QStringLiteral("Software limits");
    QTest::newRow("2")  <<  2 << QStringLiteral("Lower software limit");
    QTest::newRow("3")  <<  3 << QStringLiteral("Upper software limit");
    QTest::newRow("4")  <<  4 << QStringLiteral("Velocity");
    QTest::newRow("5")  <<  5 << QStringLiteral("Acceleration");
    QTest::newRow("6")  <<  6 << QStringLiteral("Line Laser 1 enabled");
    QTest::newRow("7")  <<  7 << QStringLiteral("Line Laser 1 intensity");
    QTest::newRow("8")  <<  8 << QStringLiteral("Line Laser 2 enabled");
    QTest::newRow("9")  <<  9 << QStringLiteral("Line Laser 2 intensity");
    QTest::newRow("10") << 10 << QStringLiteral("Line Laser 3 enabled");
    QTest::newRow("11") << 11 << QStringLiteral("Line Laser 3 intensity");
    QTest::newRow("12") << 12 << QStringLiteral("Exposure time");
    QTest::newRow("13") << 13 << QStringLiteral("Camera X");
    QTest::newRow("14") << 14 << QStringLiteral("Camera Y");
    QTest::newRow("15") << 15 << QStringLiteral("Camera width");
    QTest::newRow("16") << 16 << QStringLiteral("Camera height");
    QTest::newRow("17") << 17 << QStringLiteral("LED Flash Delay");
    QTest::newRow("18") << 18 << QStringLiteral("LED panel 1 enabled");
    QTest::newRow("19") << 19 << QStringLiteral("LED panel 1 intensity");
    QTest::newRow("20") << 20 << QStringLiteral("LED panel 1 pulse width");
    QTest::newRow("21") << 21 << QStringLiteral("LED panel 2 enabled");
    QTest::newRow("22") << 22 << QStringLiteral("LED panel 2 intensity");
    QTest::newRow("23") << 23 << QStringLiteral("LED panel 2 pulse width");
    QTest::newRow("24") << 24 << QStringLiteral("LED panel 3 enabled");
    QTest::newRow("25") << 25 << QStringLiteral("LED panel 3 intensity");
    QTest::newRow("26") << 26 << QStringLiteral("LED panel 3 pulse width");
    QTest::newRow("27") << 27 << QStringLiteral("LED panel 4 enabled");
    QTest::newRow("28") << 28 << QStringLiteral("LED panel 4 intensity");
    QTest::newRow("29") << 29 << QStringLiteral("LED panel 4 pulse width");
    QTest::newRow("30") << 30 << QStringLiteral("LED panel 5 enabled");
    QTest::newRow("31") << 31 << QStringLiteral("LED panel 5 intensity");
    QTest::newRow("32") << 32 << QStringLiteral("LED panel 5 pulse width");
    QTest::newRow("33") << 33 << QStringLiteral("LED panel 6 enabled");
    QTest::newRow("34") << 34 << QStringLiteral("LED panel 6 intensity");
    QTest::newRow("35") << 35 << QStringLiteral("LED panel 6 pulse width");
    QTest::newRow("36") << 36 << QStringLiteral("LED panel 7 enabled");
    QTest::newRow("37") << 37 << QStringLiteral("LED panel 7 intensity");
    QTest::newRow("38") << 38 << QStringLiteral("LED panel 7 pulse width");
    QTest::newRow("39") << 39 << QStringLiteral("LED panel 8 enabled");
    QTest::newRow("40") << 40 << QStringLiteral("LED panel 8 intensity");
    QTest::newRow("41") << 41 << QStringLiteral("LED panel 8 pulse width");
    QTest::newRow("42") << 42 << QStringLiteral("ScanTracker: Switch on/off scan function");
    QTest::newRow("43") << 43 << QStringLiteral("ScanTracker: Dynamic scan-width determined by graph");
    QTest::newRow("44") << 44 << QStringLiteral("ScanTracker: Dynamic scan-position determined by graph");
    QTest::newRow("45") << 45 << QStringLiteral("ScanTracker: Frequency for scan function");
    QTest::newRow("46") << 46 << QStringLiteral("ScanTracker: Fixed scan-position");
    QTest::newRow("47") << 47 << QStringLiteral("ScanTracker: Fixed scan-width");
    QTest::newRow("48") << 48 << QStringLiteral("Reset Encoder counter 1");
    QTest::newRow("49") << 49 << QStringLiteral("Reset Encoder counter 2");
    QTest::newRow("50") << 50 << QStringLiteral("LWM Plasma Amplification");
    QTest::newRow("51") << 51 << QStringLiteral("LWM Temperature Amplification");
    QTest::newRow("52") << 52 << QStringLiteral("LWM Back Reflection Amplification");
    QTest::newRow("53") << 53 << QStringLiteral("LWM External Digital Input Amplification");
    QTest::newRow("54") << 54 << QStringLiteral("Laser Power Static Center");
    QTest::newRow("55") << 55 << QStringLiteral("SLD Dimmer On/Off");
    QTest::newRow("56") << 56 << QStringLiteral("Lamp Intensity");
    QTest::newRow("57") << 57 << QStringLiteral("Wobbel Figure File Number");
    QTest::newRow("58") << 58 << QStringLiteral("Laser Power Digital");
    QTest::newRow("59") << 59 << QStringLiteral("LaserDelay");
    QTest::newRow("60") << 60 << QStringLiteral("Z Collimator Absolute Position");
    QTest::newRow("61") << 61 << QStringLiteral("Scanner Drive To Zero");
    QTest::newRow("62") << 62 << QStringLiteral("Scanner Jump Speed");
    QTest::newRow("63") << 63 << QStringLiteral("Laser on delay");
    QTest::newRow("64") << 64 << QStringLiteral("Laser off delay");
    QTest::newRow("65") << 65 << QStringLiteral("Adaptive Exposure Basic Value");
    QTest::newRow("66") << 66 << QStringLiteral("Adaptive Exposure Mode On/Off");
    QTest::newRow("67") << 67 << QStringLiteral("Scanner X Position");
    QTest::newRow("68") << 68 << QStringLiteral("Scanner Y Position");
    QTest::newRow("69") << 69 << QStringLiteral("Move Scanner to Position");
    QTest::newRow("70") << 70 << QStringLiteral("Lin Log: Mode");
    QTest::newRow("71") << 71 << QStringLiteral("Lin Log: Value 1");
    QTest::newRow("72") << 72 << QStringLiteral("Lin Log: Value 2");
    QTest::newRow("73") << 73 << QStringLiteral("Lin Log: Time 1");
    QTest::newRow("74") << 74 << QStringLiteral("Lin Log: Time 2");
    QTest::newRow("75") << 75 << QStringLiteral("Laser Dual Channel");
    QTest::newRow("76") << 76 << QStringLiteral("Laser Power Static Ring");
    QTest::newRow("77") << 77 << QStringLiteral("Basic wobble figure horizontal size");
    QTest::newRow("78") << 78 << QStringLiteral("Basic wobble figure vertical size");
    QTest::newRow("79") << 79 << QStringLiteral("Basic wobble figure frequency");
    QTest::newRow("80") << 80 << QStringLiteral("Wobble mode");
    QTest::newRow("81") << 81 << QStringLiteral("Acquisition Mode (Continuous or SingleFrame)");
    QTest::newRow("82") << 82 << QStringLiteral("Reuse last captured image (implies Acquisition Mode SingleFrame)");
    QTest::newRow("83") << 83 << QStringLiteral("Move Scanner to Position With OCT Reference");
    QTest::newRow("84") << 84 << QStringLiteral("Rotate figure");
    QTest::newRow("85") << 85 << QStringLiteral("Laser delay");
    QTest::newRow("86") << 86 << QStringLiteral("ScanTracker2D Custom Figure");
    QTest::newRow("87") << 87 << QStringLiteral("Scale in welding direction");
    QTest::newRow("88") << 88 << QStringLiteral("Scale vertical to welding direction");
    QTest::newRow("89") << 89 << QStringLiteral("ScanTracker2D: Fixed scan-position (X)");
    QTest::newRow("90") << 90 << QStringLiteral("ScanTracker2D: Fixed scan-position (Y)");
}

void HardwareParameterProductModelTest::testDisplayRole()
{
    HardwareParameterProductModel model;
    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data().toString(), "name");
}

void HardwareParameterProductModelTest::testKey_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<HardwareParameters::Key>("key");

    QTest::newRow("0")  <<  0 << HardwareParameters::Key::YAxisAbsolutePosition;
    QTest::newRow("1")  <<  1 << HardwareParameters::Key::YAxisSoftwareLimits;
    QTest::newRow("2")  <<  2 << HardwareParameters::Key::YAxisLowerLimit;
    QTest::newRow("3")  <<  3 << HardwareParameters::Key::YAxisUpperLimit;
    QTest::newRow("4")  <<  4 << HardwareParameters::Key::YAxisVelocity;
    QTest::newRow("5")  <<  5 << HardwareParameters::Key::YAxisAcceleration;
    QTest::newRow("6")  <<  6 << HardwareParameters::Key::LineLaser1OnOff;
    QTest::newRow("7")  <<  7 << HardwareParameters::Key::LineLaser1Intensity;
    QTest::newRow("8")  <<  8 << HardwareParameters::Key::LineLaser2OnOff;
    QTest::newRow("9")  <<  9 << HardwareParameters::Key::LineLaser2Intensity;
    QTest::newRow("10") << 10 << HardwareParameters::Key::FieldLight1OnOff;
    QTest::newRow("11") << 11 << HardwareParameters::Key::FieldLight1Intensity;
    QTest::newRow("12") << 12 << HardwareParameters::Key::ExposureTime;
    QTest::newRow("13") << 13 << HardwareParameters::Key::CameraRoiX;
    QTest::newRow("14") << 14 << HardwareParameters::Key::CameraRoiY;
    QTest::newRow("15") << 15 << HardwareParameters::Key::CameraRoiWidth;
    QTest::newRow("16") << 16 << HardwareParameters::Key::CameraRoiHeight;
    QTest::newRow("17") << 17 << HardwareParameters::Key::LEDFlashDelay;
    QTest::newRow("18") << 18 << HardwareParameters::Key::LEDPanel1OnOff;
    QTest::newRow("19") << 19 << HardwareParameters::Key::LEDPanel1Intensity;
    QTest::newRow("20") << 20 << HardwareParameters::Key::LEDPanel1PulseWidth;
    QTest::newRow("21") << 21 << HardwareParameters::Key::LEDPanel2OnOff;
    QTest::newRow("22") << 22 << HardwareParameters::Key::LEDPanel2Intensity;
    QTest::newRow("23") << 23 << HardwareParameters::Key::LEDPanel2PulseWidth;
    QTest::newRow("24") << 24 << HardwareParameters::Key::LEDPanel3OnOff;
    QTest::newRow("25") << 25 << HardwareParameters::Key::LEDPanel3Intensity;
    QTest::newRow("26") << 26 << HardwareParameters::Key::LEDPanel3PulseWidth;
    QTest::newRow("27") << 27 << HardwareParameters::Key::LEDPanel4OnOff;
    QTest::newRow("28") << 28 << HardwareParameters::Key::LEDPanel4Intensity;
    QTest::newRow("29") << 29 << HardwareParameters::Key::LEDPanel4PulseWidth;
    QTest::newRow("30") << 30 << HardwareParameters::Key::LEDPanel5OnOff;
    QTest::newRow("31") << 31 << HardwareParameters::Key::LEDPanel5Intensity;
    QTest::newRow("32") << 32 << HardwareParameters::Key::LEDPanel5PulseWidth;
    QTest::newRow("33") << 33 << HardwareParameters::Key::LEDPanel6OnOff;
    QTest::newRow("34") << 34 << HardwareParameters::Key::LEDPanel6Intensity;
    QTest::newRow("35") << 35 << HardwareParameters::Key::LEDPanel6PulseWidth;
    QTest::newRow("36") << 36 << HardwareParameters::Key::LEDPanel7OnOff;
    QTest::newRow("37") << 37 << HardwareParameters::Key::LEDPanel7Intensity;
    QTest::newRow("38") << 38 << HardwareParameters::Key::LEDPanel7PulseWidth;
    QTest::newRow("39") << 39 << HardwareParameters::Key::LEDPanel8OnOff;
    QTest::newRow("40") << 40 << HardwareParameters::Key::LEDPanel8Intensity;
    QTest::newRow("41") << 41 << HardwareParameters::Key::LEDPanel8PulseWidth;
    QTest::newRow("42") << 42 << HardwareParameters::Key::TrackerDriverOnOff;
    QTest::newRow("43") << 43 << HardwareParameters::Key::ScanWidthOutOfGapWidth;
    QTest::newRow("44") << 44 << HardwareParameters::Key::ScanPosOutOfGapPos;
    QTest::newRow("45") << 45 << HardwareParameters::Key::ScanTrackerFrequencyContinuously;
    QTest::newRow("46") << 46 << HardwareParameters::Key::ScanTrackerScanPosFixed;
    QTest::newRow("47") << 47 << HardwareParameters::Key::ScanTrackerScanWidthFixed;
    QTest::newRow("48") << 48 << HardwareParameters::Key::ClearEncoderCounter1;
    QTest::newRow("49") << 49 << HardwareParameters::Key::ClearEncoderCounter2;
    QTest::newRow("50") << 50 << HardwareParameters::Key::LWM40No1AmpPlasma;
    QTest::newRow("51") << 51 << HardwareParameters::Key::LWM40No1AmpTemperature;
    QTest::newRow("52") << 52 << HardwareParameters::Key::LWM40No1AmpBackReflection;
    QTest::newRow("53") << 53 << HardwareParameters::Key::LWM40No1AmpAnalogInput;
    QTest::newRow("54") << 54 << HardwareParameters::Key::ScannerLaserPowerStatic;
    QTest::newRow("55") << 55 << HardwareParameters::Key::SLDDimmerOnOff;
    QTest::newRow("56") << 56 << HardwareParameters::Key::IDMLampIntensity;
    QTest::newRow("57") << 57 << HardwareParameters::Key::ScannerFileNumber;
    QTest::newRow("58") << 58 << HardwareParameters::Key::IsLaserPowerDigital;
    QTest::newRow("59") << 59 << HardwareParameters::Key::LaserDelay;
    QTest::newRow("60") << 60 << HardwareParameters::Key::ZCollimatorPositionAbsolute;
    QTest::newRow("61") << 61 << HardwareParameters::Key::ScannerDriveToZero;
    QTest::newRow("62") << 62 << HardwareParameters::Key::ScannerJumpSpeed;
    QTest::newRow("63") << 63 << HardwareParameters::Key::LaserOnDelay;
    QTest::newRow("64") << 64 << HardwareParameters::Key::LaserOffDelay;
    QTest::newRow("65") << 65 << HardwareParameters::Key::IDMAdaptiveExposureBasicValue;
    QTest::newRow("66") << 66 << HardwareParameters::Key::IDMAdaptiveExposureModeOnOff;
    QTest::newRow("67") << 67 << HardwareParameters::Key::ScannerNewXPosition;
    QTest::newRow("68") << 68 << HardwareParameters::Key::ScannerNewYPosition;
    QTest::newRow("69") << 69 << HardwareParameters::Key::ScannerDriveToPosition;
    QTest::newRow("70") << 70 << HardwareParameters::Key::LinLogMode;
    QTest::newRow("71") << 71 << HardwareParameters::Key::LinLogValue1;
    QTest::newRow("72") << 72 << HardwareParameters::Key::LinLogValue2;
    QTest::newRow("73") << 73 << HardwareParameters::Key::LinLogTime1;
    QTest::newRow("74") << 74 << HardwareParameters::Key::LinLogTime2;
    QTest::newRow("75") << 75 << HardwareParameters::Key::LaserIsDualChannel;
    QTest::newRow("76") << 76 << HardwareParameters::Key::ScannerLaserPowerStaticRing;
    QTest::newRow("77") << 77 << HardwareParameters::Key::WobbleXSize;
    QTest::newRow("78") << 78 << HardwareParameters::Key::WobbleYSize;
    QTest::newRow("79") << 79 << HardwareParameters::Key::WobbleFrequency;
    QTest::newRow("80") << 80 << HardwareParameters::Key::WobbleMode;
    QTest::newRow("81") << 81 << HardwareParameters::Key::CameraAcquisitionMode;
    QTest::newRow("82") << 82 << HardwareParameters::Key::CameraReuseLastImage;
    QTest::newRow("83") << 83 << HardwareParameters::Key::ScannerDriveWithOCTReference;
    QTest::newRow("84") << 84 << HardwareParameters::Key::ScanTracker2DAngle;
    QTest::newRow("85") << 85 << HardwareParameters::Key::Scantracker2DLaserDelay;
    QTest::newRow("86") << 86 << HardwareParameters::Key::ScanTracker2DCustomFigure;
    QTest::newRow("87") << 87 << HardwareParameters::Key::ScanTracker2DScanWidthFixedX;
    QTest::newRow("88") << 88 << HardwareParameters::Key::ScanTracker2DScanWidthFixedY;
    QTest::newRow("89") << 89 << HardwareParameters::Key::ScanTracker2DScanPosFixedX;
    QTest::newRow("90") << 90 << HardwareParameters::Key::ScanTracker2DScanPosFixedY;
}

void HardwareParameterProductModelTest::testKey()
{
    HardwareParameterProductModel model;
    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole).value<HardwareParameters::Key>(), "key");
}

void HardwareParameterProductModelTest::testSetProduct()
{
    HardwareParameterProductModel model;
    QSignalSpy dataChangedSpy(&model, &HardwareParameterProductModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    QSignalSpy productChangedSpy(&model, &HardwareParameterProductModel::productChanged);
    QVERIFY(productChangedSpy.isValid());

    // create a Product
    auto p = new Product{QUuid::createUuid()};
    QVERIFY(p);
    QVERIFY(!p->hardwareParameters());

    QCOMPARE(dataChangedSpy.count(), 0);
    model.setProduct(p);
    QCOMPARE(model.product(), p);
    QCOMPARE(productChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);

    // setting same should not change
    model.setProduct(p);
    QCOMPARE(productChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);

    auto ps = model.getParameterSet();
    QVERIFY(ps);
    QCOMPARE(p->hardwareParameters(), ps);
    QCOMPARE(model.getParameterSet(), ps);

    p->deleteLater();
    QVERIFY(productChangedSpy.wait());
    QCOMPARE(productChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.count(), 2);
}

void HardwareParameterProductModelTest::testSetAttributeModel()
{
    HardwareParameterProductModel model;
    QSignalSpy dataChangedSpy(&model, &HardwareParameterProductModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    QSignalSpy attributeModelChangedSpy(&model, &HardwareParameterProductModel::attributeModelChanged);
    QVERIFY(attributeModelChangedSpy.isValid());

    std::unique_ptr<AttributeModel> am = std::make_unique<AttributeModel>();
    model.setAttributeModel(am.get());
    QCOMPARE(model.attributeModel(), am.get());
    QCOMPARE(attributeModelChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);

    // setting same should not change
    model.setAttributeModel(am.get());
    QCOMPARE(attributeModelChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);

    // let's delete it
    am.reset();
    QCOMPARE(attributeModelChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.count(), 2);
}

void HardwareParameterProductModelTest::testFindAttribute()
{
    HardwareParameterProductModel model;
    AttributeModel am;
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/attributes.json"));
    QSignalSpy dataChangedSpy(&model, &HardwareParameterProductModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(am.rowCount(), 2);

    auto attribute = model.index(int(HardwareParameters::Key::LEDPanel1Intensity), 0).data(Qt::UserRole + 2).value<Attribute*>();
    QVERIFY(attribute);
    // others should not have an attribute
    for (int i = 0; i < 8; i++)
    {
        if (i != int(HardwareParameters::Key::LEDPanel1Intensity))
        {
            QVERIFY(!model.index(i, 0).data(Qt::UserRole + 2).value<Attribute*>());
        }
    }
    QCOMPARE(attribute->name(), QStringLiteral("LEDPanel1Intensity"));
}

void HardwareParameterProductModelTest::testEnable()
{
    HardwareParameterProductModel model;
    AttributeModel am;
    QSignalSpy modelResetSpy{&am, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QSignalSpy dataChangedSpy(&model, &HardwareParameterProductModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    // create a Product
    auto p = new Product{QUuid::createUuid()};
    QVERIFY(p);
    QVERIFY(!p->hardwareParameters());
    model.setProduct(p);
    QCOMPARE(dataChangedSpy.count(), 1);

    QSignalSpy hasChangesChangedSpy{&model, &HardwareParameterProductModel::markAsChanged};
    QVERIFY(hasChangesChangedSpy.isValid());
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 1).toBool(), false);
    QCOMPARE(dataChangedSpy.count(), 1);
    model.setEnable(HardwareParameters::Key::LEDPanel1Intensity, true);
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(hasChangesChangedSpy.count(), 1);
    QCOMPARE(model.index(int(HardwareParameters::Key::LEDPanel1Intensity), 0).data(Qt::UserRole + 1).toBool(), true);

    auto parameter = model.index(int(HardwareParameters::Key::LEDPanel1Intensity), 0).data(Qt::UserRole + 3).value<Parameter*>();
    QVERIFY(parameter);
    QCOMPARE(parameter->name(), QStringLiteral("LEDPanel1Intensity"));

    QSignalSpy parameterDestroyedSpy{parameter, &Parameter::destroyed};
    QVERIFY(parameterDestroyedSpy.isValid());
    model.setEnable(HardwareParameters::Key::LEDPanel1Intensity, false);
    QVERIFY(!model.index(int(HardwareParameters::Key::LEDPanel1Intensity), 0).data(Qt::UserRole + 3).value<Parameter*>());
    QVERIFY(parameterDestroyedSpy.wait());
    QCOMPARE(hasChangesChangedSpy.count(), 2);

    // unset product should result in dataChanged
    p->deleteLater();
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(dataChangedSpy.count(), 4);

    // no seam, so setEnable shouldn't work
    model.setEnable(HardwareParameters::Key::LEDPanel1Intensity, true);
    QCOMPARE(dataChangedSpy.count(), 4);
}

void HardwareParameterProductModelTest::testUpdateParameter()
{
    HardwareParameterProductModel model;
    AttributeModel am;
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/attributes.json"));
    QSignalSpy dataChangedSpy(&model, &HardwareParameterProductModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    QVERIFY(dataChangedSpy.wait());
    QSignalSpy parameterChangedSpy{&model, &HardwareParameterProductModel::parameterChanged};
    QVERIFY(parameterChangedSpy.isValid());

    // create a Product
    auto p = new Product{QUuid::createUuid()};

    QVERIFY(p);
    QVERIFY(!p->hardwareParameters());
    model.setProduct(p);

    QSignalSpy hasChangesChangedSpy{&model, &HardwareParameterProductModel::markAsChanged};
    QVERIFY(hasChangesChangedSpy.isValid());

    // parameter not yet created
    model.updateHardwareParameter(HardwareParameters::Key::LEDPanel1Intensity, {55});
    QCOMPARE(hasChangesChangedSpy.count(), 0);
    // so let's create the Parameter
    QCOMPARE(parameterChangedSpy.count(), 0);
    model.setEnable(HardwareParameters::Key::LEDPanel1Intensity, true);
    QCOMPARE(parameterChangedSpy.count(), 1);
    auto parameter = model.index(int(HardwareParameters::Key::LEDPanel1Intensity), 0).data(Qt::UserRole + 3).value<Parameter*>();
    QVERIFY(parameter);
    QCOMPARE(hasChangesChangedSpy.count(), 1);

    // let's change the parameter
    QCOMPARE(parameterChangedSpy.count(), 1);
    model.updateHardwareParameter(HardwareParameters::Key::LEDPanel1Intensity, {55});
    QCOMPARE(hasChangesChangedSpy.count(), 2);
    QCOMPARE(parameterChangedSpy.count(), 2);
    QCOMPARE(parameter->value().toInt(), 55);
}

void HardwareParameterProductModelTest::testTranslations_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QUuid>("id");

    QTest::newRow("0") << QStringLiteral("Start position") << QUuid{QByteArrayLiteral("FC69766B-E8C0-42CE-BC95-471EADBC66DB")};
    QTest::newRow("1") << QStringLiteral("Software limits") << QUuid{QByteArrayLiteral("66B08DBB-5A0E-4EB5-9A6C-66FB80A3175E")};
    QTest::newRow("2") << QStringLiteral("Upper software limit") << QUuid{QByteArrayLiteral("0C56B220-868C-4323-B1F9-33D85DF3F1F4")};
    QTest::newRow("3") << QStringLiteral("Lower software limit") << QUuid{QByteArrayLiteral("EE613A26-012F-4EEA-A908-C9B07352C960")};
    QTest::newRow("4") << QStringLiteral("Velocity") << QUuid{QByteArrayLiteral("E7DBEE0B-EBBE-4973-B553-C14F7F89D5BC")};
    QTest::newRow("5") << QStringLiteral("Acceleration") << QUuid{QByteArrayLiteral("B47C8607-F8A2-4E40-A456-A0ABAAE75813")};
    QTest::newRow("6") << QStringLiteral("Line Laser 1 enabled") << QUuid{QByteArrayLiteral("3C5B7C40-A328-45F1-A107-4DD038A4FFEB")};
    QTest::newRow("7") << QStringLiteral("Line Laser 1 intensity") << QUuid{QByteArrayLiteral("13D8D645-BC7B-410A-B511-344E4D3216D7")};
    QTest::newRow("8") << QStringLiteral("Line Laser 2 enabled") << QUuid{QByteArrayLiteral("2668D3F1-8BAF-4B6C-8889-1B93D8533E0E")};
    QTest::newRow("9") << QStringLiteral("Line Laser 2 intensity") << QUuid{QByteArrayLiteral("23AE1EF0-CA79-4A85-9101-3D3D8BAAD8F5")};
    QTest::newRow("10") << QStringLiteral("Line Laser 3 enabled") << QUuid{QByteArrayLiteral("D3ED251D-D175-4A74-A9E9-09633FC4052E")};
    QTest::newRow("11") << QStringLiteral("Line Laser 3 intensity") << QUuid{QByteArrayLiteral("C3444855-F910-4593-881E-61BB50BD41EF")};
    QTest::newRow("12") << QStringLiteral("Exposure time") << QUuid{QByteArrayLiteral("DB178308-E2FD-4FA7-B60C-42CDDADA0B85")};
    QTest::newRow("13") << QStringLiteral("Camera X") << QUuid{QByteArrayLiteral("FF97662F-6DB7-4B77-9140-7E1267F902F0")};
    QTest::newRow("14") << QStringLiteral("Camera Y") << QUuid{QByteArrayLiteral("DB4EAF88-9A9F-43AE-A6D8-BE2C865336FC")};
    QTest::newRow("15") << QStringLiteral("Camera width") << QUuid{QByteArrayLiteral("30ACD6F4-41AE-4900-8F69-4DD700609BD2")};
    QTest::newRow("16") << QStringLiteral("Camera height") << QUuid{QByteArrayLiteral("EEE02A8F-298A-4C27-B6EE-2982CCAAD1A3")};
    QTest::newRow("17") << QStringLiteral("LED Flash Delay") << QUuid{QByteArrayLiteral("9824AAC2-0849-4C90-89B8-43E558E782CC")};
    QTest::newRow("18") << QStringLiteral("LED panel 1 enabled") << QUuid{QByteArrayLiteral("D988FF12-9E3C-4B80-BF81-82C6C7AE76BB")};
    QTest::newRow("19") << QStringLiteral("LED panel 1 intensity") << QUuid{QByteArrayLiteral("F4FE85AC-94C6-4EFC-A780-6FCB7B7413CF")};
    QTest::newRow("20") << QStringLiteral("LED panel 1 pulse width") << QUuid{QByteArrayLiteral("575DF2BA-C950-46A6-A4A9-04D3E943D679")};
    QTest::newRow("21") << QStringLiteral("LED panel 2 enabled") << QUuid{QByteArrayLiteral("08AD00E5-B86A-439B-A257-7C37E99CCE0D")};
    QTest::newRow("22") << QStringLiteral("LED panel 2 intensity") << QUuid{QByteArrayLiteral("CFC70EB5-31E8-4585-B307-5A3E92632240")};
    QTest::newRow("23") << QStringLiteral("LED panel 2 pulse width") << QUuid{QByteArrayLiteral("01655048-FDAC-43DD-BBF5-36D65AF99F52")};
    QTest::newRow("24") << QStringLiteral("LED panel 3 enabled") << QUuid{QByteArrayLiteral("5A2223A0-8EB7-4A02-A224-405771DC7B6E")};
    QTest::newRow("25") << QStringLiteral("LED panel 3 intensity") << QUuid{QByteArrayLiteral("28E8F256-EAD5-472C-A0CB-00BE4558C3A1")};
    QTest::newRow("26") << QStringLiteral("LED panel 3 pulse width") << QUuid{QByteArrayLiteral("75F74A37-AE25-4FE6-B117-9C2DAA3ADED3")};
    QTest::newRow("27") << QStringLiteral("LED panel 4 enabled") << QUuid{QByteArrayLiteral("82CE05F5-EA23-48E0-8BA8-7FC23F1B0592")};
    QTest::newRow("28") << QStringLiteral("LED panel 4 intensity") << QUuid{QByteArrayLiteral("6A7DCE0C-5C74-44FB-B7AC-395D975A138C")};
    QTest::newRow("29") << QStringLiteral("LED panel 4 pulse width") << QUuid{QByteArrayLiteral("8241AFD5-C10E-4A30-BD92-AB0FA420832B")};
    QTest::newRow("30") << QStringLiteral("ScanTracker: Switch on/off scan function") << QUuid{QByteArrayLiteral("70e7ef10-5462-47c2-979b-8fc706147349")};
    QTest::newRow("31") << QStringLiteral("ScanTracker: Dynamic scan-width determined by graph") << QUuid{QByteArrayLiteral("9ad3c9b6-4bfa-42b9-bdee-408729418526")};
    QTest::newRow("32") << QStringLiteral("ScanTracker: Dynamic scan-position determined by graph") << QUuid{QByteArrayLiteral("44dfce0b-ff2a-4b89-928b-a36c3735366e")};
    QTest::newRow("33") << QStringLiteral("ScanTracker: Frequency for scan function") << QUuid{QByteArrayLiteral("8b9129cf-3b59-4176-b252-5242e38eada0")};
    QTest::newRow("34") << QStringLiteral("ScanTracker: Fixed scan-position") << QUuid{QByteArrayLiteral("e38f7102-07ce-4915-ac2e-8bba878eda11")};
    QTest::newRow("35") << QStringLiteral("ScanTracker: Fixed scan-width") << QUuid{QByteArrayLiteral("bca98bbf-404e-4260-97d3-97f1b4697d96")};
    QTest::newRow("36") << QStringLiteral("Reset Encoder counter 1") << QUuid{QByteArrayLiteral("C2CE8555-E11C-4B83-A7FE-550920ACFFD4")};
    QTest::newRow("37") << QStringLiteral("Reset Encoder counter 2") << QUuid{QByteArrayLiteral("A534EB19-6E04-40C2-A299-880B44115378")};
    QTest::newRow("38") << QStringLiteral("LWM Plasma Amplification") << QUuid{QByteArrayLiteral("234FF086-4E5C-460E-BAC6-A3C9D532C07C")};
    QTest::newRow("39") << QStringLiteral("LWM Temperature Amplification") << QUuid{QByteArrayLiteral("D6D79422-E5DF-4454-9733-883A74F219B1")};
    QTest::newRow("40") << QStringLiteral("LWM Back Reflection Amplification") << QUuid{QByteArrayLiteral("9CF69A54-65F1-40D4-B402-CF5A1824F6D7")};
    QTest::newRow("41") << QStringLiteral("LWM External Digital Input Amplification") << QUuid{QByteArrayLiteral("68FB5A6B-4FF9-4DCB-8246-92EE7004EAE5")};
    QTest::newRow("42") << QStringLiteral("Laser Power Static Center") << QUuid{QByteArrayLiteral("2862509f-6b2b-4827-831c-e1149fb52e86")};
    QTest::newRow("43") << QStringLiteral("SLD Dimmer On/Off") << QUuid{QByteArrayLiteral("13B32EC3-ED25-4043-8C36-837AC04827EB")};
    QTest::newRow("44") << QStringLiteral("Lamp Intensity") << QUuid{QByteArrayLiteral("8E0993C9-5AEB-47E3-9E66-B6A73FF02743")};
    QTest::newRow("45") << QStringLiteral("Wobbel Figure File Number") << QUuid{QByteArrayLiteral("5A000DA3-C2D3-4F69-AAE9-C1921FDFD444")};
    QTest::newRow("46") << QStringLiteral("Laser Power Digital") << QUuid{QByteArrayLiteral("E515AF6B-6BC6-4494-B8C2-9A2ABE00BFCD")};
    QTest::newRow("47") << QStringLiteral("LaserDelay") << QUuid{QByteArrayLiteral("458E73FF-AE6E-49B6-BA3F-9E8877145C67")};
    QTest::newRow("48") << QStringLiteral("Z Collimator Absolute Position") << QUuid{QByteArrayLiteral("4B075FAE-3402-4C71-B245-8C085EECBDC0")};
    QTest::newRow("49") << QStringLiteral("Scanner Drive To Zero") << QUuid{QByteArrayLiteral("DBCA2D41-68CE-4849-9C53-FA70489FD5F4")};
    QTest::newRow("50") << QStringLiteral("Scanner Jump Speed") << QUuid{QByteArrayLiteral("DED8CFEC-7E89-4C0A-AE3D-A03F70C9E28A")};
    QTest::newRow("51") << QStringLiteral("Laser on delay") << QUuid{QByteArrayLiteral("5F479CF6-777F-4CEC-83C1-486B1A9B2407")};
    QTest::newRow("52") << QStringLiteral("Laser off delay") << QUuid{QByteArrayLiteral("E13815FB-DD58-46A8-8000-F7D3D693A539")};
    QTest::newRow("53") << QStringLiteral("Adaptive Exposure Basic Value") << QUuid{QByteArrayLiteral("207FE332-3C5A-44A6-B1BF-11C2F2E76EFE")};
    QTest::newRow("54") << QStringLiteral("Adaptive Exposure Mode On/Off") << QUuid{QByteArrayLiteral("A4FE4CFE-9093-4359-BA94-8E571A3CBE89")};
    QTest::newRow("55") << QStringLiteral("Scanner X Position") << QUuid{QByteArrayLiteral("553C51F1-F66D-4B36-92B4-6E84D464F833")};
    QTest::newRow("56") << QStringLiteral("Scanner Y Position") << QUuid{QByteArrayLiteral("F8DAF95D-B164-4686-8BAB-02FA473FCA94")};
    QTest::newRow("57") << QStringLiteral("Move Scanner to Position") << QUuid{QByteArrayLiteral("5C6D790E-3956-452F-825A-5D3EF1419E79")};
    QTest::newRow("58") << QStringLiteral("Lin Log: Mode") << QUuid{QByteArrayLiteral("11E8064E-CCAA-49DA-814E-6DD15B89BE21")};
    QTest::newRow("59") << QStringLiteral("Lin Log: Value 1") << QUuid{QByteArrayLiteral("E43DECF3-EB17-42AF-91ED-35354424CC9F")};
    QTest::newRow("60") << QStringLiteral("Lin Log: Value 2") << QUuid{QByteArrayLiteral("46C9FC2A-5FF6-411F-8510-16D7DD21CCB7")};
    QTest::newRow("61") << QStringLiteral("Lin Log: Time 1") << QUuid{QByteArrayLiteral("EC8E6ECD-B7BC-4E34-BF24-0BA4532650D5")};
    QTest::newRow("62") << QStringLiteral("Lin Log: Time 2") << QUuid{QByteArrayLiteral("91C51FD6-3C7F-49B5-943A-71FE504A0D8F")};
    QTest::newRow("63") << QStringLiteral("Laser Dual Channel") << QUuid{QByteArrayLiteral("822D6803-9CD3-4F2C-943A-D40391844EBF")};
    QTest::newRow("64") << QStringLiteral("Laser Power Static Ring") << QUuid{QByteArrayLiteral("87D6C0F4-1F1A-49FB-88BF-110EFDB43D2F")};
    QTest::newRow("65") << QStringLiteral("Basic wobble figure horizontal size") << QUuid{QByteArrayLiteral("85F6BA10-9DC5-4B7F-A5BD-DC3FCFD1A07F")};
    QTest::newRow("66") << QStringLiteral("Basic wobble figure vertical size") << QUuid{QByteArrayLiteral("0C1C82A0-5591-41CE-9529-710E3D6139CD")};
    QTest::newRow("67") << QStringLiteral("Basic wobble figure frequency") << QUuid{QByteArrayLiteral("0A97DA05-D9C6-482E-A8F6-022FB3B30A71")};
    QTest::newRow("68") << QStringLiteral("Wobble mode") << QUuid{QByteArrayLiteral("943B70CE-D43D-4275-B6B8-EF38164B5DAB")};
    QTest::newRow("69") << QStringLiteral("Acquisition Mode (Continuous or SingleFrame)") << QUuid{QByteArrayLiteral("D07C54CB-F40B-4315-9867-D1FD8215669E")};
    QTest::newRow("70") << QStringLiteral("Reuse last captured image (implies Acquisition Mode SingleFrame)") << QUuid{QByteArrayLiteral("78205AC3-6EEC-4262-A9CC-560A9DB2C4F6")};
    QTest::newRow("71") << QStringLiteral("LED panel 5 enabled") << QUuid{QByteArrayLiteral("2af13ae3-afe8-40c5-a965-17fcc16688a4")};
    QTest::newRow("72") << QStringLiteral("LED panel 5 intensity") << QUuid{QByteArrayLiteral("9999a6e7-3cc8-400e-8fd9-1ac6e46f4dfa")};
    QTest::newRow("73") << QStringLiteral("LED panel 5 pulse width") << QUuid{QByteArrayLiteral("220fb733-9806-4786-841d-40b87ae86906")};
    QTest::newRow("74") << QStringLiteral("LED panel 6 enabled") << QUuid{QByteArrayLiteral("c0346188-f822-4464-9e48-ee6f431f7296")};
    QTest::newRow("75") << QStringLiteral("LED panel 6 intensity") << QUuid{QByteArrayLiteral("ef5e0749-1b6b-4720-a093-93170e8ba56c")};
    QTest::newRow("76") << QStringLiteral("LED panel 6 pulse width") << QUuid{QByteArrayLiteral("407aab66-e111-4a7b-92b7-5b3c7b092363")};
    QTest::newRow("77") << QStringLiteral("LED panel 7 enabled") << QUuid{QByteArrayLiteral("fc3e4e89-893b-4ba1-90c7-9f7a8bf9acfc")};
    QTest::newRow("78") << QStringLiteral("LED panel 7 intensity") << QUuid{QByteArrayLiteral("b11f4f1e-7e3f-4e6d-b58f-061dd27b4ff2")};
    QTest::newRow("79") << QStringLiteral("LED panel 7 pulse width") << QUuid{QByteArrayLiteral("24c6ba71-687f-4c48-8f7f-b5348ae96ecd")};
    QTest::newRow("80") << QStringLiteral("LED panel 8 enabled") << QUuid{QByteArrayLiteral("42072523-e15c-4158-aaf4-5acb53f3e822")};
    QTest::newRow("81") << QStringLiteral("LED panel 8 intensity") << QUuid{QByteArrayLiteral("74a95dfe-5e76-4849-aed9-1a4e2e8012b3")};
    QTest::newRow("82") << QStringLiteral("LED panel 8 pulse width") << QUuid{QByteArrayLiteral("52495c6b-8d64-4dd4-9ef4-0af2e06437fb")};
    QTest::newRow("83") << QStringLiteral("Move Scanner to Position With OCT Reference") << QUuid{QByteArrayLiteral("837fdee9-86d5-41d2-827f-1a6b0f5ece86")};
    QTest::newRow("84") << QStringLiteral("Rotate figure") << QUuid{QByteArrayLiteral("ca12aeaa-fe98-43c9-81ec-4c3291aaf6c6")};
    QTest::newRow("85") << QStringLiteral("Laser delay") << QUuid{QByteArrayLiteral("6c42bf43-e7d7-4065-9d9b-f16036909c7b")};
    QTest::newRow("86") << QStringLiteral("ScanTracker2D Custom Figure") << QUuid{QByteArrayLiteral("49cf4844-c664-4145-8a72-a2950277c1fd")};
    QTest::newRow("87") << QStringLiteral("Scale in welding direction") << QUuid{QByteArrayLiteral("74cc5fca-7f73-4538-927b-ada54093db66")};
    QTest::newRow("88") << QStringLiteral("Scale vertical to welding direction") << QUuid{QByteArrayLiteral("fb056e33-d080-4251-b17a-9a59fef5b0fd")};
    QTest::newRow("89") << QStringLiteral("ScanTracker2D: Fixed scan-position (X)") << QUuid{QByteArrayLiteral("020f8b29-e0c3-4922-b02f-438389619d51")};
    QTest::newRow("90") << QStringLiteral("ScanTracker2D: Fixed scan-position (Y)") << QUuid{QByteArrayLiteral("7144e9f8-166a-47bd-a40a-72c874878cbc")};

    QTest::newRow("not translated") << QStringLiteral("LEDSendData") << QUuid{QByteArrayLiteral("F6D4C84B-9F71-4054-8245-7DB93DBBF81E")};
}

void HardwareParameterProductModelTest::testTranslations()
{
    AttributeModel am;
    QSignalSpy modelResetSpy{&am, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    am.load(QFINDTESTDATA("../../wm_inst/system_graphs/keyValueAttributes.json"));
    QVERIFY(modelResetSpy.wait());
    QFETCH(QUuid, id);
    auto *attribute = am.findAttribute(id);
    QVERIFY(attribute);
    QTEST(HardwareParameterProductModel::nameForAttribute(attribute), "name");
}

QTEST_GUILESS_MAIN(HardwareParameterProductModelTest)
#include "hardwareParameterProductModelTest.moc"

