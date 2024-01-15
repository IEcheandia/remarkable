#ifndef CALIBRATION_INTERFACE_H_
#define CALIBRATION_INTERFACE_H_

#include  "server/interface.h"
#include  "module/interfaces.h"
#include  "protocol/protocol.info.h"
#include  "message/calibration.h"

namespace precitec {

	using namespace  system;
	using namespace  message;

namespace interface {



	/**
	 * The central enum that holds the identifiers of all the calibration procedures - if one wants to write a new calibration procedure, it needs to be added here.
	 * ACHTUNG: bestehende Eintraege im enum CalibType und deren Wertzuweisung muessen unbedingt erhalten bleiben ! Werden so ueber Feldbus angesteuert !
	 */
	enum CalibType {
        eCalibrateAxisIb = 0,
        eCalibrateOsIbLine0, 
        eBlackLevelOffset, 
        eCalibrateGridOsIb, 
        eInitCalibrationData, 
        eCalibGridAngle, 
        eCalibrateOsIbLineTCP, 
        eCalibrateOsIbLine2, 
        eOCT_LineCalibration,
        eOCT_TCPCalibration,
        eAcquireScanFieldImage,
        eCalibrateScanFieldTarget, 
        eCalibrateScanFieldIDM_Z,
        eVerifyScanFieldIDM_Z,
        eIDMDarkReference,
        eCalibGridChessboard,
        eScannerCalibration,
        eScannerCalibrationMeasure,
        eScanmasterCameraCalibration,
        eHomingY = 0xE0, 
        eHomingZ, 
        eHomingZColl,
        eCalibrateLED
    }; // must coincide with CalibType.cs struct, Util Project


	template <int CallType>
	class TCalibration;


	template <>
	class TCalibration<AbstractInterface>
	{
	public:
		TCalibration() {}
		virtual ~TCalibration() {}
	public:
		virtual bool start(int) = 0;
	};

    struct TCalibrationMessageDefinition
    {
        MESSAGE( bool, Start, int );
        MESSAGE_LIST(Start);
    };


	template <>
	class TCalibration<Messages> : public Server<Messages>, public TCalibrationMessageDefinition
	{
	public:
		TCalibration() : info(module::Calibration, sendBufLen, replyBufLen, MessageList::NumMessages) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 5000*Bytes, replyBufLen = 5000*Bytes };
	};



} // namespace interface
} // namespace precitec


#endif /*CALIBRATION_INTERFACE_H_*/
