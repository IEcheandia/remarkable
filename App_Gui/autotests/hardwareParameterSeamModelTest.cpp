#include <QTest>
#include <QSignalSpy>

#include "../src/hardwareParameterSeamModel.h"
#include "../src/hardwareParameters.h"
#include "attribute.h"
#include "attributeModel.h"
#include "parameter.h"
#include "product.h"
#include "seam.h"

using precitec::gui::HardwareParameterSeamModel;
using precitec::gui::HardwareParameters;
using precitec::storage::Attribute;
using precitec::storage::AttributeModel;
using precitec::storage::Parameter;
using precitec::storage::Product;
using precitec::storage::Seam;

class HardwareParameterSeamModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testDisplayRole_data();
    void testDisplayRole();
    void testKey_data();
    void testKey();
    void testSetSeam();
    void testSetAttributeModel();
    void testFindAttribute();
    void testEnable();
    void testUpdateParameter();

private:
    QTemporaryDir m_dir;
};

void HardwareParameterSeamModelTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void HardwareParameterSeamModelTest::testCtor()
{
    HardwareParameterSeamModel model;
    QVERIFY(!model.seam());
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

void HardwareParameterSeamModelTest::testDisplayRole_data()
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

void HardwareParameterSeamModelTest::testDisplayRole()
{
    HardwareParameterSeamModel model;
    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data().toString(), "name");
}

void HardwareParameterSeamModelTest::testKey_data()
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

void HardwareParameterSeamModelTest::testKey()
{
    HardwareParameterSeamModel model;
    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole).value<HardwareParameters::Key>(), "key");
}

void HardwareParameterSeamModelTest::testSetSeam()
{
    HardwareParameterSeamModel model;
    QSignalSpy dataChangedSpy(&model, &HardwareParameterSeamModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    QSignalSpy seamChangedSpy(&model, &HardwareParameterSeamModel::seamChanged);
    QVERIFY(seamChangedSpy.isValid());

    // create a Product
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());

    QCOMPARE(dataChangedSpy.count(), 0);
    model.setSeam(s);
    QCOMPARE(model.seam(), s);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);

    // setting same should not change
    model.setSeam(s);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);

    auto ps = model.getParameterSet();
    QVERIFY(ps);
    QCOMPARE(s->hardwareParameters(), ps);
    QCOMPARE(model.getParameterSet(), ps);

    s->deleteLater();
    QVERIFY(seamChangedSpy.wait());
    QCOMPARE(seamChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.count(), 2);

    QVERIFY(!model.getParameterSet());
}

void HardwareParameterSeamModelTest::testSetAttributeModel()
{
    HardwareParameterSeamModel model;
    QSignalSpy dataChangedSpy(&model, &HardwareParameterSeamModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    QSignalSpy attributeModelChangedSpy(&model, &HardwareParameterSeamModel::attributeModelChanged);
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

void HardwareParameterSeamModelTest::testFindAttribute()
{
    HardwareParameterSeamModel model;
    AttributeModel am;
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/attributes.json"));
    QSignalSpy dataChangedSpy(&model, &HardwareParameterSeamModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(am.rowCount(), 2);

    auto attributeEsposureTime = model.index(12, 0).data(Qt::UserRole + 2).value<Attribute*>();
    QVERIFY(attributeEsposureTime);
    auto attributeLED1Intensity = model.index(19, 0).data(Qt::UserRole + 2).value<Attribute*>();
    QVERIFY(attributeLED1Intensity);
    // others should not have an attribute
    for (int i = 0; i < 29; i++)
    {
        if (i != 12 && i != 19)
        {
            QVERIFY(!model.index(i, 0).data(Qt::UserRole + 2).value<Attribute*>());
        }
    }
    QCOMPARE(attributeEsposureTime->name(), QStringLiteral("ExposureTime"));
    QCOMPARE(attributeLED1Intensity->name(), QStringLiteral("LEDPanel1Intensity"));
}

void HardwareParameterSeamModelTest::testEnable()
{
    HardwareParameterSeamModel model;
    AttributeModel am;
    QSignalSpy modelResetSpy{&am, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QSignalSpy dataChangedSpy(&model, &HardwareParameterSeamModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    // create a Product
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());
    model.setSeam(s);
    QCOMPARE(dataChangedSpy.count(), 1);

    QSignalSpy hasChangesChangedSpy{&model, &HardwareParameterSeamModel::markAsChanged};
    QVERIFY(hasChangesChangedSpy.isValid());
    QCOMPARE(model.index(12, 0).data(Qt::UserRole + 1).toBool(), false);
    QCOMPARE(dataChangedSpy.count(), 1);
    model.setEnable(HardwareParameters::Key::ExposureTime, true);
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(hasChangesChangedSpy.count(), 1);
    QCOMPARE(model.index(12, 0).data(Qt::UserRole + 1).toBool(), true);

    auto parameter = model.index(12, 0).data(Qt::UserRole + 3).value<Parameter*>();
    QVERIFY(parameter);
    QCOMPARE(parameter->name(), QStringLiteral("ExposureTime"));

    QSignalSpy parameterDestroyedSpy{parameter, &Parameter::destroyed};
    QVERIFY(parameterDestroyedSpy.isValid());
    model.setEnable(HardwareParameters::Key::ExposureTime, false);
    QVERIFY(!model.index(12, 0).data(Qt::UserRole + 3).value<Parameter*>());
    QVERIFY(parameterDestroyedSpy.wait());
    QCOMPARE(hasChangesChangedSpy.count(), 2);

    // unset seam should result in dataChanged
    s->deleteLater();
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(dataChangedSpy.count(), 4);

    // no seam, so setEnable shouldn't work
    model.setEnable(HardwareParameters::Key::ExposureTime, true);
    QCOMPARE(dataChangedSpy.count(), 4);
}

void HardwareParameterSeamModelTest::testUpdateParameter()
{
    HardwareParameterSeamModel model;
    AttributeModel am;
    model.setAttributeModel(&am);
    am.load(QFINDTESTDATA("testdata/attributes.json"));
    QSignalSpy dataChangedSpy(&model, &HardwareParameterSeamModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    QVERIFY(dataChangedSpy.wait());
    QSignalSpy parameterChangedSpy{&model, &HardwareParameterSeamModel::parameterChanged};
    QVERIFY(parameterChangedSpy.isValid());

    // create a Product
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    QVERIFY(!s->hardwareParameters());
    model.setSeam(s);

    QSignalSpy hasChangesChangedSpy{&model, &HardwareParameterSeamModel::markAsChanged};
    QVERIFY(hasChangesChangedSpy.isValid());

    // parameter not yet created
    model.updateHardwareParameter(HardwareParameters::Key::ExposureTime, {3.0});
    QCOMPARE(hasChangesChangedSpy.count(), 0);
    // so let's create the Parameter
    QCOMPARE(parameterChangedSpy.count(), 0);
    model.setEnable(HardwareParameters::Key::ExposureTime, true);
    QCOMPARE(parameterChangedSpy.count(), 1);
    auto parameter = model.index(12, 0).data(Qt::UserRole + 3).value<Parameter*>();
    QVERIFY(parameter);
    QCOMPARE(hasChangesChangedSpy.count(), 1);

    // let's change the parameter
    QCOMPARE(parameterChangedSpy.count(), 1);
    model.updateHardwareParameter(HardwareParameters::Key::ExposureTime, {3.0});
    QCOMPARE(hasChangesChangedSpy.count(), 2);
    QCOMPARE(parameterChangedSpy.count(), 2);
    QCOMPARE(parameter->value().toReal(), 3.0);
}

QTEST_GUILESS_MAIN(HardwareParameterSeamModelTest)
#include "hardwareParameterSeamModelTest.moc"
