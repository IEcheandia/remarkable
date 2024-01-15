#include <QTest>

#include "../include/calibration/calibrationManager.h"
#include "../include/calibration/CalibrationOCTData.h"
#include "../include/calibration/CalibrationOCTConfigurationIDM.h"
#include "../include/calibration/CalibrationOCTLineParameters.h"

using namespace precitec::math;
using namespace precitec::geo2d;
using precitec::filter::LaserLine;

using precitec::calibration::CalibrationOCTData;
using precitec::calibration::CalibrationOCTMeasurementModel;
using precitec::calibration::CalibrationOCTConfigurationIDM;


// dummy implementation of CalibrationManager
namespace precitec
{
namespace calibration
{

    interface::SmpKeyValue CalibrationManager::getIDMKeyValue(precitec::interface::Key p_key, int p_subDevice) 
    { 
        return precitec::interface::SmpKeyValue{};
    }
    interface::KeyHandle CalibrationManager::setIDMKeyValue ( interface::SmpKeyValue p_keyValue, int p_subDevice ) 
    {
        return interface::KeyHandle{};
    }

    math::CalibrationData& CalibrationManager::getCalibrationData(unsigned int p_oSensorId)
    {
        return m_oCalData[math::SensorId(p_oSensorId)];
    } 
    void CalibrationManager::sendCalibDataChangedSignal(int p_oSensorID, bool p_oInit)
    {

    }

}
}


class testOCTCoordinates : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCoaxEquivalence();
    void testGapMeasurement();
private: 
    DPoint getXZ(Point p_point, const CalibrationOCTData &rCalibOCTData);
    DPoint getXZ(Point p_point, const CalibrationData &rCalibCoaxData);
};

DPoint testOCTCoordinates::getXZ(Point p_point, const CalibrationOCTData &rCalibOCTData)
{
    auto x_um = rCalibOCTData.m_oOCTMeasurementModel.getLateralDistanceFromScanLineCenter(
                p_point.x, rCalibOCTData.m_oCurrentConfiguration);
    auto z_um = rCalibOCTData.m_oOCTMeasurementModel.getDepth(p_point.y, rCalibOCTData.m_oCurrentConfiguration);
    return DPoint(x_um/1000.0, z_um/1000.0);
}

DPoint testOCTCoordinates::getXZ(Point p_point, const CalibrationData &rCalibCoaxData)
{
    float x_mm, y_mm, z_mm;
    rCalibCoaxData.getCalibrationCoords().to3D(x_mm, y_mm, z_mm, p_point.x, p_point.y, LaserLine::FrontLaserLine);
    return DPoint(x_mm, z_mm);
    
}

void testOCTCoordinates::testCoaxEquivalence()
{
    QTemporaryDir configDir;
    qputenv("WM_BASE_DIR", configDir.path().toLocal8Bit());

    //create a default coax calibration
    auto oCalibData = CalibrationData(SensorId::eSensorId0);
    oCalibData.initConfig(SensorModel::eLinearMagnification,false);
    oCalibData.load3DFieldFromParameters();
    
    const auto oInitialCoaxParameters = oCalibData.getCoaxCalibrationData();
    
    //CalibrationOCTData with default values
    CalibrationOCTData calibOCTData;
    calibOCTData.load(configDir.path().toStdString());
    calibOCTData.updateCurrentConfiguration(CalibrationOCTConfigurationIDM{});
    
    
    calibOCTData.computeEquivalentCoaxCalibrationData(oCalibData);    
    QVERIFY(oCalibData.checkCalibrationValuesConsistency());
    
    QVERIFY(oCalibData.getCoaxCalibrationData().m_oBeta0 != oInitialCoaxParameters.m_oBeta0);
    QVERIFY(oCalibData.getCoaxCalibrationData().m_oBetaZ != oInitialCoaxParameters.m_oBetaZ);
    

    Point origin_pix(350,0);    
    DPoint origin_OCT_mm = getXZ(origin_pix, calibOCTData);
    DPoint origin_Coax_mm = getXZ(origin_pix, oCalibData);

    double tolerance = 1e-6;

    for (auto & pix :  {Point(200, 50), Point (600, 200), Point(350,0) })
    {
        DPoint oct_mm = getXZ(pix, calibOCTData) - origin_OCT_mm;
        DPoint coax_mm = getXZ(pix, oCalibData) - origin_Coax_mm;
        //std::cout << pix - origin_pix << ") \t" << oct_mm << " " << coax_mm<< " diff: " << oct_mm - coax_mm<< "\n";
        QVERIFY(precitec::math::isClose( oct_mm.x, coax_mm.x, tolerance));
        QVERIFY(precitec::math::isClose( oct_mm.y, coax_mm.y, tolerance));
    }
    
    
    
}
 
void testOCTCoordinates::testGapMeasurement()
{
    using namespace precitec::calibration;
    
   CalibrationOCTConfigurationIDM oIDMConfiguration{};
   
    //plausible values at the default configuration 
    
    int XGap = 60;    //pixel from eIDMTrackingLine 
    int YGap = 100;  //pixel from eIDMTrackingLine 
    double GapWidth_um = 1000;
    double GapDepth_um = 1000;
    
    double lateralResolution = GapWidth_um/double(XGap); // [um] between each measurement 
    double depthResolution = GapWidth_um/ (double(YGap)*oIDMConfiguration.get(eRescaleIDMValue)); // [um]/[IDM value]
   
    CalibrationOCTMeasurementModel oModel {oIDMConfiguration, {0, 0}, 700, lateralResolution, depthResolution};  // resolution: um / pixel

    //simple case: the current IDM configuration is the same as the model
    CalibrationOCTData calibOCTData(oModel, oIDMConfiguration);

    DPoint OCT_mm = getXZ(Point(XGap, YGap), calibOCTData);

    QCOMPARE(OCT_mm.x*1000, GapWidth_um);
    QCOMPARE(OCT_mm.y*1000, GapDepth_um);

    
     //create a default coax calibration
    auto oCalibData = CalibrationData(SensorId::eSensorId0);
    oCalibData.initConfig(SensorModel::eLinearMagnification,false);
    
    calibOCTData.computeEquivalentCoaxCalibrationData(oCalibData);    
    QVERIFY(oCalibData.checkCalibrationValuesConsistency(1e-5, precitec::eInfo, true));
    
    Point pix (XGap, YGap);
    Point origin (0,0);
    
    DPoint oct_mm = getXZ(pix, calibOCTData) -getXZ(origin, calibOCTData) ;
    DPoint coax_mm = getXZ(pix, oCalibData) - getXZ(origin, oCalibData) ;
    double tolerance = 1e-6;
    
    //std::cout << pix  << ") \t" << oct_mm << " " << coax_mm<< " diff: " << oct_mm - coax_mm<< "\n";
    QVERIFY(precitec::math::isClose( oct_mm.x, coax_mm.x, tolerance));
    QVERIFY(precitec::math::isClose( oct_mm.y, coax_mm.y, tolerance));


    
}

 
 

QTEST_GUILESS_MAIN(testOCTCoordinates)

#include "testOCTCoordinates.moc"
