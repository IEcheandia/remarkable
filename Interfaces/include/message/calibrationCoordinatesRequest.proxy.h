#ifndef CALIBRATIONCOORDINATESREQUEST_PROXY_H_
#define CALIBRATIONCOORDINATESREQUEST_PROXY_H_

#include  "message/calibrationCoordinatesRequest.interface.h"
#include  "server/proxy.h"

namespace precitec {
namespace interface {

	using namespace  system;

	template <>
	class TCalibrationCoordinatesRequest<MsgProxy> : public Server<MsgProxy>, public TCalibrationCoordinatesRequest<AbstractInterface>, public TCalibrationCoordinatesRequestMessageDefinition
	{
	public:

		TCalibrationCoordinatesRequest() : PROXY_CTOR(TCalibrationCoordinatesRequest), TCalibrationCoordinatesRequest<AbstractInterface>()
		{
		}
		TCalibrationCoordinatesRequest(SmpProtocolInfo &p) : PROXY_CTOR1(TCalibrationCoordinatesRequest, p), TCalibrationCoordinatesRequest<AbstractInterface>()
		{
		}
		virtual ~TCalibrationCoordinatesRequest() {}

	public:

		virtual Vec3D get3DCoordinates(ScreenCooordinate_t pX, ScreenCooordinate_t pY, SensorId p_oSensorID, LaserLine p_LaserLine)
		{
			INIT_MESSAGE(Get3DCoordinates);
			sender().marshal( pX );
			sender().marshal( pY );
			sender().marshal( p_oSensorID);
			sender().marshal( p_LaserLine);
			sender().send();
			Vec3D ret;
			sender().deMarshal(ret);
			return ret;
		}
		virtual geo2d::DPoint getCoordinatesFromGrayScaleImage(ScreenCooordinate_t pX, ScreenCooordinate_t pY, ScannerContextInfo scannerContext, SensorId p_oSensorID)
        {
            INIT_MESSAGE(GetCoordinatesFromGrayScaleImage);
			sender().marshal( pX );
			sender().marshal( pY );
            sender().marshal(scannerContext);
			sender().marshal( p_oSensorID);
			sender().send();
            geo2d::DPoint ret;
            sender().deMarshal(ret);
            return ret;            
        }

        virtual  geo2d::DPoint getTCPPosition(SensorId p_oSensorId, ScannerContextInfo scannerContext)
        {
            INIT_MESSAGE(GetTCPPosition);
            sender().marshal( p_oSensorId);
            sender().marshal(scannerContext);
            sender().send();
            geo2d::DPoint ret;
            sender().deMarshal(ret);
            return ret;
        }
		
        virtual bool availableOCTCoordinates(OCT_Mode mode)
        {
            INIT_MESSAGE(AvailableOCTCoordinates);
            sender().marshal(mode);
            sender().send();
            bool ret = false;
            sender().deMarshal(ret);
            return ret;
            
        }

                
        virtual geo2d::DPoint  getNewsonPosition(double xScreen, double yScreen, OCT_Mode mode )
        {
            INIT_MESSAGE(GetNewsonPosition);
            sender().marshal(xScreen);
            sender().marshal(yScreen);
            sender().marshal(mode);
            sender().send();
            geo2d::DPoint ret;
            sender().deMarshal(ret);
            return ret;
        }
        
        virtual geo2d::DPoint  getScreenPositionFromNewson(double xNewson, double yNewson, OCT_Mode mode )
        {
            INIT_MESSAGE(GetScreenPositionFromNewson);
            sender().marshal( xNewson );
            sender().marshal( yNewson );
            sender().marshal(mode);
            sender().send();
            geo2d::DPoint ret;
            sender().deMarshal(ret);
            return ret;
        }
        
        virtual geo2d::DPoint getScannerPositionFromScanFieldImage(double x_pixel, double y_pixel, Configuration scanfieldImageConfiguration)
        {
            INIT_MESSAGE(GetScannerPositionFromScanFieldImage);
            sender().marshal( x_pixel );
            sender().marshal( y_pixel );
            sender().marshal( scanfieldImageConfiguration );
            sender().send();
            geo2d::DPoint ret;
            sender().deMarshal(ret);
            return ret;
        }
        
        virtual geo2d::DPoint getScanFieldImagePositionFromScannerPosition(double x_mm, double y_mm, Configuration scanfieldImageConfiguration)
        {
            INIT_MESSAGE(GetScanFieldImagePositionFromScannerPosition);
            sender().marshal( x_mm );
            sender().marshal( y_mm );
            sender().marshal( scanfieldImageConfiguration );
            sender().send();
            geo2d::DPoint ret;
            sender().deMarshal(ret);
            return ret;
        }

	};

} // namespace interface
} // namespace precitec

#endif /* CALIBRATIONCOORDINATESREQUEST_PROXY_H_ */
