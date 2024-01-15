#include <QTest>

#include "../include/viWeldHead/Scanlab/calculator.h"

using precitec::hardware::welding::Calculator;

/**
 *  This class is used to check the calculations from the calculator class.
 **/
class CalculatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testCalibrationFactor();
    void testWobbleStartPosition();
    void testWobbleStartPositionIsNull();
    void testBitAndMMTransformation();
    void testVectorCalculus();
    void testLimitTo3Digits();
    void testRotateVector();
    void testAddOffset();
    void testFuzzyIsNull();
};

void CalculatorTest::testCtor()
{
    Calculator calc;

    QCOMPARE(calc.calibrationFactor(), 4000.0);
    QCOMPARE(calc.wobbleStartPosition(), std::make_pair(0.0, 0.0));
}

void CalculatorTest::testCalibrationFactor()
{
    Calculator calc;

    QCOMPARE(calc.calibrationFactor(), 4000.0);
    calc.setCalibrationFactor(2000.0);
    QCOMPARE(calc.calibrationFactor(), 2000.0);
}

void CalculatorTest::testWobbleStartPosition()
{
    Calculator calc;

    calc.setWobbleStartPosition(std::make_pair(2.5, -0.25));
    QCOMPARE(calc.wobbleStartPosition(), std::make_pair(2.5, -0.25));
    calc.setWobbleStartPosition(std::make_pair(2.5, 1.25));
    QCOMPARE(calc.wobbleStartPosition(), std::make_pair(2.5, 1.25));
}

void CalculatorTest::testWobbleStartPositionIsNull()
{
    Calculator calc;

    QVERIFY(calc.wobbleStartPositionIsNull());
    calc.setWobbleStartPosition(std::make_pair(1.0, 0.0));
    QVERIFY(!calc.wobbleStartPositionIsNull());
    calc.setWobbleStartPosition(std::make_pair(0.0, 1.0));
    QVERIFY(!calc.wobbleStartPositionIsNull());
    calc.setWobbleStartPosition(std::make_pair(0.005, 0.005));
    QVERIFY(!calc.wobbleStartPositionIsNull());
    calc.setWobbleStartPosition(std::make_pair(-0.005, 1.005));
    QVERIFY(!calc.wobbleStartPositionIsNull());
    calc.setWobbleStartPosition(std::make_pair(0.000000000000000005, 0.000000000000000005));
    QVERIFY(calc.wobbleStartPositionIsNull());
}

void CalculatorTest::testBitAndMMTransformation()
{
    Calculator calc;

    long bits = 4000;
    double millimeter = 1.0;

    QCOMPARE(calc.calculateBitsFromMM(millimeter), bits);
    QCOMPARE(calc.calculateMMFromBits(bits), millimeter);

    bits = 12000;
    millimeter = 3.0;

    QCOMPARE(calc.calculateBitsFromMM(millimeter), bits);
    QCOMPARE(calc.calculateMMFromBits(bits), millimeter);

    bits = 10000;
    millimeter = 2.5;

    QCOMPARE(calc.calculateBitsFromMM(millimeter), bits);
    QCOMPARE(calc.calculateMMFromBits(bits), millimeter);
}

void CalculatorTest::testVectorCalculus()
{
    Calculator calc;

    //Vector with same direction like x axis
    std::pair<double, double> startPoint {0.0, 0.0};
    std::pair<double, double> endPoint {1.0, 0.0};

    auto vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector, std::make_pair(1.0, 0.0));

    auto length = calc.calculateVectorLength(vector);
    QCOMPARE(length, 1.0);

    auto angleToX = calc.angleToXAxis(vector);
    QCOMPARE(angleToX, 0.0);

    auto rotatedVector = calc.rotateVector(vector, angleToX);
    QCOMPARE(rotatedVector, std::make_pair(1.0, 0.0));

    //Vector with angle < 45°
    startPoint = std::make_pair(-1.0, -1.0);
    endPoint = std::make_pair(5.0, 1.0);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector, std::make_pair(6.0, 2.0));

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 6.324);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 0.321);

    //Vector with angle 45°
    startPoint = std::make_pair(1.0, 1.0);
    endPoint = std::make_pair(2.0, 2.0);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector, std::make_pair(1.0, 1.0));

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 1.414);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 0.785);

    //Vector with angle < 90°
    startPoint = std::make_pair(0.0, 0.0);
    endPoint = std::make_pair(0.5, 1.0);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector, std::make_pair(0.5, 1.0));

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 1.118);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 1.107);

    //Vector with angle 90°
    startPoint = std::make_pair(-1.0, -1.0);
    endPoint = std::make_pair(-1.0, 1.0);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector, std::make_pair(0.0, 2.0));

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 2.0);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 1.570);

    //Vector with angle < 135°
    startPoint = std::make_pair(1.0, 1.0);
    endPoint = std::make_pair(0.9, 1.25);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector.first, -0.1);
    QCOMPARE(vector.second, 0.25);

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 0.269);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 1.951);

    //Vector with angle 135°
    startPoint = std::make_pair(1.0, -1.0);
    endPoint = std::make_pair(-1.0, 1.0);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector, std::make_pair(-2.0, 2.0));

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 2.828);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 2.356);

    //Vector with angle < 180°
    startPoint = std::make_pair(0.0, 2.0);
    endPoint = std::make_pair(-5.0, 2.5);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector, std::make_pair(-5.0, 0.5));

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 5.024);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 3.041);

    //Vector with angle 180°
    startPoint = std::make_pair(-1.0, -1.0);
    endPoint = std::make_pair(-1.1, -1.0);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector.first, -0.1);
    QCOMPARE(vector.second, 0.0);

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 0.1);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 3.141);

    /**********************************/
    //Vector with angle < 225°
    startPoint = std::make_pair(-0.6, -0.6);
    endPoint = std::make_pair(-1.6, -1.2);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector, std::make_pair(-1.0, -0.6));

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 1.166);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 3.682);

    //Vector with angle 225°
    startPoint = std::make_pair(2.5, 2.0);
    endPoint = std::make_pair(-5.0, -5.5);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector, std::make_pair(-7.5, -7.5));

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 10.606);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 3.926);

    //Vector with angle < 270°
    startPoint = std::make_pair(-1.0, 1.0);
    endPoint = std::make_pair(-1.1, 0.1);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector.first, -0.1);
    QCOMPARE(vector.second, -0.9);

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 0.905);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 4.601);

    //Vector with angle 270°
    startPoint = std::make_pair(0.0, 10.0);
    endPoint = std::make_pair(0.0, 3.0);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector, std::make_pair(0.0, -7.0));

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 7.000);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 4.712);

    //Vector with angle < 315°
    startPoint = std::make_pair(1.0, -1.0);
    endPoint = std::make_pair(1.5, -7.0);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector, std::make_pair(0.5, -6.0));

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 6.02);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 4.795);

    //Vector with angle 315
    startPoint = std::make_pair(-5.0, 1.0);
    endPoint = std::make_pair(1.0, -5.0);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector, std::make_pair(6.0, -6.0));

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 8.485);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 5.497);

    //Vector with angle < 360°
    startPoint = std::make_pair(1.0, 1.0);
    endPoint = std::make_pair(1.25, 0.8);

    vector = calc.calculateVector(startPoint, endPoint);
    QCOMPARE(vector.first, 0.25);
    QCOMPARE(vector.second, -0.2);

    length = calc.calculateVectorLength(vector);
    QCOMPARE(calc.limitTo3Digits(length), 0.32);

    angleToX = calc.angleToXAxis(vector);
    QCOMPARE(calc.limitTo3Digits(angleToX), 5.608);
}

void CalculatorTest::testRotateVector()
{
    Calculator calc;

    std::pair<double, double> vector {1.0, 1.0};
    double angle = 3.1416;

    QCOMPARE(calc.limitTo3Digits(calc.rotateVector(vector, angle)), std::make_pair(-0.999, -1.0));

    angle = 3.1416 / 2.0;
    QCOMPARE(calc.limitTo3Digits(calc.rotateVector(vector, angle)), std::make_pair(-1.0, 0.999));

    angle = 3.1416 / 4.0;
    QCOMPARE(calc.limitTo3Digits(calc.rotateVector(vector, angle)), std::make_pair(0.0, 1.414));

    angle = 2 * 3.1416;
    QCOMPARE(calc.limitTo3Digits(calc.rotateVector(vector, angle)), std::make_pair(0.999, 1.0));
}

void CalculatorTest::testAddOffset()
{
    Calculator calc;

    double value = 0.5;

    QCOMPARE(calc.addOffset(value, 0.5), 1.0);
    QCOMPARE(calc.addOffset(value, -0.5), 0.0);
    QCOMPARE(calc.addOffset(value, 0.25), 0.75);
    QCOMPARE(calc.addOffset(value, 1.5), 2.0);
}

void CalculatorTest::testFuzzyIsNull()
{
    Calculator calc;

    QVERIFY(!calc.fuzzyIsNull(500.01));
    QVERIFY(!calc.fuzzyIsNull(1.0));
    QVERIFY(!calc.fuzzyIsNull(0.005));
    QVERIFY(!calc.fuzzyIsNull(-0.005));
    QVERIFY(calc.fuzzyIsNull(0.000000000000000005));
    QVERIFY(calc.fuzzyIsNull(0.0));
}

void CalculatorTest::testLimitTo3Digits()
{
    Calculator calc;

    QCOMPARE(calc.limitTo3Digits(4.59868), 4.598);
    QCOMPARE(calc.limitTo3Digits(4.15), 4.15);
    QCOMPARE(calc.limitTo3Digits(5.00), 5.00);

    QCOMPARE(calc.limitTo3Digits(std::make_pair(3.59868, 1.59868)), std::make_pair(3.598, 1.598));
    QCOMPARE(calc.limitTo3Digits(std::make_pair(3.0, 1.15)), std::make_pair(3.0, 1.15));
    QCOMPARE(calc.limitTo3Digits(std::make_pair(3.0, 1.0)), std::make_pair(3.0, 1.0));
}

QTEST_GUILESS_MAIN(CalculatorTest)
#include "calculatorTest.moc"
