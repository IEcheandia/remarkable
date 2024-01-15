#include <QTest>
#include "../ramping.h"

namespace testFunction
{
    double threeDigits(double value)
    {
        return std::round(value * 1000) / 1000;
    }
}

class RampingTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCtor();
    void testRampLength();
    void testRampStartPower();
    void testRampEndPower();
    void testRampStartPowerRing();
    void testRampEndPowerRing();
    void testRampStep();
    void testReversePoints();
    void testCreateRamp();
    void testLengthFromVector();
    void testVectorFromPoints();
    void testLinearInterpolationCorePower_data();
    void testLinearInterpolationCorePower();
    void testLinearInterpolationRingPower_data();
    void testLinearInterpolationRingPower();
};

void RampingTest::testCtor()
{
    Ramping ramping;
    QCOMPARE(ramping.length(), 0.0);
    QCOMPARE(ramping.startPower(), 0.0);
    QCOMPARE(ramping.endPower(), 0.0);
    QCOMPARE(ramping.startPowerRing(), 0.0);
    QCOMPARE(ramping.endPowerRing(), 0.0);
    QCOMPARE(ramping.rampStep(), 0.01);
}

void RampingTest::testRampLength()
{
    Ramping ramping;
    double length = 0.8;
    ramping.setLength(length);
    QCOMPARE(ramping.length(), length);
    length = 1.5;
    ramping.setLength(length);
    QCOMPARE(ramping.length(), length);
}

void RampingTest::testRampStartPower()
{
    Ramping ramping;
    double power = 0.2;
    ramping.setStartPower(power);
    QCOMPARE(ramping.startPower(), power);
    power = 1.5;
    ramping.setStartPower(power);
    QCOMPARE(ramping.startPower(), power);
}

void RampingTest::testRampEndPower()
{
    Ramping ramping;
    double power = 0.2;
    ramping.setEndPower(power);
    QCOMPARE(ramping.endPower(), power);
    power = 1.0;
    ramping.setEndPower(power);
    QCOMPARE(ramping.endPower(), power);
}

void RampingTest::testRampStartPowerRing()
{
    Ramping ramping;
    double powerRing = 0.5;
    ramping.setStartPowerRing(powerRing);
    QCOMPARE(ramping.startPowerRing(), powerRing);
    powerRing = 0.3;
    ramping.setStartPowerRing(powerRing);
    QCOMPARE(ramping.startPowerRing(), powerRing);
}

void RampingTest::testRampEndPowerRing()
{
    Ramping ramping;
    double powerRing = 0.9;
    ramping.setEndPowerRing(powerRing);
    QCOMPARE(ramping.endPowerRing(), powerRing);
    powerRing = 0.75;
    ramping.setEndPowerRing(powerRing);
    QCOMPARE(ramping.endPowerRing(), powerRing);
}

void RampingTest::testRampStep()
{
    Ramping ramping;
    double rampStep = 0.1;
    ramping.setRampStep(rampStep);
    QCOMPARE(ramping.rampStep(), rampStep);
    rampStep = 0.05;
    ramping.setRampStep(rampStep);
    QCOMPARE(ramping.rampStep(), rampStep);
}

void RampingTest::testReversePoints()
{
    Ramping ramping;

    std::vector<precitec::geo2d::TPoint<double>> testPoints = {precitec::geo2d::TPoint<double>(0.5, 0), precitec::geo2d::TPoint<double>(1.0, 0), precitec::geo2d::TPoint<double>(2.0, 0), precitec::geo2d::TPoint<double>(5.0, 0)};
    std::vector<double> power {0.1, 0.3, 0.2, 1.0};
    std::vector<double> powerRing {0.25, 0.15, 0.75, 0.5};
    std::vector<double> velocity {100.0, 250.0, 375.5, 1000.0};
    precitec::geo2d::AnnotatedDPointarray testData {testPoints.size(), precitec::geo2d::TPoint<double>(0.0, 0.0), 255};
    testData.getData() = testPoints;
    testData.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower) = power;
    testData.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing) = powerRing;
    testData.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity) = velocity;

    const auto& reversedContour = ramping.reversePoints(testData);

    std::vector<precitec::geo2d::TPoint<double>> resultPoints = {precitec::geo2d::TPoint<double>(5.0, 0), precitec::geo2d::TPoint<double>(2.0, 0), precitec::geo2d::TPoint<double>(1.0, 0), precitec::geo2d::TPoint<double>(0.5, 0)};
    precitec::geo2d::AnnotatedDPointarray resultData {resultPoints.size(), precitec::geo2d::TPoint<double>(0.0, 0.0), 255};
    resultData.getData() = resultPoints;

    QCOMPARE(reversedContour.size(), resultData.size());

    for (std::size_t i = 0; i < reversedContour.size(); i++)
    {
        QCOMPARE(reversedContour.getData().at(i).x, resultData.getData().at(i).x);
        QCOMPARE(reversedContour.getData().at(i).y, resultData.getData().at(i).y);
        QCOMPARE(reversedContour.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower), power);
        QCOMPARE(reversedContour.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing), powerRing);
        QCOMPARE(reversedContour.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity), velocity);
    }
}

void RampingTest::testCreateRamp()
{
    Ramping ramping;

    const auto& length = 3.4;
    const auto& rampStep = 0.1;

    ramping.setLength(length);
    ramping.setStartPower(0.35);
    ramping.setEndPower(0.15);
    ramping.setStartPowerRing(0.1);
    ramping.setEndPowerRing(0.2);
    ramping.setRampStep(rampStep);

    std::vector<precitec::geo2d::TPoint<double>> testPoints {precitec::geo2d::TPoint<double>(0.5, 0), precitec::geo2d::TPoint<double>(1.0, 0), precitec::geo2d::TPoint<double>(2.0, 0), precitec::geo2d::TPoint<double>(5.0, 0)};
    std::vector<double> testVelocities {100.0, 200.0, 300.0, 100.0};
    precitec::geo2d::AnnotatedDPointarray testData {testPoints.size(), precitec::geo2d::TPoint<double>(0.0, 0.0), 255};
    testData.getData() = testPoints;
    testData.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity) = testVelocities;

    const auto& ramp = ramping.createRamp(testData, 0);

    std::vector<precitec::geo2d::TPoint<double>> resultPoints;
    for (std::size_t i = 0; i < (length / rampStep) + 1; i++)
    {
        resultPoints.emplace_back((rampStep * i) + testPoints.front().x, 0.0);
    }
    precitec::geo2d::AnnotatedDPointarray resultData {resultPoints.size(), precitec::geo2d::TPoint<double>(0.0, 0.0), 255};
    resultData.getData() = resultPoints;

    QCOMPARE(ramp.size(), (length / rampStep) + 1);

    for (std::size_t i = 0; i < ramp.size(); i++)
    {
        QCOMPARE(ramp.getData().at(i).x, resultData.getData().at(i).x);
        QCOMPARE(ramp.getData().at(i).y, resultData.getData().at(i).y);
    }

    std::vector<double> resultPower = {ramping.startPower()};
    const auto& slope = (ramping.endPower() - ramping.startPower()) / length;
    for (std::size_t i = 1; i < ramp.size(); i++)
    {
        resultPower.emplace_back(slope * rampStep + resultPower.back());
    }

    QVERIFY(ramp.hasScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower));

    QCOMPARE(ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower).size(), ramp.size());
    QCOMPARE(ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower).size(), resultPower.size());


    for (std::size_t i = 0; i < ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower).size(); i++)
    {
        const auto& powers = ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower);
        QCOMPARE(testFunction::threeDigits(powers.at(i)), testFunction::threeDigits(resultPower.at(i)));
    }

    std::vector<double> resultPowerRing = {ramping.startPowerRing()};
    const auto& slopeRing = (ramping.endPowerRing() - ramping.startPowerRing()) / length;
    for (std::size_t i = 1; i < ramp.size(); i++)
    {
        resultPowerRing.emplace_back(slopeRing * rampStep + resultPowerRing.back());
    }

    QVERIFY(ramp.hasScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing));

    QCOMPARE(ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing).size(), ramp.size());
    QCOMPARE(ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing).size(), resultPower.size());

    for (std::size_t i = 0; i < ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing).size(); i++)
    {
        const auto& ringPowers = ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing);
        QCOMPARE(testFunction::threeDigits(ringPowers.at(i)), testFunction::threeDigits(resultPowerRing.at(i)));
    }

    std::vector<double> resultVelocity = {100.0, -1.0, -1.0, -1.0, -1.0, 200.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 300.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0};
    QCOMPARE(resultVelocity.size(), ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity).size());

    QVERIFY(ramp.hasScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity));

    QCOMPARE(ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity).size(), ramp.size());
    QCOMPARE(ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity).size(), resultPower.size());


    for (std::size_t i = 0; i < ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity).size(); i++)
    {
        const auto& velocity = ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity);
        QCOMPARE(testFunction::threeDigits(velocity.at(i)), testFunction::threeDigits(resultVelocity.at(i)));
    }
}

void RampingTest::testLengthFromVector()
{
    Ramping ramping;

    std::vector<precitec::geo2d::TPoint<double>> testPoints = {precitec::geo2d::TPoint<double>(0.5, 0.0), precitec::geo2d::TPoint<double>(1.0, 1.0), precitec::geo2d::TPoint<double>(0.0, 0.0), precitec::geo2d::TPoint<double>(-5.0, 1.0), precitec::geo2d::TPoint<double>(-2.5, -1.0)};

    std::vector<double> lengths {0.5, 1.414, 0.0, 5.099, 2.693};

    QCOMPARE(testPoints.size(), lengths.size());

    std::size_t i = 0;
    for (const auto& element : testPoints)
    {
        QCOMPARE(testFunction::threeDigits(ramping.lengthFromVector(element)), lengths.at(i));
        i++;
    }
}

void RampingTest::testVectorFromPoints()
{
    Ramping ramping;

    precitec::geo2d::TPoint<double> p0 = precitec::geo2d::TPoint<double>(0.5, 0.0);
    precitec::geo2d::TPoint<double> p1 = precitec::geo2d::TPoint<double>(1.5, 0.0);
    precitec::geo2d::TPoint<double> vector = ramping.vectorFromPoints(p0, p1);

    precitec::geo2d::TPoint<double> expectedVector = precitec::geo2d::TPoint<double>(1.0, 0.0);

    QCOMPARE(vector, expectedVector);

    p0 = precitec::geo2d::TPoint<double>(-0.5, 0.5);
    p1 = precitec::geo2d::TPoint<double>(-2.5, -0.25);
    vector = ramping.vectorFromPoints(p0, p1);
    expectedVector = precitec::geo2d::TPoint<double>(-2.0, -0.75);

    QCOMPARE(vector, expectedVector);
}

void RampingTest::testLinearInterpolationCorePower_data()
{
    QTest::addColumn<double>("startPower");
    QTest::addColumn<double>("endPower");
    QTest::addColumn<double>("maxLength");
    QTest::addColumn<std::vector<double>>("lengths");
    QTest::addColumn<std::vector<double>>("expectedPower");

    QTest::newRow("ZeroToHundred") << 0.0 << 1.0 << 0.5 << std::vector<double> {0.0, 0.1, 0.25, 0.3, 0.5} << std::vector<double> {0.0, 0.2, 0.5, 0.6, 1.0};
    QTest::newRow("FiftyToHundred") << 0.0 << 0.5 << 0.25 << std::vector<double> {0.0, 0.01, 0.05, 0.15, 0.2, 0.25} << std::vector<double> {0.0, 0.02, 0.1, 0.3, 0.4, 0.5};
    QTest::newRow("TwentyFiveToTen") << 0.25 << 0.1 << 1.5 << std::vector<double> {0.0, 0.01, 0.5, 0.75, 1.2, 0.145} << std::vector<double> {0.25, 0.249, 0.2, 0.175, 0.13, 0.236};
}

void RampingTest::testLinearInterpolationCorePower()
{
    Ramping ramp;

    QFETCH(double, startPower);
    ramp.setStartPower(startPower);

    QFETCH(double, endPower);
    ramp.setEndPower(endPower);

    QFETCH(double, maxLength);
    ramp.setLength(maxLength);

    QFETCH(std::vector<double>, lengths);
    std::vector<double> resultPower;

    for (const auto& length : lengths)
    {
        resultPower.emplace_back(testFunction::threeDigits(ramp.linearInterpolationCorePower(length)));
    }

    QFETCH(std::vector<double>, expectedPower);
    QCOMPARE(resultPower, expectedPower);
}

void RampingTest::testLinearInterpolationRingPower_data()
{
    QTest::addColumn<double>("startPower");
    QTest::addColumn<double>("endPower");
    QTest::addColumn<double>("maxLength");
    QTest::addColumn<std::vector<double>>("lengths");
    QTest::addColumn<std::vector<double>>("expectedPower");

    QTest::newRow("ZeroToHundred") << 0.0 << 1.0 << 0.5 << std::vector<double> {0.0, 0.1, 0.25, 0.3, 0.5} << std::vector<double> {0.0, 0.2, 0.5, 0.6, 1.0};
    QTest::newRow("FiftyToHundred") << 0.0 << 0.5 << 0.25 << std::vector<double> {0.0, 0.01, 0.05, 0.15, 0.2, 0.25} << std::vector<double> {0.0, 0.02, 0.1, 0.3, 0.4, 0.5};
    QTest::newRow("TwentyFiveToTen") << 0.25 << 0.1 << 1.5 << std::vector<double> {0.0, 0.01, 0.5, 0.75, 1.2, 0.145} << std::vector<double> {0.25, 0.249, 0.2, 0.175, 0.13, 0.236};
}

void RampingTest::testLinearInterpolationRingPower()
{
    Ramping ramp;

    QFETCH(double, startPower);
    ramp.setStartPowerRing(startPower);

    QFETCH(double, endPower);
    ramp.setEndPowerRing(endPower);

    QFETCH(double, maxLength);
    ramp.setLength(maxLength);

    QFETCH(std::vector<double>, lengths);
    std::vector<double> resultPower;

    for (const auto& length : lengths)
    {
        resultPower.emplace_back(testFunction::threeDigits(ramp.linearInterpolationRingPower(length)));
    }

    QFETCH(std::vector<double>, expectedPower);
    QCOMPARE(resultPower, expectedPower);
}

QTEST_GUILESS_MAIN(RampingTest)
#include "rampingTest.moc"
