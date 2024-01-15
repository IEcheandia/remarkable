#include <QTest>
#include "../laserlineTracker1.h"

typedef precitec::interface::ImageFrame t_imageframe;

Q_DECLARE_METATYPE(t_imageframe)
Q_DECLARE_METATYPE(LaserlineTracker1ParametersT)


class LaserLineTrackerTest : public QObject
{
    Q_OBJECT
private:

private Q_SLOTS:
    void testCtor();
    void testProcess_data();
    void testProcess();
};

void LaserLineTrackerTest::testCtor()
{
    LaserlineTracker1T obj;
    QVERIFY(!obj.result.bIsValid);
}


void LaserLineTrackerTest::testProcess_data()
{
    QTest::addColumn<LaserlineTracker1ParametersT> ("parameters");
    QTest::addColumn<precitec::interface::ImageFrame> ("frame");
    QTest::addColumn<int> ("processResult");
    QTest::addColumn<int> ("firstValidIndex");
    QTest::addColumn<int> ("lastValidIndex");

    byte background_intensity = 100;
    byte laser_intensity = 180;    
    
    Size2d HWROI_Size(512,512);
    
    precitec::image::BImage oSensorImage(HWROI_Size);
    oSensorImage.fill(background_intensity); //fill must be called here, it crashes in a subroi
    
    int roi_w = 300;
    int roi_h =50;
    //simulate roi filter in the middle of the image
    geo2d::Point oRoiStart ((HWROI_Size.width-roi_w)/2, (HWROI_Size.height-roi_h)/2);
    geo2d::Rect oInRoi(oRoiStart, Size2d(roi_w, roi_h));
    
    auto  fDefaultParameters = [] () {
                LaserlineTracker1T obj; //constructor assign default values
                
                obj.m_par.iTrackStart = 0; //von links
                obj.m_par.startAreaX = 1;
                obj.m_par.startAreaY=1;
                obj.m_par.doubleTracking = false;
                obj.m_par.iSuchSchwelle = 101;
                obj.m_par.iMittelungX = 1;
                obj.m_par.iMittelungY = 1;
                return obj.m_par;
            };
    {          
        
            
        //create a  laser line with a slope 
        //              ____
        //        ___/
        
        int laser_start_x = 10;
        int laser_start_y = 10;
        int laser_end_x = 290;
        int laser_end_y = 20;
        int laser_half_thickness = 1; //actual thickness for an horizontal laser line: 1 + 2 * laser_half_thickness
        
        QVERIFY(roi_w <= HWROI_Size.width && roi_h <= HWROI_Size.height);
        
        
        precitec::interface::ImageFrame oFrame;
        oFrame.data() = precitec::image::BImage(oSensorImage, oInRoi, true);

        auto & rImage = oFrame.data();
        
        QCOMPARE(rImage.width(),roi_w);
        QCOMPARE(rImage.height(), roi_h);

        //verify the test parameters
        QVERIFY(laser_start_x < roi_w);
        QVERIFY(laser_end_x < roi_w);
        QVERIFY(laser_start_y  + laser_half_thickness  < roi_h);
        QVERIFY(laser_end_y  + laser_half_thickness  < roi_h);
        QVERIFY(laser_start_y  - laser_half_thickness >= 0);
        QVERIFY(laser_end_y - laser_half_thickness >= 0);
        
        int x1 = laser_end_x / 3;
        int x2 = std::min(x1+5, laser_end_x);
        double delta_y = double(laser_end_y - laser_start_y)/double(x2-x1);
        
        
        for (int yThickness = - laser_half_thickness; yThickness <= laser_half_thickness; yThickness++)
        {
            
            for (int x = laser_start_x; x <x1; ++x)
            {
                rImage[laser_start_y + yThickness][x]= laser_intensity;
            }
            
            double y = laser_start_y + yThickness;
            for (int x = x1; x < x2; ++x)
            {
                rImage[int(y)][x]= laser_intensity;
                y += delta_y;
            }
            
            for (int x = x2 ; x <=laser_end_x; ++x)
            {
                rImage[laser_end_y + yThickness][x]= laser_intensity;
            }
        }
        
        QCOMPARE(rImage[laser_start_y][laser_start_x], laser_intensity);
        QCOMPARE(rImage[laser_end_y][laser_end_x], laser_intensity);    
        QCOMPARE(rImage[laser_end_y][laser_end_x+1 ], background_intensity);    
        
        oFrame.context().setImageNumber(5);
        oFrame.context().setPosition(300);
        oFrame.context().HW_ROI_x0 = 40;
        oFrame.context().HW_ROI_y0 = 50;
        
        //expected results from the original implementation

        QTest::newRow("laserLinePresent simple tracking") << fDefaultParameters() << oFrame << 0 << laser_start_x << laser_end_x+1;

        auto par2 = fDefaultParameters();
        par2.doubleTracking = true;
        QTest::newRow("laserLinePresent double tracking") << par2 << oFrame << 0 << laser_start_x -1 << laser_end_x+1;

        auto par3 = fDefaultParameters();
        par3.iTrackStart = 1; //from right
        par3.startAreaX = 5;
        par3.startAreaY = 10;
        QTest::newRow("laserLinePresent from right") << par3 << oFrame << 0 << 5 << 294;

        auto par4 = fDefaultParameters();
        par4.iTrackStart = 2; //automatic
        par4.iMittelungX = 5;
        par4.iMittelungY = 2;
        QTest::newRow("laserLinePresent automatic") << par4 << oFrame << 1 << 101 << 290;

        auto par5 = fDefaultParameters();
        par5.iTrackStart = 0; //from left
        par5.lapJoin = true;
        par5.upperMaxLineDiff = 8;
        par5.lowerMaxLineDiff = 7;
        QTest::newRow("laserLinePresent from left with dist") << par5 << oFrame << 1 << 10 << 99;

        par5.iTrackStart = 1; //from right
        QTest::newRow("laserLinePresent from right with dist") << par5 << oFrame << 1 << 97 << 290;

        par5.iTrackStart = 0; //from left
        par5.upperMaxLineDiff = 10;
        par5.lowerMaxLineDiff = 10;
        QTest::newRow("laserLinePresent from left with dist") << par5 << oFrame << 0 << 10 << 291;

        par5.iTrackStart = 1; //from right
        QTest::newRow("laserLinePresent from right with dist") << par5 << oFrame << 0 << 9 << 290;

        auto par6 = fDefaultParameters();
        par6.iPixelY = 1;
        par6.iPixelYLower = 5;
        par6.iTrackStart = 0; // from left
        QTest::newRow("laserLinePresent from left with different pixelY and pixelYLower") << par6 << oFrame << 0 << laser_start_x << laser_end_x+1;

        par6.iTrackStart = 1; // from right
        QTest::newRow("laserLinePresent from right with different pixelY and pixelYLower") << par6 << oFrame << 1 << x2 << laser_end_x;
    }

    {
        precitec::interface::ImageFrame oFrame2;
        oFrame2.data() = precitec::image::BImage(oSensorImage,  geo2d::Rect(oRoiStart, Size2d(roi_w+1, 1+1)));

        //expected results from the original implementation
        QTest::newRow("ROI too short") << fDefaultParameters()  << oFrame2 << -1 << -1 << -1;
        auto par2 = fDefaultParameters();
        par2.doubleTracking = true;
        QTest::newRow("ROI too short doubleTracking") << par2 << oFrame2 << -1 << -1 << -1;
    }
 
    {  
        auto par = fDefaultParameters();
        par.startAreaX = 5;
        precitec::interface::ImageFrame oFrame;
        oFrame.data() = precitec::image::BImage(oSensorImage,  geo2d::Rect(oRoiStart, Size2d(par.startAreaX, roi_h+1 )));
                
        QTest::newRow("ROI equal to parStartX") << par  << oFrame << -1 << -1 << -1;
        par.doubleTracking = true;
        QTest::newRow("ROI equal to parStartX doubleTracking") << par << oFrame << -1 << -1 << -1;
    }
    
    {  
        auto par = fDefaultParameters();
        par.startAreaY = 5;
        precitec::interface::ImageFrame oFrame;
        oFrame.data() = precitec::image::BImage(oSensorImage,  geo2d::Rect(oRoiStart, Size2d(roi_w+1, par.startAreaY )));
                
        QTest::newRow("ROI equal to parStartY") << par  << oFrame << -1 << -1 << -1;
        par.doubleTracking = true;
        QTest::newRow("ROI equal to parStartY doubleTracking") << par << oFrame << -1 << -1 << -1;
    }
    
    {    
        precitec::interface::ImageFrame oFrame3;
        oFrame3.data() = precitec::image::BImage(oSensorImage,  geo2d::Rect(oRoiStart, Size2d(1+1, roi_h+1)));
        
        //expected results from the original implementation
        
        QTest::newRow("ROI too narrow simpleTracking") << fDefaultParameters()  << oFrame3 << -1 << -1 << -1;  //values after bugfix narrow roi
        
        auto par2 = fDefaultParameters();
        par2.doubleTracking = true;
        QTest::newRow("ROI too narrow doubleTracking") << par2 << oFrame3 << -1 << -1 << -1;
    }
    
    {
        // new image (like the output of imageArithmetic) with a thick horizontal line from (0,16) to (24,16)
        precitec::interface::ImageFrame oFrame4;
        oFrame4.data().resizeFill(Size2d(40,30), background_intensity);
        auto & rImage4 = oFrame4.data();
        
        int laser_start_y = 16;
        int laser_start_x = 5;
        int laser_end_x = 22;
        auto laser_length = laser_end_x - laser_start_x + 1;
        QVERIFY(laser_start_y >= 10); // see IMinBorder in LaserlineTracker1T::searchLinesUpperLower

    
        memset(rImage4.rowBegin(laser_start_y-1) + laser_start_x, laser_intensity,laser_length);
        memset(rImage4.rowBegin(laser_start_y) + laser_start_x, laser_intensity, laser_length);
        memset(rImage4.rowBegin(laser_start_y+1) + laser_start_x, laser_intensity,  laser_length);
        QCOMPARE(rImage4[laser_start_y][laser_start_x],laser_intensity);
        QCOMPARE(rImage4[laser_start_y][laser_start_x-1],background_intensity);
        QCOMPARE(rImage4[laser_start_y][laser_end_x],laser_intensity);
        QCOMPARE(rImage4[laser_start_y][laser_end_x+1],background_intensity);
        
        //expected results from the original implementation

        QTest::newRow("FilteredImage simple Tracking") << fDefaultParameters()  << oFrame4 << 0 << laser_start_x  << laser_end_x+1;
        auto par2 = fDefaultParameters();
        
        par2.doubleTracking = true;
        QTest::newRow("FilteredImage doubleTracking") << par2 << oFrame4 << 0 << laser_start_x -1 << laser_end_x+1;
                
        auto par3 = fDefaultParameters();
        par3.iTrackStart = 1; //from right
        par3.startAreaX = 5;
        par3.startAreaY = 10;
        QTest::newRow("FilteredImage from right") << par3 << oFrame4 << 0 << 0 << 26;
        
        auto par4 = fDefaultParameters();
        par4.iTrackStart = 2; //automatic
        par4.iMittelungX = 5;
        par4.iMittelungY = 2;
        QTest::newRow("FilteredImage automatic") << par4 << oFrame4 << 0 << 0 << 22;
    }    
    

}


void LaserLineTrackerTest::testProcess()
{
    QFETCH(LaserlineTracker1ParametersT, parameters);
    QFETCH(precitec::interface::ImageFrame, frame);
    
    LaserlineTracker1T obj;
    obj.m_par = parameters;
    
    obj.alloc(20, 20); //the correct size is actually not necessary, the check is inside laserlineTracker1::process
    int ret = obj.process(&frame);
    
//    QVERIFY( !obj.result.bIsValid || ( obj.result.firstValidIndex >= 0  && obj.result.lastValidIndex >= 0));

    QTEST(obj.result.firstValidIndex, "firstValidIndex");
    QTEST(obj.result.lastValidIndex, "lastValidIndex");
    
    //repeat processing
    precitec::interface::ImageFrame blackFrame;
    blackFrame.data().resizeFill(Size2d(512,512), 0);
    ret = obj.process(&blackFrame);
    QCOMPARE(ret, -1);
    QCOMPARE(obj.result.firstValidIndex, -1);
    QCOMPARE(obj.result.lastValidIndex, -1);
    
    ret = obj.process(&frame);

    QTEST(ret, "processResult");
    QTEST(obj.result.firstValidIndex, "firstValidIndex");
    QTEST(obj.result.lastValidIndex, "lastValidIndex");
        
}


QTEST_GUILESS_MAIN(LaserLineTrackerTest)
#include "LaserlineTrackerTest.moc"
