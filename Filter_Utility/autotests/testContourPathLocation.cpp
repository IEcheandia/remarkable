#include <QTest>

#include "../contourPathLocation.cpp"

namespace precitec
{
namespace filter
{

class TestContourPathLocation : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testcontourLocalInfo();
};

void TestContourPathLocation::testcontourLocalInfo()
{
    struct Point
    {
        double x;
        double y;
    };

    const std::vector<Point> contourIn =
    {
        {0, 0},
        {1, 1},
    };

    double x;
    double y;
    double nx;
    double ny;

    // Test Case 1:
    // Contour start point
    contourLocalInfo(contourIn, 0.0, x, y, nx, ny);
    QCOMPARE(x, 0.0);
    QCOMPARE(y, 0.0);
    QCOMPARE(nx, 1 / std::sqrt(2));
    QCOMPARE(ny, -1 / std::sqrt(2));

    // Test Case 2:
    // Contour end point
    contourLocalInfo(contourIn, 1.0, x, y, nx, ny);
    QCOMPARE(x, 1.0);
    QCOMPARE(y, 1.0);
    QCOMPARE(nx, 1 / std::sqrt(2));
    QCOMPARE(ny, -1 / std::sqrt(2));
}

} //namespace filter
} //namespace precitec

QTEST_GUILESS_MAIN(precitec::filter::TestContourPathLocation)
#include "testContourPathLocation.moc"
