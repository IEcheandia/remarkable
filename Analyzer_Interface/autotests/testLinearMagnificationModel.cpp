#include <QTest>
#include "coordinates/linearMagnificationModel.h"

class TestLinearMagnificationModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testAbsoluteZ();
};


using namespace precitec::math;
using namespace precitec::geo2d;
using namespace precitec::coordinates;

void TestLinearMagnificationModel::testAbsoluteZ()
{
    double beta0 = 0.1;
    double betaZ = 0.2;
    bool invertX = true;
    bool invertY = false;
    bool isHighPlaneOnImageTop = true;
    double dPix = 0.066;
    Point pointOrigin = {512,512};
    DPoint point1 {10, 20};
    DPoint point2 {20, 110};
    LineEquation laserLineOnXYPlane {{point1.x, point2.x}, {point1.y, point2.y}};
    LinearMagnificationModel model (beta0, betaZ, invertX, invertY, isHighPlaneOnImageTop,
        dPix, pointOrigin, laserLineOnXYPlane
    );


    //test change of reference system for points
    for (auto pointXY : {DPoint{0.0, laserLineOnXYPlane.getY(0.0)},
                        DPoint{100.0, laserLineOnXYPlane.getY(100.0)},
                        point1, point2})
    {
        auto point1L = model.transformToLaserLineAxis(pointXY);
        auto newPointXY =  model.transformToXYPlane(point1L);
        QVERIFY(isClose(pointXY.x, newPointXY.x, 1e-12));
        QVERIFY(isClose(pointXY.y, newPointXY.y, 1e-12));
    }

    //test change of reference system for line
    for (auto lineXY : {laserLineOnXYPlane, LineEquation(0.0,1.0, 0.0), LineEquation(1.0, 0.0, 200.0)})
    {
        auto lineL = model.transformToLaserLineAxis(lineXY);
        auto newLineXY =  model.transformToXYPlane(lineL);
        double computed_a, computed_b, computed_c, expected_a, expected_b, expected_c;
        newLineXY.getCoefficients(computed_a, computed_b, computed_c, true);
        lineXY.getCoefficients(expected_a, expected_b, expected_c, true);
        std::cout << lineXY << " \n\t-> " << lineL << " \n\t-> " << newLineXY << std::endl;
        std::cout << computed_a - expected_a << std::endl;
        QVERIFY(isClose(computed_a, expected_a, 1e-12));
        QVERIFY(isClose(computed_b, expected_b, 1e-12));
        QVERIFY(isClose(computed_c, expected_c, 1e-12));
    }

    //test if laser line used for construction corresponds to z=0
    {
        auto coords = model.laserScreenCoordinatesTo3D(point1);
        QVERIFY(coords.first);
        QVERIFY(isClose((double)(coords.second.z), 0.0, 1e-5));

        auto line0 = model.getLaserLineAtZCollimatorHeight(coords.second.z);
        QVERIFY(line0.isValid());
        QVERIFY(isClose(line0.distance(point1.x, point1.y), 0.0, 1e-5));

    }

    //test change of reference for z = 0
    {
        DPoint newZReferencePoint{0.0, 0.0};
        model.adjustLineToReferenceZ(newZReferencePoint);
        auto newLine0 = model.getLaserLineAtZCollimatorHeight(0.0);
        QVERIFY(newLine0.isValid());
        QVERIFY(isClose(newLine0.distance(newZReferencePoint.x, newZReferencePoint.y), 0.0, 1e-5));
        auto coords = model.laserScreenCoordinatesTo3D(newZReferencePoint);
        QVERIFY(coords.first);
        QVERIFY(isClose((double)(coords.second.z), 0.0, 1e-5));
    }

    // test getLaserLineAtZCollimatorHeight
    for (double expected_z : {-1.0, 0.0, 1.0})
    {
        auto line = model.getLaserLineAtZCollimatorHeight(expected_z);
        QVERIFY(line.isValid());
        auto coords = model.laserScreenCoordinatesTo3D(DPoint{0.0, line.getY(0.0)});
        QVERIFY(coords.first);
        QVERIFY(isClose((double)(coords.second.z), expected_z, 1e-5));
    }

}



QTEST_MAIN(TestLinearMagnificationModel)
#include "testLinearMagnificationModel.moc"
