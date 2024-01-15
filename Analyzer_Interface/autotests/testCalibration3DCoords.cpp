#include <QTest>
#include "math/calibration3DCoords.h"
#include "math/calibrationStructures.h"
#include "math/Calibration3DCoordsLoader.h"
#include <util/camGridData.h>

class TestCalibration3DCoords : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCoax_data();
    void testCoax();
    void testScheimpflug();
};

using namespace precitec::math;
using precitec::geo2d::Size;
using precitec::geo2d::Point;
using precitec::filter::LaserLine;


float dist(float xa,float ya,float za,float xb,float yb,float zb)
{
    float dx = xb - xa;
    float dy = yb - ya;
    float dz = zb - za;
        
    return  std::sqrt(dx*dx + dy*dy + dz*dz);
}

void TestCalibration3DCoords::testCoax_data()
{
    QTest::addColumn<double>("beta0");
    QTest::addColumn<bool>("invertX");
    QTest::addColumn<int>("sensorWidth");
    QTest::addColumn<int>("sensorHeight");
    QTest::addColumn<double>("sensorPixelSize");
    
    QTest::addColumn<double>("expected_dx");
    QTest::addColumn<double>("expected_dy");
    QTest::addColumn<double>("expected_dz1");
    QTest::addColumn<double>("expected_dz2");
    QTest::addColumn<double>("expected_dz3");
    
    

    QTest::newRow("1024x1024") << 0.3144555989 << false << 1024 << 1024 << 0.0106 
        << 0.674181 << 0.3370904 << 0.4258422 << 0.3569345 << 0.21199989;
    QTest::newRow("1280x1024") << 0.3144555989 << false << 1280 << 1024 << 0.0066
        << 0.41977310 << 0.20988655 << 0.26514816 << 0.22224331 << 0.13199997;
        
    QTest::newRow("1024x1024_invertx") << 0.3144555989 << true << 1024 << 1024 << 0.0106 
        << -0.674181 << 0.3370904 << 0.4258422 << 0.3569345 << 0.21199989;
    QTest::newRow("1280x1024_invertx") << 0.3144555989 << true << 1280 << 1024 << 0.0066
        << -0.41977310 << 0.20988655 << 0.26514816 << 0.22224331 << 0.13199997;

}

void TestCalibration3DCoords::testCoax()
{
    QFETCH(double, beta0);
    QFETCH(bool, invertX);
    QFETCH(int, sensorWidth);
    QFETCH(int, sensorHeight);
    QFETCH(double, sensorPixelSize);
    
    CoaxCalibrationData oCoaxData;
    oCoaxData.m_oBeta0 = beta0;
    oCoaxData.m_oBetaZ = 0.2489175749;
    oCoaxData.m_oBetaZ2 = 0.2969726798;
    oCoaxData.m_oBetaZTCP = 0.5;
    oCoaxData.m_oDpixX = sensorPixelSize;
    oCoaxData.m_oDpixY = sensorPixelSize;
    oCoaxData.m_oWidth = sensorWidth;
    oCoaxData.m_oHeight = sensorHeight;
    oCoaxData.m_oOrigX = sensorWidth/2;
    oCoaxData.m_oOrigY = sensorHeight/2;
    oCoaxData.m_oAxisFactor = 1.0;
    oCoaxData.m_oHighPlaneOnImageTop = true;
    oCoaxData.m_oHighPlaneOnImageTop_2 = true;
    oCoaxData.m_oHighPlaneOnImageTop_TCP = true;
    oCoaxData.m_oInvertX = invertX;
    
    bool useOrientedLine = false;
    Calibration3DCoords coords;
    bool ok = loadCoaxModel(coords, oCoaxData, useOrientedLine);
    QVERIFY(ok);
    QVERIFY(!coords.isScheimpflugCase());
    
    Calibration3DCoords coords_inv;
    oCoaxData.m_oHighPlaneOnImageTop = !oCoaxData.m_oHighPlaneOnImageTop;
    oCoaxData.m_oHighPlaneOnImageTop_2 = !oCoaxData.m_oHighPlaneOnImageTop_2;
    oCoaxData.m_oHighPlaneOnImageTop_TCP = !oCoaxData.m_oHighPlaneOnImageTop_TCP;
    loadCoaxModel(coords_inv, oCoaxData, useOrientedLine);
    QVERIFY(!coords_inv.isScheimpflugCase());
    
    Size oExpectedSensorSize {int(oCoaxData.m_oWidth), int(oCoaxData.m_oHeight)};
    auto oSensorSize = coords.getSensorSize();
    int oWidth(-1), oHeight(-1);
    coords.getSensorSize(oWidth, oHeight);
    QCOMPARE(oSensorSize, oExpectedSensorSize);
    QCOMPARE(oWidth, oExpectedSensorSize.width);
    QCOMPARE(oHeight, oExpectedSensorSize.height);
    
    float x,y,z, x_inv, y_inv, z_inv;
    
     
    std::vector<Point> validCoordinates= {
        {0,0}, 
        {oExpectedSensorSize.width-1, 0},
        {0, oExpectedSensorSize.height -1},
        {oExpectedSensorSize.width-1,oExpectedSensorSize.height -1},
        {int(oCoaxData.m_oOrigX), int(oCoaxData.m_oOrigY)}
    };
    
    
    std::vector<Point> invalidCoordinates = {
        {-1,-1},
        {oExpectedSensorSize.width, 0},
        {0, oExpectedSensorSize.height},
        {oExpectedSensorSize.width, oExpectedSensorSize.height}
    };
    
    
    for (auto & rPoint : validCoordinates)
    {            
          
        for (int line = 0; line < (int)LaserLine::NumberLaserLines; line ++)
        {
            float x,y,z;
            bool valid = coords.to3D(x, y, z, rPoint.x, rPoint.y, LaserLine(line));
            QVERIFY(valid);
            valid = coords_inv.to3D(x_inv, y_inv, z_inv, rPoint.x, rPoint.y, LaserLine(line) );
            QVERIFY(valid);
            QCOMPARE(x_inv, x);
            QCOMPARE(y_inv, y);
            QCOMPARE(z_inv, -z);
        }        
    }
    
    for (auto & rPoint : invalidCoordinates)
    {            
        
        for (int line = 0; line < (int) LaserLine::NumberLaserLines; line ++)
        {
            float x,y,z;
            bool valid = coords.to3D(x, y, z, rPoint.x, rPoint.y, LaserLine(line));
            QVERIFY(!valid);
            valid = coords_inv.to3D(x_inv, y_inv, z_inv, rPoint.x, rPoint.y, LaserLine(line) );
            QVERIFY(!valid);
        }        
    }
    
    //check coordinates difference
    {
        Point A {100,100};
        Point B {120, 90};
        float xa,ya,za,xb,yb,zb;

        QFETCH(double, expected_dx );
        QFETCH(double, expected_dy );
        QFETCH(double, expected_dz1);
        QFETCH(double, expected_dz2);
        QFETCH(double, expected_dz3);
        
        
        {
            bool valid = coords.to3D(xa, ya, za, A.x, A.y, LaserLine::FrontLaserLine);
            QVERIFY(valid);
            valid = coords.to3D(xb, yb, zb, B.x, B.y, LaserLine::FrontLaserLine);
            QVERIFY(valid);
            double tol = 1e-7;
            QVERIFY(isClose(double(xb-xa), expected_dx, tol));
            QVERIFY(isClose(double(yb-ya), expected_dy, tol));
            QVERIFY(isClose(double(zb-za), expected_dz1, tol));
        }
        
        {
            bool valid = coords.to3D(xa, ya, za, A.x, A.y, LaserLine::BehindLaserLine);
            QVERIFY(valid);
            valid = coords.to3D(xb, yb, zb, B.x, B.y, LaserLine::BehindLaserLine);
            QVERIFY(valid);
            double tol = 1e-7;
            QVERIFY(isClose(double(xb-xa), expected_dx, tol));
            QVERIFY(isClose(double(yb-ya), expected_dy, tol));
            QVERIFY(isClose(double(zb-za), expected_dz2, tol));
        }
        
        {
            bool valid = coords.to3D(xa, ya, za, A.x, A.y, LaserLine::CenterLaserLine);
            QVERIFY(valid);
            valid = coords.to3D(xb, yb, zb, B.x, B.y, LaserLine::CenterLaserLine);
            QVERIFY(valid);
            double tol = 1e-7;
            QVERIFY(isClose(double(xb-xa), expected_dx, tol));
            QVERIFY(isClose(double(yb-ya), expected_dy, tol));
            QVERIFY(isClose(double(zb-za), expected_dz3, tol));
        }
        
        
    
    }
    
    
    
    for (int line = 0; line < (int) LaserLine::NumberLaserLines; line ++)
    {
               
        int X = oCoaxData.m_oOrigX;
        int Y = oCoaxData.m_oOrigY;
        bool valid = coords.to3D(x,y,z, X, Y, LaserLine(line) );
        QVERIFY2(valid, "invalid coordinate at image origin");
        QCOMPARE(x,0.0);
        QCOMPARE(y,0.0);
        QCOMPARE(z,0.0);
        
        //measure an horizontal segment (in the image)
        {
            int XA = 0; int XB = 1000; int YA = 0; int YB = 0; 
            double dist_pix = dist(XA,YA,0,XB, YB, 0);
            
            float xa,ya,za,xb,yb,zb;
            valid = coords.to3D(xa, ya, za, XA, YA, LaserLine(line));
            QVERIFY(valid);
            valid = coords.to3D(xb, yb, zb, XB, YB, LaserLine(line));
            QVERIFY(valid);
            
            double dist_mm = dist(xa,ya,za,xb,yb,zb);
            double computed_factorHorizontal = coords.factorHorizontal();
            double expected_factorHorizontal = dist_pix / dist_mm;
            QVERIFY(computed_factorHorizontal > 0);
            QVERIFY(expected_factorHorizontal > 0);
            
            double comparison_tolerance = 5e-5;
            QVERIFY(isClose(expected_factorHorizontal, oCoaxData.m_oBeta0/ oCoaxData.m_oDpixX, comparison_tolerance));
            QVERIFY(isClose(computed_factorHorizontal, expected_factorHorizontal, comparison_tolerance));
            
            QVERIFY(isClose(coords.distanceOnHorizontalPlane(XA,YA,XB,YB), dist_mm, comparison_tolerance));
            QVERIFY(isClose(coords.pixel_to_mm_OnHorizontalPlane(100, XA,YA), double(dist_mm / dist_pix), comparison_tolerance));
                        
        }
        
        //measure a vertical segment (in the image)
        {
            int XA = 100; int XB = 100; int YA = 0; int YB = 1000; 
            double dist_pix = dist(XA,YA,0,XB, YB, 0);
            
            float xa,ya,za,xb,yb,zb;
            valid = coords.to3D(xa, ya, za, XA, YA, LaserLine(line));
            QVERIFY(valid);
            valid = coords.to3D(xb, yb, zb, XB, YB, LaserLine(line));
            QVERIFY(valid);
        
            double dist_mm = dist(xa,ya,za,xb,yb,zb);
            double computed_factorVertical = coords.factorVertical(100, XA,YA, LaserLine(line));
            double expected_factorVertical = dist_pix / dist_mm;
            QVERIFY( computed_factorVertical > 0);
            QVERIFY( expected_factorVertical > 0);

            
            double comparison_tolerance = 1e-5;
            QVERIFY(isClose(computed_factorVertical, expected_factorVertical, comparison_tolerance));

            double dist_mm_2d = dist(xa,ya,0,xb,yb,0);
            QCOMPARE(coords.distanceOnHorizontalPlane(XA,YA,XB,YB), dist_mm_2d);
            
            QVERIFY(isClose(coords.pixel_to_mm_OnHorizontalPlane(100, XA,YA), double(dist_mm_2d / dist_pix), comparison_tolerance)); // only in the coax model
            
        }
        
        // measure an oblique segment
        {
            int XA = 50; int XB = 1000; int YA = 50; int YB = 1000; 
            float DY = YB - YA;
            double dist_pix = dist(XA,YA,0,XB, YB, 0);
            
            float xa,ya,za,xb,yb,zb;
            valid = coords.to3D(xa, ya, za, XA, YA, LaserLine(line));
            QVERIFY(valid);
            valid = coords.to3D(xb, yb, zb, XB, YB, LaserLine(line));
            QVERIFY(valid);
                               
            double computed_factorVertical = coords.factorVertical(100, XA,YA, LaserLine(line));
            double expected_factorVertical = std::abs(DY) / dist(xa,0,za,xb,0,zb);
            QVERIFY( computed_factorVertical > 0);
            QVERIFY( expected_factorVertical > 0);

            
            double comparison_tolerance = 1e-5;
            QVERIFY(isClose(computed_factorVertical, expected_factorVertical, comparison_tolerance));

            double dist_mm_2d = dist(xa,ya,0,xb,yb,0);
            QCOMPARE(coords.distanceOnHorizontalPlane(XA,YA,XB,YB), dist_mm_2d);
            
            QVERIFY(isClose(coords.pixel_to_mm_OnHorizontalPlane(100, XA,YA), double(dist_mm_2d / dist_pix), comparison_tolerance)); // only in the coax model
            
        }
        // test inverse transform
        {
            precitec::geo2d::DPoint TCP_pix {587.6, 406.6};
            std::vector<precitec::geo2d::DPoint> contour_mm = { //{-1.0, -1.0}, {-0.02, 0.01},{0.0,0.0},{2.0,3.0},
            {2.3333333333333333333,1.3333333333333333}};
            auto contour_pix = coords.distanceTCPmmToSensorCoordCoax(contour_mm, TCP_pix.x, TCP_pix.y);
            
            
            precitec::geo2d::TPoint<float> TCP_mm;
            coords.convertScreenToHorizontalPlane(TCP_mm.x, TCP_mm.y, std::round(TCP_pix.x), std::round(TCP_pix.y));
            
            QCOMPARE(contour_pix.size(), contour_mm.size());
            for (unsigned int i=0; i < contour_mm.size(); i++ )
            {   
                const auto & rExpectedDist = contour_mm[i];
                const auto & rPoint_pix = contour_pix[i];
                
                precitec::geo2d::TPoint<float> rPoint_mm, rPoint_floor_mm, rPoint_ceil_mm;
                coords.convertScreenToHorizontalPlane(rPoint_mm.x, rPoint_mm.y, std::round(rPoint_pix.x), std::round(rPoint_pix.y));
                coords.convertScreenToHorizontalPlane(rPoint_floor_mm.x, rPoint_floor_mm.y, std::floor(rPoint_pix.x -0.5), std::floor(rPoint_pix.y -0.5));
                coords.convertScreenToHorizontalPlane(rPoint_ceil_mm.x, rPoint_ceil_mm.y, std::ceil(rPoint_pix.x + 0.5), std::ceil(rPoint_pix.y + 0.5));
                
                auto rActualDist = rPoint_mm - TCP_mm;
                auto rFloorDist = rPoint_floor_mm - TCP_mm;
                auto rCeilDist = rPoint_ceil_mm - TCP_mm;
                
                
                QVERIFY( std::abs(rActualDist.x - rExpectedDist.x)  <=  std::abs(rFloorDist.x - rExpectedDist.x));
                QVERIFY( std::abs(rActualDist.x - rExpectedDist.x)  <=  std::abs(rFloorDist.x - rExpectedDist.x));
                QVERIFY( std::abs(rActualDist.y - rExpectedDist.y)  <=  std::abs(rFloorDist.y - rExpectedDist.y));
                QVERIFY( std::abs(rActualDist.y - rExpectedDist.y)  <=  std::abs(rFloorDist.y - rExpectedDist.y));
                
                
            }
        }
    }

}

void TestCalibration3DCoords::testScheimpflug()
{
    std::string testdatafolder = QFINDTESTDATA("testdata/scheimpflug/").toStdString();
    precitec::system::CamGridData oCamGridData;
    std::string p_oCamGridImageFilename = testdatafolder + "/config/calibImgData0fallback.csv";
    std::string oMsgError = oCamGridData.loadFromCSV(p_oCamGridImageFilename);
    QVERIFY(oMsgError.empty());
        
    Calibration3DCoords coords;
    bool ok = loadCamGridData(coords, oCamGridData);
    QVERIFY(ok);
    QVERIFY(coords.isScheimpflugCase());
        
    Size oExpectedSensorSize {oCamGridData.sensorWidth(),oCamGridData.sensorHeight()};
    QCOMPARE(oExpectedSensorSize.width, 1024);
    QCOMPARE(oExpectedSensorSize.height, 1024);
    auto oSensorSize = coords.getSensorSize();
    int oWidth(-1), oHeight(-1);
    coords.getSensorSize(oWidth, oHeight);
    QCOMPARE(oSensorSize, oExpectedSensorSize);
    QCOMPARE(oWidth, oExpectedSensorSize.width);
    QCOMPARE(oHeight, oExpectedSensorSize.height);
    
    
    std::vector<Point> validCoordinates= {
        {0,0}, 
        {oExpectedSensorSize.width-1, 0},
        {0, oExpectedSensorSize.height -1},
        {oExpectedSensorSize.width-1,oExpectedSensorSize.height -1},
        {oCamGridData.originX(), oCamGridData.originY()}
    };
    
    for (auto & rPoint : validCoordinates)
    {            
        float x0,y0,z0;
        bool valid = coords.to3D(x0, y0, z0, rPoint.x, rPoint.y, LaserLine(0));
        QVERIFY(valid);
        
        for (int line = 1; line < (int) LaserLine::NumberLaserLines; line ++)
        {
            float x,y,z;
            bool valid = coords.to3D(x, y, z, rPoint.x, rPoint.y, LaserLine(line));
            QVERIFY(valid);
            QCOMPARE(x,x0);
            QCOMPARE(y,y0);
            QCOMPARE(z,z0);
        }        
    }
    
    
    std::vector<Point> invalidCoordinates = {
        {-1,-1},
        {oExpectedSensorSize.width, 0},
        {0, oExpectedSensorSize.height},
        {oExpectedSensorSize.width, oExpectedSensorSize.height}
    };
    
    
    for (auto & rPoint : invalidCoordinates)
    {            
        
        for (int line = 0; line < (int) LaserLine::NumberLaserLines; line ++)
        {
            float x,y,z;
            bool valid = coords.to3D(x, y, z, rPoint.x, rPoint.y, LaserLine(line));
            QVERIFY(!valid);
        }        
    }
    
     for (int line = 0; line < (int) LaserLine::NumberLaserLines; line ++)
    {
               
        float x,y,z;
        int X = oCamGridData.originX();
        int Y = oCamGridData.originY();
        bool valid = coords.to3D(x,y,z, X, Y, LaserLine(line) );
        QVERIFY2(valid, "invalid coordinate at image origin");
        QVERIFY(isClose<double>(x,0.0, 1e-4));        
                
        //measure a short vertical segment (in the image) - otherwise test distanceOnHorizontalPlane can not be approximated
        {
            int XA = 100; int XB = 100; int YA = 100; int YB = 105; 
            float DY = YB - YA;
            double dist_pix = dist(XA,YA,0,XB, YB, 0);
            
            float xa,ya,za,xb,yb,zb;
            valid = coords.to3D(xa, ya, za, XA, YA, LaserLine(line));
            QVERIFY(valid);
            valid = coords.to3D(xb, yb, zb, XB, YB, LaserLine(line));
            QVERIFY(valid);
        
            double dist_mm = dist(xa,ya,za,xb,yb,zb);
            double computed_factorVertical = coords.factorVertical(DY, (XA+XB)/2, (YA+YB)/2, LaserLine(line));
            double expected_factorVertical = dist_pix / dist_mm;
            QVERIFY( computed_factorVertical > 0);
            QVERIFY( expected_factorVertical > 0);

            
            double comparison_tolerance = 1e-5;
            QVERIFY(isClose(computed_factorVertical, expected_factorVertical, comparison_tolerance));

            //FIXME
            QEXPECT_FAIL("", "Distance On Horizontal plane (y coordinate) not well defined in scheimpflug case", Continue);
            double dist_mm_2d = dist(xa,ya,0,xb,yb,0);
            QCOMPARE(coords.distanceOnHorizontalPlane(XA,YA,XB,YB), dist_mm_2d); 
            
            QEXPECT_FAIL("", "Distance On Horizontal plane (y coordinate) not well defined in scheimpflug case", Continue);
            QVERIFY(isClose(coords.pixel_to_mm_OnHorizontalPlane(DY, (XA+XB)/2, (YA+YB)/2), double(dist_mm_2d / DY), comparison_tolerance)); 
            
        }
        
        
        // measure an oblique segment
        {
            int XA = 50; int XB = 60; int YA = 50; int YB = 60; 
            float DY = YB - YA;
            double dist_pix = dist(XA,YA,0,XB, YB, 0);
            
            float xa,ya,za,xb,yb,zb;
            valid = coords.to3D(xa, ya, za, XA, YA, LaserLine(line));
            QVERIFY(valid);
            valid = coords.to3D(xb, yb, zb, XB, YB, LaserLine(line));
            QVERIFY(valid);
                              
            double computed_factorVertical = coords.factorVertical(DY, (XA+XB)/2, (YA+YB)/2, LaserLine(line));
            double expected_factorVertical = std::abs(DY) / dist(xa,0,za,xb,0,zb);
            QVERIFY( computed_factorVertical > 0);
            QVERIFY( expected_factorVertical > 0);

            
            double comparison_tolerance_pix = 1; //pix/mm
            QVERIFY(isClose(computed_factorVertical, expected_factorVertical, comparison_tolerance_pix));

            
                        
            //FIXME
            double comparison_tolerance_mm= 1e-5;
            double dist_mm_2d = dist(xa,ya,0,xb,yb,0);
            QEXPECT_FAIL("", "Distance On Horizontal plane (y coordinate) not well defined in scheimpflug case", Continue);
            QCOMPARE(coords.distanceOnHorizontalPlane(XA,YA,XB,YB), dist_mm_2d);
            QEXPECT_FAIL("", "Distance On Horizontal plane (y coordinate) not well defined in scheimpflug case", Continue);
            QVERIFY(isClose(coords.pixel_to_mm_OnHorizontalPlane(dist_pix, (XA+XB)/2, (YA+YB)/2), double(dist_mm_2d / dist_pix), comparison_tolerance_mm)); // only in the coax model
            
        }
    }
    
}



QTEST_MAIN(TestCalibration3DCoords)
#include "testCalibration3DCoords.moc"
