#include <QTest>
#include <QTemporaryDir>

#include "../include/viWeldHead/Scanlab/Scanlab.h"

namespace testHelper
{
struct TestHelper
{
    double limitPrecisionToSixDigits(const double& value)
    {
        auto valueCopy = static_cast<int> (std::round(value * 1000000));
        return valueCopy / 1000000.0;
    }
};
}

/*
 * This class is used to check the definition of the pre position for accelerate the scanner to avoid burning in at the beginning.
 */
class PositionDifferenceToleranceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testPositionDifferenceTolerance_data();
    void testPositionDifferenceTolerance();

private:
    QTemporaryDir m_dir;
};

void PositionDifferenceToleranceTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void PositionDifferenceToleranceTest::testCtor()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    QCOMPARE(scanlabClass->getPositionDifferenceTolerance(), 0.5);

    delete scanlabClass;
}

void PositionDifferenceToleranceTest::testPositionDifferenceTolerance_data()
{
    QTest::addColumn<double>("toleranceInMillimeter");
    QTest::addColumn<int>("scanlabBits");
    QTest::addColumn<double>("toleranceInPercent");
    QTest::addColumn<int>("bits");

    QTest::newRow("Bit01") << 0.004 << 16 << 0.000015 << 1;
    QTest::newRow("Bit10") << 0.063 << 252 << 0.00024 << 16;
    QTest::newRow("Bit14") << 0.07875 << 315 << 0.0003 << 20;
    QTest::newRow("Bit64") << 0.39325 << 1573 << 0.0015 << 100;
    QTest::newRow("BitB7") << 0.7195 << 2878 << 0.002745 << 183;
    QTest::newRow("BitFF") << 1.00275 << 4011 << 0.003825 << 255;
}

void PositionDifferenceToleranceTest::testPositionDifferenceTolerance()
{
    auto scanlabClass = new precitec::hardware::Scanlab;
    testHelper::TestHelper helper;

    //Calibration factor is 4000
    QFETCH(double, toleranceInMillimeter);
    QFETCH(int, bits);
    //Mm to bit
    const auto toleranceInScanlabBitFromMillimeter = scanlabClass->transformToleranceInMillimeterToScanlabBit(toleranceInMillimeter);
    QTEST(toleranceInScanlabBitFromMillimeter, "scanlabBits");
    const auto toleranceInPercentFromScanlabBit = helper.limitPrecisionToSixDigits(scanlabClass->transformToleranceInScanlabBitToPercent(toleranceInScanlabBitFromMillimeter));
    QTEST(toleranceInPercentFromScanlabBit, "toleranceInPercent");
    const auto toleranceInBitFromPercent = scanlabClass->transformToleranceInPercentToBit(toleranceInPercentFromScanlabBit);
    QTEST(toleranceInBitFromPercent, "bits");
    //Bit to mm
    const auto toleranceInPercentFromBits = helper.limitPrecisionToSixDigits(scanlabClass->transformToleranceInBitToPercent(bits));
    QTEST(toleranceInPercentFromBits, "toleranceInPercent");
    const auto toleranceInScanlabBitFromPercent = scanlabClass->transformToleranceInPercentToScanlabBit(toleranceInPercentFromBits);
    QTEST(toleranceInScanlabBitFromPercent, "scanlabBits");
    const auto toleranceInMillimeterFromScanlabBit = scanlabClass->transformToleranceInScanlabBitToMillimeter(toleranceInScanlabBitFromPercent);
    QTEST(toleranceInMillimeterFromScanlabBit, "toleranceInMillimeter");

    delete scanlabClass;
}

QTEST_GUILESS_MAIN(PositionDifferenceToleranceTest)
#include "positionDifferenceToleranceTest.moc"
