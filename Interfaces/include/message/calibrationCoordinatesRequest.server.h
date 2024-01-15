#ifndef CALIBRATIONCOORDINATESREQUEST_SERVER_H_
#define CALIBRATIONCOORDINATESREQUEST_SERVER_H_

#include  "message/calibrationCoordinatesRequest.interface.h"

namespace precitec {
namespace interface {

	template <>
	class TCalibrationCoordinatesRequest<MsgServer> : public TCalibrationCoordinatesRequest<AbstractInterface>
	{
	public:
		TCalibrationCoordinatesRequest(){}
		virtual ~TCalibrationCoordinatesRequest(){}

		virtual Vec3D get3DCoordinates(ScreenCooordinate_t pX, ScreenCooordinate_t  pY,  SensorId p_oSensorID, LaserLine p_LaserLine)
		{
			std::cout << "DEBUG: TCalibrationCoordinatesRequest<MsgServer>::get3DCoordinates( " << ") called!!" << std::endl;
			return Vec3D(-1,-1,-1);
		}
		
        virtual geo2d::DPoint getCoordinatesFromGrayScaleImage(ScreenCooordinate_t pX, ScreenCooordinate_t pY, ScannerContextInfo scannerContext, SensorId p_oSensorID)
        {
            
			std::cout << "DEBUG: TCalibrationCoordinatesRequest<MsgServer>::getCoordinatesFromGrayScaleImage( " << ") called!!" << std::endl;
			return {-1000.0, -1000.0};
        }

        virtual geo2d::DPoint getTCPPosition(SensorId p_oSensorId, ScannerContextInfo scannerContext)
        {
            std::cout << "DEBUG: TCalibrationCoordinatesRequest<MsgServer>::getTCPPosition( " << ") called!!" << std::endl;
            return {0.0, 0.0};
        }
		
        virtual bool availableOCTCoordinates(OCT_Mode mode)
        {
            std::cout << "DEBUG: TCalibrationCoordinatesRequest<MsgServer>::availableOCTCoordinates called " << std::endl;
            return false;
        }

		
        virtual geo2d::DPoint  getNewsonPosition(double xScreen, double yScreen, OCT_Mode mode)
        {
            std::cout << "DEBUG: TCalibrationCoordinatesRequest<MsgServer>::getNewsonPosition called " << std::endl;
            return {0,0};
        } 
        
        virtual geo2d::DPoint  getScreenPositionFromNewson(double xNewson, double yNewson, OCT_Mode mode)
        {
            std::cout << "DEBUG: TCalibrationCoordinatesRequest<MsgServer>::getScreenPositionFromNewson called " << std::endl;
            return {0,0};
        }
        
        virtual geo2d::DPoint getScannerPositionFromScanFieldImage(double x_pixel, double y_pixel, Configuration scanfieldImageConfiguration)
        {
            std::cout << "DEBUG: TCalibrationCoordinatesRequest<MsgServer>::getScannerPositionFromScanFieldImage called " << std::endl;
            return {0,0};
        }
        
        
        virtual geo2d::DPoint getScanFieldImagePositionFromScannerPosition(double x_mm, double y_mm, Configuration scanfieldImageConfiguration)
        {
            std::cout << "DEBUG: TCalibrationCoordinatesRequest<MsgServer>::getScanFieldImagePositionFromScannerPosition called " << std::endl;
            return {0,0};
        }


	};

} // namespace interface
} // namespace precitec

#endif /* CALIBRATIONCOORDINATESREQUEST_SERVER_H_ */
