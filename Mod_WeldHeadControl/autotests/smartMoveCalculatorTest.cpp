#include <QTest>
#include <QDebug>

#include "../include/viWeldHead/Scanlab/smartMoveCalculator.h"
#include "common/definesSmartMove.h"

using precitec::hardware::SmartMoveCalculator;
using precitec::smartMove::DriveToLimits;
using precitec::smartMove::ScanfieldBits;

namespace
{
double limitToThreeDigits(double valueToLimit)
{
    return static_cast<double>(static_cast<int>((valueToLimit * 1000))) / 1000.0;
}
}

/**
 *  This class is used to check the SmartMove calculations from the SmartMove calculator class.
 **/
class SmartMoveCalculatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSetScanfieldSize();
    void testCalculateDriveToBits();
    void testCalculateDriveToMillimeters();
    void testCalculateMillimeterToBits();
    void testCalculateBitsToMillimeter();
    void testCalculateBitsPerMillisecondsFromMeterPerSecond();
    void testCalculateMeterPerSecondFromBitsPerMilliseconds();
    void testCalculateMillimeterFromMeter();
    void testCalculateMeterFromMillimeter();
    void testCalculateMicroMeterFromMillimeter();
    void testCalculateMillimeterFromMicroMeter();
    void testCalculateNanometerFromMicrometer();
    void testCalculateMicrometerFromNanometer();
    void testCalculateResolutionInNanometer();
    void testConversionFactor();
};

void SmartMoveCalculatorTest::testCtor()
{
    SmartMoveCalculator calc;

    QCOMPARE(calc.scanfieldSize(), 50);
}

void SmartMoveCalculatorTest::testSetScanfieldSize()
{
    SmartMoveCalculator calc;

    QCOMPARE(calc.scanfieldSize(), 50);
    calc.setScanfieldSize(100);
    QCOMPARE(calc.scanfieldSize(), 100);
    calc.setScanfieldSize(1000);
    QCOMPARE(calc.scanfieldSize(), 1000);
}

void SmartMoveCalculatorTest::testCalculateDriveToBits()
{
    SmartMoveCalculator calc;

    calc.setScanfieldSize(0);
    QCOMPARE(calc.calculateDriveToBits(0), DriveToLimits::OriginScanfield);
    QCOMPARE(calc.calculateDriveToBits(100), DriveToLimits::OriginScanfield);
    QCOMPARE(calc.calculateDriveToBits(-100), DriveToLimits::OriginScanfield);

    calc.setScanfieldSize(50);
    QCOMPARE(calc.calculateDriveToBits(0), DriveToLimits::OriginScanfield);
    QCOMPARE(calc.calculateDriveToBits(50), DriveToLimits::PositiveScanfield);
    QCOMPARE(calc.calculateDriveToBits(-50), DriveToLimits::NegativeScanfield);
    QCOMPARE(calc.calculateDriveToBits(25), DriveToLimits::PositiveScanfield / 2);
    QCOMPARE(calc.calculateDriveToBits(-25), DriveToLimits::NegativeScanfield / 2);
    QCOMPARE(calc.calculateDriveToBits(10), DriveToLimits::PositiveScanfield / 5);
    QCOMPARE(calc.calculateDriveToBits(-10), DriveToLimits::NegativeScanfield / 5);

    calc.setScanfieldSize(100);
    QCOMPARE(calc.calculateDriveToBits(0), DriveToLimits::OriginScanfield);
    QCOMPARE(calc.calculateDriveToBits(50), DriveToLimits::PositiveScanfield / 2);
    QCOMPARE(calc.calculateDriveToBits(-50), DriveToLimits::NegativeScanfield / 2);
    QCOMPARE(calc.calculateDriveToBits(75), DriveToLimits::PositiveScanfield * 3 / 4);
    QCOMPARE(calc.calculateDriveToBits(-75), DriveToLimits::NegativeScanfield * 3 / 4);
}

void SmartMoveCalculatorTest::testCalculateDriveToMillimeters()
{
    SmartMoveCalculator calc;

    calc.setScanfieldSize(0);
    QCOMPARE(calc.calculateDriveToMillimeter(0), DriveToLimits::OriginScanfield);
    QCOMPARE(calc.calculateDriveToMillimeter(100), DriveToLimits::OriginScanfield);
    QCOMPARE(calc.calculateDriveToMillimeter(-100), DriveToLimits::OriginScanfield);

    calc.setScanfieldSize(500);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::OriginScanfield), 0);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::PositiveScanfield), 500);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::NegativeScanfield), -500);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::PositiveScanfield / 2), 249);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::NegativeScanfield / 2), -250);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::PositiveScanfield / 4), 124);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::NegativeScanfield / 4), -125);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::PositiveScanfield / 5), 99);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::NegativeScanfield / 5), -99);

    calc.setScanfieldSize(100);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::OriginScanfield), 0);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::PositiveScanfield / 2), 49);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::NegativeScanfield / 2), -50);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::PositiveScanfield * 3 / 4), 74);
    QCOMPARE(calc.calculateDriveToMillimeter(DriveToLimits::NegativeScanfield * 3 / 4), -75);
}

void SmartMoveCalculatorTest::testCalculateMillimeterToBits()
{
    SmartMoveCalculator calc;

    calc.setScanfieldSize(0);
    QCOMPARE(calc.calculateMillimeterToBits(0), ScanfieldBits::Origin);
    QCOMPARE(calc.calculateMillimeterToBits(50), ScanfieldBits::Origin);
    QCOMPARE(calc.calculateMillimeterToBits(-50), ScanfieldBits::Origin);

    calc.setScanfieldSize(50);
    QCOMPARE(calc.calculateMillimeterToBits(0.0), ScanfieldBits::Origin);
    QCOMPARE(calc.calculateMillimeterToBits(-50.0), ScanfieldBits::Min);
    QCOMPARE(calc.calculateMillimeterToBits(50.0), ScanfieldBits::Max);
    QCOMPARE(calc.calculateMillimeterToBits(-25.0), ScanfieldBits::Min / 2);
    QCOMPARE(calc.calculateMillimeterToBits(25.0), ScanfieldBits::Max / 2);
    QCOMPARE(calc.calculateMillimeterToBits(-5.0), ScanfieldBits::Min / 10);
    QCOMPARE(calc.calculateMillimeterToBits(5.0), ScanfieldBits::Max / 10);

    calc.setScanfieldSize(100);
    QCOMPARE(calc.calculateMillimeterToBits(0.0), ScanfieldBits::Origin);
    QCOMPARE(calc.calculateMillimeterToBits(-100.0), ScanfieldBits::Min);
    QCOMPARE(calc.calculateMillimeterToBits(100.0), ScanfieldBits::Max);
    QCOMPARE(calc.calculateMillimeterToBits(-75.5), -6333398);
    QCOMPARE(calc.calculateMillimeterToBits(75.5), 6333398);
    QCOMPARE(calc.calculateMillimeterToBits(-5.0), ScanfieldBits::Min / 20);
    QCOMPARE(calc.calculateMillimeterToBits(5.0), ScanfieldBits::Max / 20);
}

void SmartMoveCalculatorTest::testCalculateBitsToMillimeter()
{
    SmartMoveCalculator calc;

    calc.setScanfieldSize(0);
    QCOMPARE(calc.calculateBitsToMillimeter(ScanfieldBits::Origin), ScanfieldBits::Origin);
    QCOMPARE(calc.calculateBitsToMillimeter(ScanfieldBits::Min), ScanfieldBits::Origin);
    QCOMPARE(calc.calculateBitsToMillimeter(ScanfieldBits::Max), ScanfieldBits::Origin);

    calc.setScanfieldSize(50);
    QCOMPARE(calc.calculateBitsToMillimeter(ScanfieldBits::Origin), 0.0);
    QCOMPARE(calc.calculateBitsToMillimeter(ScanfieldBits::Min), -50.0);
    QCOMPARE(calc.calculateBitsToMillimeter(ScanfieldBits::Max), 50.0);
    QCOMPARE(limitToThreeDigits(calc.calculateBitsToMillimeter(ScanfieldBits::Min / 2)), -24.999);
    QCOMPARE(limitToThreeDigits(calc.calculateBitsToMillimeter(ScanfieldBits::Max / 2)), 24.999);
    QCOMPARE(limitToThreeDigits(calc.calculateBitsToMillimeter(ScanfieldBits::Min / 10)), -4.999);
    QCOMPARE(limitToThreeDigits(calc.calculateBitsToMillimeter(ScanfieldBits::Max / 10)), 4.999);

    calc.setScanfieldSize(100);
    QCOMPARE(calc.calculateBitsToMillimeter(ScanfieldBits::Origin), 0.0);
    QCOMPARE(calc.calculateBitsToMillimeter(ScanfieldBits::Min), -100.0);
    QCOMPARE(calc.calculateBitsToMillimeter(ScanfieldBits::Max), 100.0);
    QCOMPARE(limitToThreeDigits(calc.calculateBitsToMillimeter(-6333398)), -75.499);
    QCOMPARE(limitToThreeDigits(calc.calculateBitsToMillimeter(6333398)), 75.499);
    QCOMPARE(limitToThreeDigits(calc.calculateBitsToMillimeter(ScanfieldBits::Min / 20)), -4.999);
    QCOMPARE(limitToThreeDigits(calc.calculateBitsToMillimeter(ScanfieldBits::Max / 20)), 4.999);
}

void SmartMoveCalculatorTest::testCalculateBitsPerMillisecondsFromMeterPerSecond()
{
    SmartMoveCalculator calc;

    calc.setScanfieldSize(5000);
    QCOMPARE(static_cast<int>(calc.calculateBitsPerMillisecondsFromMeterPerSecond(5)), 8389);
    QCOMPARE(static_cast<int>(calc.calculateBitsPerMillisecondsFromMeterPerSecond(10)), 16778);
    QCOMPARE(static_cast<int>(calc.calculateBitsPerMillisecondsFromMeterPerSecond(20)), 33557);

    calc.setScanfieldSize(100);
    QCOMPARE(static_cast<int>(calc.calculateBitsPerMillisecondsFromMeterPerSecond(1)), 90909);
    QCOMPARE(static_cast<int>(calc.calculateBitsPerMillisecondsFromMeterPerSecond(10)), 909090);
    QCOMPARE(static_cast<int>(calc.calculateBitsPerMillisecondsFromMeterPerSecond(12)), 1090909);
}

void SmartMoveCalculatorTest::testCalculateMeterPerSecondFromBitsPerMilliseconds()
{
    SmartMoveCalculator calc;

    calc.setScanfieldSize(5000);
    QCOMPARE(calc.calculateMeterPerSecondFromBitsPerMilliseconds(8389), 4.999844);
    QCOMPARE(calc.calculateMeterPerSecondFromBitsPerMilliseconds(16778), 9.999688);
    QCOMPARE(calc.calculateMeterPerSecondFromBitsPerMilliseconds(33557), 19.999972);

    calc.setScanfieldSize(100);
    QCOMPARE(calc.calculateMeterPerSecondFromBitsPerMilliseconds(90909), 0.999999);
    QCOMPARE(calc.calculateMeterPerSecondFromBitsPerMilliseconds(909090), 9.99999);
    QCOMPARE(calc.calculateMeterPerSecondFromBitsPerMilliseconds(1090909), 11.999999);
}

void SmartMoveCalculatorTest::testCalculateMillimeterFromMeter()
{
    SmartMoveCalculator calc;

    QCOMPARE(calc.calculateMillimeterFromMeter(100), 100000);
    QCOMPARE(calc.calculateMillimeterFromMeter(1000), 1000000);
    QCOMPARE(calc.calculateMillimeterFromMeter(50), 50000);
    QCOMPARE(calc.calculateMillimeterFromMeter(2), 2000);
}

void SmartMoveCalculatorTest::testCalculateMeterFromMillimeter()
{
    SmartMoveCalculator calc;

    QCOMPARE(calc.calculateMeterFromMillimeter(50), 0.05);
    QCOMPARE(calc.calculateMeterFromMillimeter(5000), 5);
    QCOMPARE(calc.calculateMeterFromMillimeter(10000), 10);
    QCOMPARE(calc.calculateMeterFromMillimeter(15000), 15);
    QCOMPARE(calc.calculateMeterFromMillimeter(250000), 250);
}

void SmartMoveCalculatorTest::testCalculateMicroMeterFromMillimeter()
{
    SmartMoveCalculator calc;

    QCOMPARE(calc.calculateMicrometerFromMillimeter(100), 100000);
    QCOMPARE(calc.calculateMicrometerFromMillimeter(1000), 1000000);
    QCOMPARE(calc.calculateMicrometerFromMillimeter(50), 50000);
    QCOMPARE(calc.calculateMicrometerFromMillimeter(2), 2000);
}

void SmartMoveCalculatorTest::testCalculateMillimeterFromMicroMeter()
{
    SmartMoveCalculator calc;

    QCOMPARE(calc.calculateMillimeterFromMicrometer(50), 0.05);
    QCOMPARE(calc.calculateMillimeterFromMicrometer(5000), 5);
    QCOMPARE(calc.calculateMillimeterFromMicrometer(10000), 10);
    QCOMPARE(calc.calculateMillimeterFromMicrometer(15000), 15);
    QCOMPARE(calc.calculateMillimeterFromMicrometer(250000), 250);
}

void SmartMoveCalculatorTest::testCalculateNanometerFromMicrometer()
{
    SmartMoveCalculator calc;

    QCOMPARE(calc.calculateNanometerFromMicrometer(100), 100000);
    QCOMPARE(calc.calculateNanometerFromMicrometer(1000), 1000000);
    QCOMPARE(calc.calculateNanometerFromMicrometer(50), 50000);
    QCOMPARE(calc.calculateNanometerFromMicrometer(2), 2000);
}

void SmartMoveCalculatorTest::testCalculateMicrometerFromNanometer()
{
    SmartMoveCalculator calc;

    QCOMPARE(calc.calculateMicrometerFromNanometer(50), 0.05);
    QCOMPARE(calc.calculateMicrometerFromNanometer(5000), 5);
    QCOMPARE(calc.calculateMicrometerFromNanometer(10000), 10);
    QCOMPARE(calc.calculateMicrometerFromNanometer(15000), 15);
    QCOMPARE(calc.calculateMicrometerFromNanometer(250000), 250);
}

void SmartMoveCalculatorTest::testCalculateResolutionInNanometer()
{
    SmartMoveCalculator calc;

    QCOMPARE(calc.calculateResolutionInNanometer(), 5);

    calc.setScanfieldSize(100);
    QCOMPARE(calc.calculateResolutionInNanometer(), 11);

    calc.setScanfieldSize(25);
    QCOMPARE(calc.calculateResolutionInNanometer(), 2);
}

void SmartMoveCalculatorTest::testConversionFactor()
{
    SmartMoveCalculator calc;

    int scanfieldSize = 0;
    calc.setScanfieldSize(scanfieldSize);
    QCOMPARE(calc.conversionFactor(), 0.0);

    scanfieldSize = 100;
    calc.setScanfieldSize(scanfieldSize);
    QCOMPARE(calc.conversionFactor(), ScanfieldBits::Max / static_cast<double>(scanfieldSize));

    scanfieldSize = 50;
    calc.setScanfieldSize(scanfieldSize);
    QCOMPARE(calc.conversionFactor(), ScanfieldBits::Max / static_cast<double>(scanfieldSize));

    scanfieldSize = 5;
    calc.setScanfieldSize(scanfieldSize);
    QCOMPARE(calc.conversionFactor(), ScanfieldBits::Max / static_cast<double>(scanfieldSize));

    scanfieldSize = 3;
    calc.setScanfieldSize(scanfieldSize);
    QCOMPARE(calc.conversionFactor(), ScanfieldBits::Max / static_cast<double>(scanfieldSize));
}

QTEST_GUILESS_MAIN(SmartMoveCalculatorTest)
#include "smartMoveCalculatorTest.moc"
