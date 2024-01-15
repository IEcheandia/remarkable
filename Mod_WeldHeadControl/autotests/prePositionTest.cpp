#include <QTest>
#include <cmath>

#include "../include/viWeldHead/Scanlab/Scanlab.h"

/*
 * This class is used to check the definition of the pre position for accelerate the scanner to avoid burning in at the beginning.
 */

namespace testHelper
{
struct TestHelper
{
    std::pair<double, double> limitPrecisionToThreeDigits(const std::pair<double, double>& point)
    {
        std::pair<int, int> pointCopy = std::make_pair(point.first * 1000, point.second * 1000);
        return std::pair<double, double> {pointCopy.first / 1000.0, pointCopy.second / 1000.0};
    }
    double calculateLengthFromTwoPoints(const std::pair<double, double>& firstPoint, const std::pair<double, double>& secondPoint)
    {
        return std::sqrt(( std::pow(secondPoint.first - firstPoint.first, 2) + std::pow(secondPoint.second - firstPoint.second, 2)));
    }
};
}

class PrePositionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testLaserDelay();
    void testScannerMarkSpeed();
    void testPrePositionUnitCircle_data();
    void testPrePositionUnitCircle();
    void testPrePositionOrigin_data();
    void testPrePositionOrigin();
    void testPrePosition_data();
    void testPrePosition();

private:
    QTemporaryDir m_dir;
};

void PrePositionTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void PrePositionTest::testCtor()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    QCOMPARE(scanlabClass->GetLaserDelay(), 0.0);
    QCOMPARE(scanlabClass->GetScannerMarkSpeed(), 0.1);
    auto& figureWelding = scanlabClass->rtc6FigureWelding();
    QCOMPARE(figureWelding.m_calibrationFactor, 0.0);
    auto calibrationFactor = 1000.0;
    figureWelding.set_CalibrationFactor(calibrationFactor);
    QCOMPARE(figureWelding.m_calibrationFactor, 1000.0);

    delete scanlabClass;
}

void PrePositionTest::testLaserDelay()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    QCOMPARE(scanlabClass->GetLaserDelay(), 0.0);
    scanlabClass->SetLaserDelay(1.5);
    QCOMPARE(scanlabClass->GetLaserDelay(), 1.5);

    delete scanlabClass;
}

void PrePositionTest::testScannerMarkSpeed()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    QCOMPARE(scanlabClass->GetScannerMarkSpeed(), 0.1);
    scanlabClass->SetScannerMarkSpeed(1000.0);
    QCOMPARE(scanlabClass->GetScannerMarkSpeed(), 1000.0);

    auto& figureWelding = scanlabClass->rtc6FigureWelding();
    double jumpSpeed;
    double markSpeed;
    int wobbleFrequency;
    figureWelding.get_Speeds(jumpSpeed, markSpeed, wobbleFrequency);
    QCOMPARE(markSpeed, 400.0);
    figureWelding.set_Speeds(scanlabClass->GetScannerJumpSpeed(), scanlabClass->GetScannerMarkSpeed(), scanlabClass->GetScannerWobbleFrequency());
    figureWelding.get_Speeds(jumpSpeed, markSpeed, wobbleFrequency);
    QCOMPARE(markSpeed, 1000.0);

    delete scanlabClass;
}

typedef std::pair<double, double> point;
void PrePositionTest::testPrePositionUnitCircle_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<point>("firstPoint");
    QTest::addColumn<point>("secondPoint");
    QTest::addColumn<point>("prePosition");
    QTest::addColumn<double>("prePositionLength");
    QTest::addColumn<double>("prePositionTime");

    QTest::newRow("posX_Axis") << 0 << std::make_pair(0.0, 0.0) << std::make_pair(1.0, 0.0) << std::make_pair(-1.0, 0.0) << 1.0 << 1.0;
    QTest::newRow("posY_Axis") << 0 << std::make_pair(0.0, 0.0) << std::make_pair(0.0, 1.0) << std::make_pair(0.0, -1.0) << 1.0 << 1.0;
    QTest::newRow("45_degree") << 0 << std::make_pair(0.0, 0.0) << std::make_pair(1.0, 1.0) << std::make_pair(-0.707, -0.707) << 1.0 << 1.0;
    QTest::newRow("mirroredPosX_Axis") << 0 << std::make_pair(1.0, 0.0) << std::make_pair(0.0, 0.0) << std::make_pair(2.0, 0.0) << 1.0 << 1.0;
    QTest::newRow("mirroredPosY_Axis") << 0 << std::make_pair(0.0, 1.0) << std::make_pair(0.0, 0.0) << std::make_pair(0.0, 2.0) << 1.0 << 1.0;
    QTest::newRow("mirrored45_degree") << 0 << std::make_pair(1.0, 1.0) << std::make_pair(0.0, 0.0) << std::make_pair(1.707, 1.707) << 1.0 << 1.0;
    QTest::newRow("135_degree") << 0 << std::make_pair(0.0, 0.0) << std::make_pair(-1.0, 1.0) << std::make_pair(0.707, -0.707) << 1.0 << 1.0;
    QTest::newRow("negX_Axis") << 0 << std::make_pair(0.0, 0.0) << std::make_pair(-1.0, 0.0) << std::make_pair(1.0, 0.0) << 1.0 << 1.0;
    QTest::newRow("mirrored135_degree") << 0 << std::make_pair(-1.0, 1.0) << std::make_pair(0.0, 0.0) << std::make_pair(-1.707, 1.707) << 1.0 << 1.0;
    QTest::newRow("mirroredNegX_Axis") << 0 << std::make_pair(-1.0, 0.0) << std::make_pair(0.0, 0.0) << std::make_pair(-2.0, 0.0) << 1.0 << 1.0;
    QTest::newRow("225_degree") << 0 << std::make_pair(0.0, 0.0) << std::make_pair(-1.0, -1.0) << std::make_pair(0.707, 0.707) << 1.0 << 1.0;
    QTest::newRow("negY_Axis") << 0 << std::make_pair(0.0, 0.0) << std::make_pair(0.0, -1.0) << std::make_pair(0.0, 1.0) << 1.0 << 1.0;
    QTest::newRow("mirrored225_degree") << 0 << std::make_pair(-1.0, -1.0) << std::make_pair(0.0, 0.0) << std::make_pair(-1.707, -1.707) << 1.0 << 1.0;
    QTest::newRow("mirroredNegY_Axis") << 0 << std::make_pair(0.0, -1.0) << std::make_pair(0.0, 0.0) << std::make_pair(0.0, -2.0) << 1.0 << 1.0;
    QTest::newRow("315_degree") << 0 << std::make_pair(0.0, 0.0) << std::make_pair(1.0, -1.0) << std::make_pair(-0.707, 0.707) << 1.0 << 1.0;
    QTest::newRow("mirrored315_degree") << 0 << std::make_pair(1.0, -1.0) << std::make_pair(0.0, 0.0) << std::make_pair(1.707, -1.707) << 1.0 << 1.0;
}

void PrePositionTest::testPrePositionUnitCircle()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    auto& figureWelding = scanlabClass->rtc6FigureWelding();
    scanlabClass->SetScannerMarkSpeed(1.0);
    figureWelding.set_Speeds(0.0, scanlabClass->GetScannerMarkSpeed(), 0);
    figureWelding.set_CalibrationFactor(1);

    auto calculatedPoint = figureWelding.definePrePosition(scanlabClass->GetLaserDelay(), std::make_pair(0.0, 0.0), std::make_pair(1.0, 0.0));

    QCOMPARE(calculatedPoint.first, 0.0);
    QCOMPARE(calculatedPoint.second, 0.0);

    testHelper::TestHelper helper;

    auto prePositionLength = 0.0;
    auto timePrePositionToFirstPosition = 0.0;
    scanlabClass->SetLaserDelay(1.0);

    QFETCH(point, firstPoint);
    QFETCH(point, secondPoint);
    calculatedPoint = figureWelding.definePrePosition(scanlabClass->GetLaserDelay(), firstPoint, secondPoint);
    QTEST(helper.limitPrecisionToThreeDigits(calculatedPoint), "prePosition");
    prePositionLength = helper.calculateLengthFromTwoPoints(firstPoint, calculatedPoint);
    QTEST(prePositionLength, "prePositionLength");
    timePrePositionToFirstPosition = prePositionLength / (figureWelding.m_oMarkSpeed / figureWelding.m_calibrationFactor);
    QTEST(timePrePositionToFirstPosition, "prePositionTime");

    delete scanlabClass;
}

void PrePositionTest::testPrePositionOrigin_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<point>("firstPoint");
    QTest::addColumn<point>("secondPoint");
    QTest::addColumn<point>("prePosition");
    QTest::addColumn<double>("prePositionLength");
    QTest::addColumn<double>("prePositionTime");

    QTest::newRow("0_degree") << 0 << std::make_pair(-1.0, 0.0) << std::make_pair(1.0, 0.0) << std::make_pair(-51.0, 0.0) << 50.0 << 0.5;
    QTest::newRow("30_degree") << 0 << std::make_pair(-0.866, -0.5) << std::make_pair(0.866, 0.5) << std::make_pair(-44.166, -25.5) << 50.0 << 0.5;
    QTest::newRow("60_degree") << 0 << std::make_pair(-0.5, -0.866) << std::make_pair(0.5, 0.866) << std::make_pair(-25.5, -44.166) << 50.0 << 0.5;
    QTest::newRow("90_degree") << 0 << std::make_pair(0.0, -1.0) << std::make_pair(0.0, 1.0) << std::make_pair(0.0, -51.0) << 50.0 << 0.5;
    QTest::newRow("120_degree") << 0 << std::make_pair(0.5, -0.866) << std::make_pair(-0.5, 0.866) << std::make_pair(25.5, -44.166) << 50.0 << 0.5;
    QTest::newRow("150_degree") << 0 << std::make_pair(0.866, -0.5) << std::make_pair(-0.866, 0.5) << std::make_pair(44.166, -25.5) << 50.0 << 0.5;
    QTest::newRow("180_degree") << 0 << std::make_pair(1.0, 0.0) << std::make_pair(-1.0, 0.0) << std::make_pair(51.0, 0.0) << 50.0 << 0.5;
    QTest::newRow("210_degree") << 0 << std::make_pair(0.866, 0.5) << std::make_pair(-0.866, -0.5) << std::make_pair(44.166, 25.5) << 50.0 << 0.5;
    QTest::newRow("240_degree") << 0 << std::make_pair(0.5, 0.866) << std::make_pair(-0.5, -0.866) << std::make_pair(25.5, 44.166) << 50.0 << 0.5;
    QTest::newRow("270_degree") << 0 << std::make_pair(0.0, 1.0) << std::make_pair(0.0, -1.0) << std::make_pair(0.0, 51.0) << 50.0 << 0.5;
    QTest::newRow("300_degree") << 0 << std::make_pair(-0.5, 0.866) << std::make_pair(0.5, -0.866) << std::make_pair(-25.5, 44.166) << 50.0 << 0.5;
    QTest::newRow("330_degree") << 0 << std::make_pair(-0.866, 0.5) << std::make_pair(0.866, -0.5) << std::make_pair(-44.166, 25.5) << 50.0 << 0.5;
}

void PrePositionTest::testPrePositionOrigin()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    auto& figureWelding = scanlabClass->rtc6FigureWelding();

    //Speed = 100 m / s
    scanlabClass->SetScannerMarkSpeed(400.0);
    figureWelding.set_Speeds(0.0, scanlabClass->GetScannerMarkSpeed(), 0);
    figureWelding.set_CalibrationFactor(4);

    testHelper::TestHelper helper;

    auto prePositionLength = 0.0;
    auto timePrePositionToFirstPosition = 0.0;
    scanlabClass->SetLaserDelay(0.5);

    QFETCH(point, firstPoint);
    QFETCH(point, secondPoint);
    auto calculatedPoint = figureWelding.definePrePosition(scanlabClass->GetLaserDelay(), firstPoint, secondPoint);
    QTEST(helper.limitPrecisionToThreeDigits(calculatedPoint), "prePosition");
    prePositionLength = helper.calculateLengthFromTwoPoints(firstPoint, calculatedPoint);
    QTEST(prePositionLength, "prePositionLength");
    timePrePositionToFirstPosition = prePositionLength / (figureWelding.m_oMarkSpeed / figureWelding.m_calibrationFactor);
    QTEST(timePrePositionToFirstPosition, "prePositionTime");

    delete scanlabClass;
}

void PrePositionTest::testPrePosition_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<point>("firstPoint");
    QTest::addColumn<point>("secondPoint");
    QTest::addColumn<point>("prePosition");
    QTest::addColumn<double>("prePositionLength");
    QTest::addColumn<double>("prePositionTime");

    QTest::newRow("1") << 0 << std::make_pair(1.0, 1.0) << std::make_pair(5.0, 3.0) << std::make_pair(-12.416, -5.708) << 15.0 << 1.5;
    QTest::newRow("2") << 0 << std::make_pair(5.0, 3.0) << std::make_pair(1.0, 1.0) << std::make_pair(18.416, 9.708) << 15.0 << 1.5;
    QTest::newRow("3") << 0 << std::make_pair(-7.5, 0.5) << std::make_pair(-3.5, 1.5) << std::make_pair(-22.052, -3.138) << 15.0 << 1.5;
    QTest::newRow("4") << 0 << std::make_pair(-3.5, 1.5 ) << std::make_pair(-7.5, 0.5) << std::make_pair(11.052, 5.138) << 15.0 << 1.5;
    QTest::newRow("5") << 0 << std::make_pair(-0.75, -0.9) << std::make_pair(-1.5, -2.0) << std::make_pair(7.7, 11.493) << 15.0 << 1.5;
    QTest::newRow("6") << 0 << std::make_pair(-1.5, -2.0) << std::make_pair(-0.75, -0.9) << std::make_pair(-9.95, -14.393) << 15.0 << 1.5;
    QTest::newRow("7") << 0 << std::make_pair(9.5, -5.0) << std::make_pair(5.25, -0.1) << std::make_pair(19.328, -16.331) << 15.0 << 1.5;
    QTest::newRow("8") << 0 << std::make_pair(5.25, -0.1) << std::make_pair(9.5, -5.0) << std::make_pair(-4.578, 11.231) << 15.0 << 1.5;
}

void PrePositionTest::testPrePosition()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    auto& figureWelding = scanlabClass->rtc6FigureWelding();

    //Speed = 10 m / s
    scanlabClass->SetScannerMarkSpeed(100.0);
    figureWelding.set_Speeds(0.0, scanlabClass->GetScannerMarkSpeed(), 0);
    figureWelding.set_CalibrationFactor(10);

    testHelper::TestHelper helper;

    auto prePositionLength = 0.0;
    auto timePrePositionToFirstPosition = 0.0;
    scanlabClass->SetLaserDelay(1.5);

    QFETCH(point, firstPoint);
    QFETCH(point, secondPoint);
    auto calculatedPoint = figureWelding.definePrePosition(scanlabClass->GetLaserDelay(), firstPoint, secondPoint);
    QTEST(helper.limitPrecisionToThreeDigits(calculatedPoint), "prePosition");
    prePositionLength = helper.calculateLengthFromTwoPoints(firstPoint, calculatedPoint);
    QTEST(prePositionLength, "prePositionLength");
    timePrePositionToFirstPosition = prePositionLength / (figureWelding.m_oMarkSpeed / figureWelding.m_calibrationFactor);
    QTEST(timePrePositionToFirstPosition, "prePositionTime");

    delete scanlabClass;
}

QTEST_GUILESS_MAIN(PrePositionTest)
#include "prePositionTest.moc"
