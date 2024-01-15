#ifndef CALIBRATIONCOORDINATESREQUEST_HANDLER_H_
#define CALIBRATIONCOORDINATESREQUEST_HANDLER_H_

#include  "server/handler.h"
#include  "message/calibrationCoordinatesRequest.interface.h"

namespace precitec {

using namespace  system;
using namespace  message;

namespace interface {

	template <>
	class TCalibrationCoordinatesRequest<MsgHandler> : public Server<MsgHandler>, public TCalibrationCoordinatesRequestMessageDefinition
	{
	public:

		MSG_HANDLER(TCalibrationCoordinatesRequest);
        
	public:

		void registerCallbacks()
		{            
			REGISTER_MESSAGE(Get3DCoordinates, get3DCoordinates);
            REGISTER_MESSAGE(GetCoordinatesFromGrayScaleImage, getCoordinatesFromGrayScaleImage);
            REGISTER_MESSAGE(GetTCPPosition, getTCPPosition);
			REGISTER_MESSAGE(AvailableOCTCoordinates, availableOCTCoordinates);
            REGISTER_MESSAGE(GetNewsonPosition, getNewsonPosition);
			REGISTER_MESSAGE(GetScreenPositionFromNewson, getScreenPositionFromNewson);
			REGISTER_MESSAGE(GetScannerPositionFromScanFieldImage, getScannerPositionFromScanFieldImage);
			REGISTER_MESSAGE(GetScanFieldImagePositionFromScannerPosition, getScanFieldImagePositionFromScannerPosition);
		}

		void get3DCoordinates(Receiver &receiver)
		{
			ScreenCooordinate_t pX; receiver.deMarshal( pX );
			ScreenCooordinate_t pY; receiver.deMarshal( pY);
			SensorId p_oSensorId; receiver.deMarshal(p_oSensorId);
			LaserLine p_oLaserLine; receiver.deMarshal(p_oLaserLine);
			Vec3D ret = server_->get3DCoordinates( pX, pY, p_oSensorId, p_oLaserLine );
			receiver.marshal(ret);
			receiver.reply();
		}
		
		void getCoordinatesFromGrayScaleImage(Receiver & receiver)
        {
            ScreenCooordinate_t xScreen; receiver.deMarshal(xScreen);
            ScreenCooordinate_t yScreen; receiver.deMarshal(yScreen);
            ScannerContextInfo p_oScannerContext;
            receiver.deMarshal(p_oScannerContext);
			SensorId p_oSensorId; receiver.deMarshal(p_oSensorId);
            geo2d::DPoint ret = server_->getCoordinatesFromGrayScaleImage(xScreen, yScreen, p_oScannerContext, p_oSensorId);
            receiver.marshal(ret);
            receiver.reply();
        }

        void getTCPPosition(Receiver & receiver)
        {
            SensorId p_oSensorId; receiver.deMarshal(p_oSensorId);
            ScannerContextInfo p_oScannerContext; receiver.deMarshal(p_oScannerContext);
            geo2d::DPoint ret = server_->getTCPPosition(p_oSensorId, p_oScannerContext);
            receiver.marshal(ret);
            receiver.reply();
        }
		
        void availableOCTCoordinates(Receiver &receiver)
        {
            OCT_Mode mode; receiver.deMarshal(mode);
            bool ret = server_->availableOCTCoordinates(mode);
            receiver.marshal(ret);
            receiver.reply();
        }
		
        void  getNewsonPosition(Receiver &receiver)
        {
            double xScreen; receiver.deMarshal(xScreen);
            double yScreen; receiver.deMarshal(yScreen);
            OCT_Mode mode; receiver.deMarshal(mode);
            geo2d::DPoint ret = server_->getNewsonPosition(xScreen, yScreen, mode);
            receiver.marshal(ret);
            receiver.reply();
        }
        
        void  getScreenPositionFromNewson(Receiver &receiver)
        {
            double xNewson; receiver.deMarshal(xNewson);
            double yNewson; receiver.deMarshal(yNewson);
            OCT_Mode mode; receiver.deMarshal(mode);
            geo2d::DPoint ret = server_->getScreenPositionFromNewson(xNewson, yNewson, mode);
            receiver.marshal(ret);
            receiver.reply();
        }
        
        void getScannerPositionFromScanFieldImage(Receiver &receiver)
        {
            double x_pixel; receiver.deMarshal(x_pixel);
            double y_pixel; receiver.deMarshal(y_pixel);
            Configuration scanfieldImageConfiguration; receiver.deMarshal(scanfieldImageConfiguration);
            geo2d::DPoint ret = server_->getScannerPositionFromScanFieldImage(x_pixel, y_pixel,  scanfieldImageConfiguration);
            receiver.marshal(ret);
            receiver.reply();
        }
		
		
        void getScanFieldImagePositionFromScannerPosition(Receiver &receiver)
        {
            double x_mm; receiver.deMarshal(x_mm);
            double y_mm; receiver.deMarshal(y_mm);
            Configuration scanfieldImageConfiguration; receiver.deMarshal(scanfieldImageConfiguration);
            geo2d::DPoint ret = server_->getScanFieldImagePositionFromScannerPosition(x_mm, y_mm, scanfieldImageConfiguration);
            receiver.marshal(ret);
            receiver.reply();
        }
		
	};

} // namespace interface
} // namespace precitec

#endif /* CALIBRATIONCOORDINATESREQUEST_HANDLER_H_ */
