#include <QTest>

#include "../include/calibration/chessboardWeldPointGenerator.h"
#include <math/mathCommon.h>
#include <qrandom.h>
#include <geo/point.h>
#include <qtestcase.h>
#include <common/systemConfiguration.h>

using weldingPoints = std::vector<precitec::geo2d::DPoint>;
Q_DECLARE_METATYPE(weldingPoints);

class TestChessboardWeldPointGenerator: public QObject
{
public:
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    weldingPoints testGenerateExpectedPoints(int scanFieldWidth, int scanFieldHeight, int xMax, int yMax);
    void testChessboardWeldPointGenerator_data();
    void testChessboardWeldPointGenerator();
};


void TestChessboardWeldPointGenerator::testCtor()
{
    precitec::calibration_algorithm::ChessboardWeldPointGenerator testChessboardWeldPointGenerator(100, 100, 50, 50);
}

weldingPoints TestChessboardWeldPointGenerator::testGenerateExpectedPoints(int scanFieldWidth, int scanFieldHeight, int xMax, int yMax)
{
    weldingPoints expectedPoints;
    int index = 0;
    const auto halfScanfieldWidth = scanFieldWidth / 2;
    const auto halfScanfieldHeight = scanFieldHeight / 2;
    const auto midX = halfScanfieldWidth - (halfScanfieldWidth % 10);
    const auto midY = halfScanfieldHeight - (halfScanfieldHeight % 10);

    for (int y = midY; y >= -midY; y-=10)
    {
        for (int x = -midX; x <= midX; x+=10)
        {
            auto xCoord = x;
            if (index%2 != 0)
            {
                xCoord *= -1;
            }
            if ((std::pow(xCoord, 2) / std::pow(halfScanfieldWidth, 2) + std::pow(y, 2) / std::pow(halfScanfieldHeight, 2)) <= 1)
            {
                if (x >= -xMax && x <= xMax && y >= -yMax && y <= yMax)
                {
                    expectedPoints.emplace_back(precitec::geo2d::DPoint(xCoord, y));
                }
            }
        }
        ++index;
    }
    return expectedPoints;
}

void TestChessboardWeldPointGenerator::testChessboardWeldPointGenerator_data()
{
    QTest::addColumn<int>("lenseType");
    QTest::addColumn<int>("scanFieldWidth");
    QTest::addColumn<int>("scanFieldHeight");
    QTest::addColumn<int>("xMinScanfieldRange");
    QTest::addColumn<int>("yMinScanfieldRange");
    QTest::addColumn<int>("xMaxScanfieldRange");
    QTest::addColumn<int>("yMaxScanfieldRange");
    QTest::addColumn<weldingPoints>("expectedPoints");
    QTest::addColumn<int>("expectedNumberOfPoints");

    auto expectedPoints = testGenerateExpectedPoints(220, 104, 1000, 1000);
    QTest::addRow("scanField220x104") << 1 << 220 << 104 << -1000 << -1000 << 1000 << 1000 << expectedPoints << 185;
    expectedPoints = testGenerateExpectedPoints(220, 104, 40, 30);
    QTest::addRow("scanField220x104_40x30") << 1 << 220 << 104 << -40 << -30 << 10 << 20 << expectedPoints << 63;
    expectedPoints = testGenerateExpectedPoints(220, 104, 60, 50);
    QTest::addRow("scanField220x104_40x30") << 1 << 220 << 104 << -40 << -40 << 60 << 50 << expectedPoints << 131;
    expectedPoints = testGenerateExpectedPoints(220, 104, 0, 0);
    QTest::addRow("scanField220x104_0x0") << 1 << 220 << 104 << 0 << 0 << 0 << 0 << expectedPoints << 1;
    expectedPoints = testGenerateExpectedPoints(380, 290, 70, 80);
    QTest::addRow("scanField380x290_70x80") << 2 << 380 << 290 << -70 << -80 << 10 << 10 << expectedPoints << 255;
    expectedPoints = testGenerateExpectedPoints(170, 100, 1000, 1000);
    QTest::addRow("scanField170x100") << 3 << 170 << 100 << -1000 << -1000 << 1000 << 1000 << expectedPoints << 131;
}


void TestChessboardWeldPointGenerator::testChessboardWeldPointGenerator()
{
    QFETCH(int, lenseType);
    QFETCH(int, xMinScanfieldRange);
    QFETCH(int, yMinScanfieldRange);
    QFETCH(int, xMaxScanfieldRange);
    QFETCH(int, yMaxScanfieldRange);
    QFETCH(weldingPoints, expectedPoints);
    QFETCH(int, expectedNumberOfPoints);
    precitec::interface::SystemConfiguration::instance().setInt("ScanlabScanner_Lens_Type", lenseType);
    precitec::calibration_algorithm::ChessboardWeldPointGenerator testChessboardWeldPointGenerator(xMinScanfieldRange, yMinScanfieldRange, xMaxScanfieldRange, yMaxScanfieldRange);
    const auto points = testChessboardWeldPointGenerator.getWeldingPoints();
    QCOMPARE(points.size(), expectedNumberOfPoints);
    QCOMPARE(points, expectedPoints);
}

QTEST_MAIN(TestChessboardWeldPointGenerator)
#include "testChessboardWeldPointGenerator.moc"
