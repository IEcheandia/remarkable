#include <QTest>
#include <QTemporaryDir>

#include "../include/viWeldHead/Scanlab/Scanlab.h"

using precitec::LoggerSignals;
using precitec::StatusSignals;

Q_DECLARE_METATYPE(LoggerSignals)
Q_DECLARE_METATYPE(StatusSignals)

/*
 * This class is used to check the debug mode of the scanlab scanner.
 */

class ScanlabLoggerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testSetParameters();
    void testSetLoggedSignals_data();
    void testSetLoggedSignals();
    void testSetWrongLoggedSignals();
    void testSetStatusSignals_data();
    void testSetStatusSignals();
    void testSetWrongStatusSignal();
    void testSaveDataToFile();

private:
    QTemporaryDir m_dir;
};

void ScanlabLoggerTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void ScanlabLoggerTest::testCtor()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    QCOMPARE(scanlabClass->GetEnableDebugMode(), false);
    QCOMPARE(scanlabClass->rtc6FigureWelding().get_EnableDebugMode(), false);
    QCOMPARE(scanlabClass->GetMeasurementPeriod(), 1);
    QCOMPARE(scanlabClass->rtc6FigureWelding().get_MeasurementPeriod(), 1);
    QCOMPARE(static_cast<LoggerSignals>(scanlabClass->getLoggedSignal(0)), LoggerSignals::NoSignal);
    QCOMPARE(scanlabClass->rtc6FigureWelding().getLoggedSignal(0), LoggerSignals::NoSignal);
    QCOMPARE(static_cast<LoggerSignals>(scanlabClass->getLoggedSignal(1)), LoggerSignals::NoSignal);
    QCOMPARE(scanlabClass->rtc6FigureWelding().getLoggedSignal(1), LoggerSignals::NoSignal);
    QCOMPARE(static_cast<LoggerSignals>(scanlabClass->getLoggedSignal(2)), LoggerSignals::NoSignal);
    QCOMPARE(scanlabClass->rtc6FigureWelding().getLoggedSignal(2), LoggerSignals::NoSignal);
    QCOMPARE(static_cast<LoggerSignals>(scanlabClass->getLoggedSignal(3)), LoggerSignals::NoSignal);
    QCOMPARE(scanlabClass->rtc6FigureWelding().getLoggedSignal(3), LoggerSignals::NoSignal);
    QCOMPARE(static_cast<StatusSignals>(scanlabClass->GetStatusSignalHead1Axis1()), StatusSignals::NoSignal);
    QCOMPARE(static_cast<StatusSignals>(scanlabClass->GetStatusSignalHead1Axis2()), StatusSignals::NoSignal);

    delete scanlabClass;
}

void ScanlabLoggerTest::testSetParameters()
{
    auto scanlabClass = new precitec::hardware::Scanlab();

    scanlabClass->SetEnableDebugMode(true);
    QCOMPARE(scanlabClass->GetEnableDebugMode(), true);
    QCOMPARE(scanlabClass->rtc6FigureWelding().get_EnableDebugMode(), true);
    auto newPeriod = 100;
    scanlabClass->SetMeasurementPeriod(newPeriod);
    QCOMPARE(scanlabClass->GetMeasurementPeriod(), newPeriod);
    QCOMPARE(scanlabClass->rtc6FigureWelding().get_MeasurementPeriod(), newPeriod);

    delete scanlabClass;
}

void ScanlabLoggerTest::testSetLoggedSignals_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<LoggerSignals>("LoggerSignal");

    QTest::newRow("0") << -1 << LoggerSignals::NoSignal;
    QTest::newRow("1") << 0 << LoggerSignals::LaserOn;
    QTest::newRow("2") << 1 << LoggerSignals::StatusAx;
    QTest::newRow("3") << 2 << LoggerSignals::StatusAy;
    QTest::newRow("4") << 4 << LoggerSignals::StatusBx;
    QTest::newRow("5") << 5 << LoggerSignals::StatusBy;
    QTest::newRow("6") << 7 << LoggerSignals::SampleX;
    QTest::newRow("7") << 8 << LoggerSignals::SampleY;
    QTest::newRow("8") << 9 << LoggerSignals::SampleZ;
    QTest::newRow("9") << 10 << LoggerSignals::SampleAx_Corr;
    QTest::newRow("10") << 11 << LoggerSignals::SampleAy_Corr;
    QTest::newRow("11") << 12 << LoggerSignals::SampleAz_Corr;
    QTest::newRow("12") << 13 << LoggerSignals::SampleBx_Corr;
    QTest::newRow("13") << 14 << LoggerSignals::SampleBy_Corr;
    QTest::newRow("14") << 15 << LoggerSignals::SampleBz_Corr;
    QTest::newRow("15") << 16 << LoggerSignals::StatusAx_LaserOn;
    QTest::newRow("16") << 17 << LoggerSignals::StatusAy_LaserOn;
    QTest::newRow("17") << 18 << LoggerSignals::StatusBx_LaserOn;
    QTest::newRow("18") << 19 << LoggerSignals::StatusBy_LaserOn;
    QTest::newRow("19") << 20 << LoggerSignals::SampleAx_Output;
    QTest::newRow("20") << 21 << LoggerSignals::SampleAy_Output;
    QTest::newRow("21") << 22 << LoggerSignals::SampleBx_Output;
    QTest::newRow("22") << 23 << LoggerSignals::SampleBy_Output;
    QTest::newRow("23") << 24 << LoggerSignals::LaserControl;
    QTest::newRow("24") << 25 << LoggerSignals::SampleAx_Transform;
    QTest::newRow("25") << 26 << LoggerSignals::SampleAy_Transform;
    QTest::newRow("26") << 27 << LoggerSignals::SampleAz_Transform;
    QTest::newRow("27") << 28 << LoggerSignals::SampleBx_Transform;
    QTest::newRow("28") << 29 << LoggerSignals::SampleBy_Transform;
    QTest::newRow("29") << 30 << LoggerSignals::SampleBz_Transform;
    QTest::newRow("30") << 31 << LoggerSignals::VectorControl;
    QTest::newRow("31") << 32 << LoggerSignals::FocusShift;
    QTest::newRow("32") << 33 << LoggerSignals::Analog_Out1;
    QTest::newRow("33") << 34 << LoggerSignals::Analog_Out2;
    QTest::newRow("34") << 35 << LoggerSignals::Digital_Out16Bit;
    QTest::newRow("35") << 36 << LoggerSignals::Digital_Out8Bit;
    QTest::newRow("36") << 37 << LoggerSignals::PulseLenght;
    QTest::newRow("37") << 38 << LoggerSignals::HalfPeriod;
    QTest::newRow("38") << 39 << LoggerSignals::Free0;
    QTest::newRow("39") << 40 << LoggerSignals::Free1;
    QTest::newRow("40") << 41 << LoggerSignals::Free2;
    QTest::newRow("41") << 42 << LoggerSignals::Free3;
    QTest::newRow("42") << 43 << LoggerSignals::Encoder0;
    QTest::newRow("43") << 44 << LoggerSignals::Encoder1;
    QTest::newRow("44") << 45 << LoggerSignals::Mark_Speed;
    QTest::newRow("45") << 46 << LoggerSignals::DigitalIn_Extension1;
    QTest::newRow("46") << 47 << LoggerSignals::Zoom;
    QTest::newRow("47") << 48 << LoggerSignals::Free4;
    QTest::newRow("48") << 49 << LoggerSignals::Free5;
    QTest::newRow("49") << 50 << LoggerSignals::Free6;
    QTest::newRow("50") << 51 << LoggerSignals::Free7;
    QTest::newRow("51") << 52 << LoggerSignals::TimeStamp;
    QTest::newRow("52") << 53 << LoggerSignals::WobbleAmplitude;
    QTest::newRow("53") << 54 << LoggerSignals::ReadAnalogIn;
    QTest::newRow("54") << 55 << LoggerSignals::ScaledEncoderX;
    QTest::newRow("55") << 56 << LoggerSignals::ScaledEncoderY;
    QTest::newRow("56") << 57 << LoggerSignals::ScaledEncoderZ;
}

void ScanlabLoggerTest::testSetLoggedSignals()
{
    auto scanlabClass = new precitec::hardware::Scanlab;

    QFETCH(int, row);
    scanlabClass->setLoggedSignal(0, row);
    scanlabClass->setLoggedSignal(1, row);
    scanlabClass->setLoggedSignal(2, row);
    scanlabClass->setLoggedSignal(3, row);
    QTEST(scanlabClass->getLoggedSignal(0), "row");
    QTEST(scanlabClass->getLoggedSignal(1), "row");
    QTEST(scanlabClass->getLoggedSignal(2), "row");
    QTEST(scanlabClass->getLoggedSignal(3), "row");
    QTEST(scanlabClass->rtc6FigureWelding().getLoggedSignal(0), "LoggerSignal");
    QTEST(scanlabClass->rtc6FigureWelding().getLoggedSignal(1), "LoggerSignal");
    QTEST(scanlabClass->rtc6FigureWelding().getLoggedSignal(2), "LoggerSignal");
    QTEST(scanlabClass->rtc6FigureWelding().getLoggedSignal(3), "LoggerSignal");

    delete scanlabClass;
}

void ScanlabLoggerTest::testSetWrongLoggedSignals()
{
    auto scanlabClass = new precitec::hardware::Scanlab;

    scanlabClass->setLoggedSignal(0, 58);
    QCOMPARE(scanlabClass->getLoggedSignal(0), -1);
    QCOMPARE(scanlabClass->rtc6FigureWelding().getLoggedSignal(0), LoggerSignals::NoSignal);
    scanlabClass->setLoggedSignal(1, 58);
    QCOMPARE(scanlabClass->getLoggedSignal(1), -1);
    QCOMPARE(scanlabClass->rtc6FigureWelding().getLoggedSignal(1), LoggerSignals::NoSignal);
    scanlabClass->setLoggedSignal(2, 58);
    QCOMPARE(scanlabClass->getLoggedSignal(2), -1);
    QCOMPARE(scanlabClass->rtc6FigureWelding().getLoggedSignal(2), LoggerSignals::NoSignal);
    scanlabClass->setLoggedSignal(3, 58);
    QCOMPARE(scanlabClass->getLoggedSignal(3), -1);
    QCOMPARE(scanlabClass->rtc6FigureWelding().getLoggedSignal(3), LoggerSignals::NoSignal);

    delete scanlabClass;
}

void ScanlabLoggerTest::testSetStatusSignals_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("CommandCode");

    QTest::newRow("0") << -1 << 0;
    QTest::newRow("1") << 0 << 0x0500;
    QTest::newRow("2") << 1 << 0x0501;
    QTest::newRow("3") << 2 << 0x0502;
    QTest::newRow("4") << 3 << 0x0503;
    QTest::newRow("5") << 4 << 0x0504;
    QTest::newRow("6") << 5 << 0x0505;
    QTest::newRow("7") << 6 << 0x0506;
    QTest::newRow("8") << 7 << 0x0512;
    QTest::newRow("9") << 8 << 0x0514;
    QTest::newRow("10") << 9 << 0x0515;
}

void ScanlabLoggerTest::testSetStatusSignals()
{
    auto scanlabClass = new precitec::hardware::Scanlab;

    QFETCH(int, row);
    scanlabClass->setStatusSignalHead1Axis(0, row);
    scanlabClass->setStatusSignalHead1Axis(1, row);

    QTEST(scanlabClass->GetStatusSignalHead1Axis1(), "row");
    QTEST(scanlabClass->GetStatusSignalHead1Axis2(), "row");
    QTEST(scanlabClass->rtc6FigureWelding().setStatusSignalHead1Axis(AbstractScanner2DWelding::Axis::First, static_cast<StatusSignals>(row)), "CommandCode");
    QTEST(scanlabClass->rtc6FigureWelding().setStatusSignalHead1Axis(AbstractScanner2DWelding::Axis::Second, static_cast<StatusSignals>(row)), "CommandCode");

    delete scanlabClass;
}

void ScanlabLoggerTest::testSetWrongStatusSignal()
{
    auto scanlabClass = new precitec::hardware::Scanlab;

    scanlabClass->setStatusSignalHead1Axis(0, 10);
    QCOMPARE(scanlabClass->GetStatusSignalHead1Axis1(), -1);
    QCOMPARE(scanlabClass->rtc6FigureWelding().setStatusSignalHead1Axis(AbstractScanner2DWelding::Axis::First, static_cast<StatusSignals>(10)), 0);
    scanlabClass->setStatusSignalHead1Axis(1, 10);
    QCOMPARE(scanlabClass->GetStatusSignalHead1Axis2(), -1);
    QCOMPARE(scanlabClass->rtc6FigureWelding().setStatusSignalHead1Axis(AbstractScanner2DWelding::Axis::Second, static_cast<StatusSignals>(10)), 0);

    delete scanlabClass;
}

void ScanlabLoggerTest::testSaveDataToFile()
{
    auto scanlabClass = new precitec::hardware::Scanlab;
    auto& figureWelding = scanlabClass->rtc6FigureWelding();

    std::vector<int> measurementSignalOne = {0, 1, 2, 3, 4};
    std::vector<int> measurementSignalTwo = {2, 4, 6, 8, 10};
    std::vector<int> measurementSignalThree = {4, 3, 2, 1, 0};
    std::vector<int> measurementSignalFour = {10, 8, 6, 4, 2};

    QTemporaryDir tmp;
    QDir dir{tmp.path()};
    QVERIFY(tmp.isValid());
    QVERIFY(dir.exists());

    QCOMPARE(figureWelding.saveDataToFile(dir.path().toStdString()), false);
    QVERIFY(!dir.exists("loggedScannerData.txt"));

    figureWelding.m_measurementSignals.at(0) = measurementSignalOne;
    figureWelding.setLoggedSignal(1, LoggerSignals::Analog_Out1);
    QCOMPARE(figureWelding.saveDataToFile(dir.path().toStdString()), false);
    QVERIFY(!dir.exists("loggedScannerData.txt"));

    figureWelding.setLoggedSignal(1, LoggerSignals::NoSignal);
    figureWelding.m_measurementSignals.at(0).clear();
    figureWelding.setLoggedSignal(0, LoggerSignals::Analog_Out1);
    QCOMPARE(figureWelding.saveDataToFile(dir.path().toStdString()), false);
    QVERIFY(!dir.exists("loggedScannerData.txt"));

    figureWelding.m_measurementSignals.at(0) = measurementSignalOne;
    figureWelding.setLoggedSignal(0, LoggerSignals::Analog_Out1);
    QCOMPARE(figureWelding.saveDataToFile(dir.path().toStdString()), true);
    QVERIFY(dir.exists("loggedScannerData.txt"));
    QFile dataOneSignal{dir.absoluteFilePath("loggedScannerData.txt")};

    dataOneSignal.open(QIODevice::ReadOnly);
    QCOMPARE(dataOneSignal.readAll(), QStringLiteral("Data1\n0\n1\n2\n3\n4\n"));

    figureWelding.m_measurementSignals.at(1) = measurementSignalTwo;
    figureWelding.m_measurementSignals.at(2) = measurementSignalThree;
    figureWelding.m_measurementSignals.at(3) = measurementSignalFour;

    figureWelding.setLoggedSignal(1, LoggerSignals::Analog_Out2);
    figureWelding.setLoggedSignal(2, LoggerSignals::ReadAnalogIn);
    figureWelding.setLoggedSignal(3, LoggerSignals::LaserOn);
    QCOMPARE(figureWelding.saveDataToFile(dir.path().toStdString()), true);
    QVERIFY(dir.exists("loggedScannerData.txt"));
    QFile dataAllSignals{dir.absoluteFilePath("loggedScannerData.txt")};

    dataAllSignals.open(QIODevice::ReadOnly);
    QCOMPARE(dataAllSignals.readAll(), QStringLiteral("Data1,Data2,Data3,Data4\n0,2,4,10\n1,4,3,8\n2,6,2,6\n3,8,1,4\n4,10,0,2\n"));

    delete scanlabClass;
}

QTEST_GUILESS_MAIN(ScanlabLoggerTest)
#include "scannerLoggerTest.moc"
