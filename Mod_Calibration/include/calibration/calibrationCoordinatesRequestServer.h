#ifndef CALIBRATIONCOORDINATESREQUESTSERVER_H_
#define CALIBRATIONCOORDINATESREQUESTSERVER_H_

#include  "message/calibrationCoordinatesRequest.server.h"
#include "calibration/calibrationManager.h"

namespace precitec {
namespace calibration {
    
class CalibrationManager;

class CalibrationCoordinatesRequestServer : public TCalibrationCoordinatesRequest<MsgServer>
{
public:
    explicit CalibrationCoordinatesRequestServer( CalibrationManager* p_pCalibrationManager )
        :m_pCalibrationManager( p_pCalibrationManager )
        {};
    virtual ~CalibrationCoordinatesRequestServer() override {}

    virtual Vec3D get3DCoordinates(ScreenCooordinate_t pX, ScreenCooordinate_t pY, SensorId p_oSensorID, LaserLine p_LaserLine)
    {
        assert(m_pCalibrationManager);
        Vec3D result;
        bool ok = m_pCalibrationManager->get3DCoordinates(result,pX, pY, p_oSensorID, p_LaserLine);
        if (!ok)
        {
            wmLog(eWarning, "Invalid coordinate requested %d %d laserline %s \n", pX, pY, laserLineName(p_LaserLine).c_str());
            result = Vec3D({-1000, -1000, -1000}); //invalid values checked by ImageMeasuringDialog
        }
        return result;
    }
    
    
    virtual geo2d::DPoint getCoordinatesFromGrayScaleImage(ScreenCooordinate_t pX, ScreenCooordinate_t pY, ScannerContextInfo scannerContext, SensorId p_oSensorID)
    {
        assert(m_pCalibrationManager);
        return m_pCalibrationManager->getCoordinatesFromGrayScaleImage(pX, pY, scannerContext, p_oSensorID);
    }


    virtual geo2d::DPoint getTCPPosition(SensorId p_oSensorId, ScannerContextInfo scannerContext)
    {
        assert(m_pCalibrationManager);
        return m_pCalibrationManager->getTCPPosition(p_oSensorId, scannerContext, LaserLine::FrontLaserLine); //FIXME
    }

    
    virtual bool availableOCTCoordinates(OCT_Mode mode)
    {
        assert(m_pCalibrationManager);
        return m_pCalibrationManager->availableOCTCoordinates(mode);
    }
    
    
    virtual geo2d::DPoint getNewsonPosition(double xScreen, double yScreen, OCT_Mode mode)
    {
        assert(m_pCalibrationManager);
        return m_pCalibrationManager->getNewsonPosition(xScreen, yScreen, mode);
    }
    
    virtual geo2d::DPoint getScreenPositionFromNewson(double xNewson, double yNewson, OCT_Mode mode)
    {
        assert(m_pCalibrationManager);
        return m_pCalibrationManager->getScreenPositionFromNewson( xNewson, yNewson, mode);
    }

    virtual geo2d::DPoint getScannerPositionFromScanFieldImage(double x_pixel, double y_pixel, Configuration scanfieldImageConfiguration)
    {
        assert(m_pCalibrationManager);
        return m_pCalibrationManager->getScannerPositionFromScanFieldImage( x_pixel, y_pixel,  scanfieldImageConfiguration);
    }

    virtual geo2d::DPoint getScanFieldImagePositionFromScannerPosition(double x_mm, double y_mm, Configuration scanfieldImageConfiguration)
    {
        assert(m_pCalibrationManager);
        return m_pCalibrationManager->getScanFieldImagePositionFromScannerPosition( x_mm, y_mm,  scanfieldImageConfiguration);
    }

protected:

    CalibrationManager *m_pCalibrationManager;	///< Pointer to calibration manager that is handling the actual calibration mechanism.

};


} // namespace interface
} // namespace precitec

#endif /* CALIBRATIONCOORDINATESREQUEST_SERVER_H_ */
