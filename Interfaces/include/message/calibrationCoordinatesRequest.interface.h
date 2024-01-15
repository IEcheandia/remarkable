#ifndef CALIBRATIONCOORDINATESREQUEST_INTERFACE_H_
#define CALIBRATIONCOORDINATESREQUEST_INTERFACE_H_

#include  "server/interface.h"
#include  "module/interfaces.h"
#include  "protocol/protocol.info.h"
#include  "message/calibrationCoordinatesRequest.h"

namespace precitec {
namespace interface {

	template <int CallType>
	class TCalibrationCoordinatesRequest;

	template <>
	class TCalibrationCoordinatesRequest<AbstractInterface>
	{
	public:
		TCalibrationCoordinatesRequest() {}
		virtual ~TCalibrationCoordinatesRequest() {}
		virtual Vec3D get3DCoordinates(ScreenCooordinate_t pX, ScreenCooordinate_t pY, SensorId p_oSensorID, LaserLine p_LaserLine) = 0; 
        virtual geo2d::DPoint getCoordinatesFromGrayScaleImage(ScreenCooordinate_t pX, ScreenCooordinate_t pY, ScannerContextInfo scannerContext, SensorId p_oSensorID) = 0;
        virtual geo2d::DPoint getTCPPosition(SensorId p_oSensorId, ScannerContextInfo scannerContext) = 0;
        virtual bool availableOCTCoordinates(OCT_Mode mode) = 0;
		virtual geo2d::DPoint getNewsonPosition(double xScreen, double yScreen, OCT_Mode mode ) = 0; 
        virtual geo2d::DPoint getScreenPositionFromNewson(double xNewson, double yNewson, OCT_Mode mode ) = 0; 
        
        // scanfield image -> scanner coordinates
        virtual geo2d::DPoint getScannerPositionFromScanFieldImage(double x_pixel, double y_pixel,  Configuration scanfieldImageConfiguration) = 0;
        //scanner coordinates -> scanfield image
        virtual geo2d::DPoint getScanFieldImagePositionFromScannerPosition(double x_mm, double y_mm,  Configuration scanfieldImageConfiguration) = 0;

	};

    struct TCalibrationCoordinatesRequestMessageDefinition
    {
        MESSAGE(Vec3D, Get3DCoordinates, ScreenCooordinate_t, ScreenCooordinate_t, SensorId, LaserLine);
        MESSAGE(geo2d::DPoint, GetCoordinatesFromGrayScaleImage, ScreenCooordinate_t, ScreenCooordinate_t, ScannerContextInfo, SensorId);
        MESSAGE(geo2d::DPoint, GetTCPPosition, SensorId, ScannerContextInfo);
        MESSAGE(bool, AvailableOCTCoordinates, OCT_Mode);
        MESSAGE(geo2d::DPoint, GetNewsonPosition, double, double, OCT_Mode);
        MESSAGE(geo2d::DPoint, GetScreenPositionFromNewson, double, double, OCT_Mode);
        MESSAGE(geo2d::DPoint, GetScannerPositionFromScanFieldImage,  double,  double, Configuration);
        MESSAGE(geo2d::DPoint, GetScanFieldImagePositionFromScannerPosition,  double,  double, Configuration);
        MESSAGE_LIST(
            Get3DCoordinates,
            GetCoordinatesFromGrayScaleImage,
            GetTCPPosition,
            AvailableOCTCoordinates,
            GetNewsonPosition,
            GetScreenPositionFromNewson, 
            GetScannerPositionFromScanFieldImage, 
            GetScanFieldImagePositionFromScannerPosition
        );
    };

	template <>
	class TCalibrationCoordinatesRequest<Messages> : public Server<Messages>, public TCalibrationCoordinatesRequestMessageDefinition
	{
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 100*KBytes, replyBufLen = 100*Bytes};

	public:
		TCalibrationCoordinatesRequest() : info(system::module::CalibrationCoordinatesRequest, sendBufLen, replyBufLen, MessageList::NumMessages) {}
		MessageInfo info;

	};



} // namespace interface
} // namespace precitec


#endif /*CALIBRATIONCOORDINATESREQUEST_INTERFACE_H_*/
