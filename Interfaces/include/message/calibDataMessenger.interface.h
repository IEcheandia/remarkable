#ifndef CALIBDATAMESSENGER_INTERFACE_H_
#define CALIBDATAMESSENGER_INTERFACE_H_

#include  "server/interface.h"
#include  "module/interfaces.h"
#include  "protocol/protocol.info.h"
#include  "message/calibDataMessenger.h"

namespace precitec {

	using namespace  system;
	using namespace  message;

namespace interface {

	template <int CallType>
	class TCalibDataMsg;

	template <>
	class TCalibDataMsg<AbstractInterface>
	{
	public:
		TCalibDataMsg() {}
		virtual ~TCalibDataMsg() {}
		virtual bool calibDataChanged(int, bool) = 0; //to not be confused with systemStatusHandler TSystemStatus::signalCalibDataChanged, this one is only used by CalibrationManager::sendCalibDataChangedSignal
		virtual bool set3DCoords(int, Calibration3DCoords, CalibrationParamMap) = 0;
		virtual bool sendCorrectionGrid(int, CalibrationCameraCorrectionContainer, CalibrationIDMCorrectionContainer ) = 0;
	};

    struct TCalibDataMsgDefinition
    {
		MESSAGE( bool, CalibDataChanged, int, bool );

		MESSAGE( bool, Set3DCoords, int, Calibration3DCoords, CalibrationParamMap);
		
		MESSAGE( bool, SendCorrectionGrid, int, CalibrationCameraCorrectionContainer, CalibrationIDMCorrectionContainer );

		MESSAGE_LIST(
			CalibDataChanged,
			Set3DCoords, 
			SendCorrectionGrid
		);
    };

	template <>
	class TCalibDataMsg<Messages> : public Server<Messages>, public TCalibDataMsgDefinition
	{
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 2 * 4 * MAX_CAMERA_HEIGHT * MAX_CAMERA_WIDTH * Bytes + 100*KBytes, replyBufLen = 500*Bytes, NumBuffers=4 };

	public:
		TCalibDataMsg() : info(system::module::CalibDataMsg, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	};



} // namespace interface
} // namespace precitec


#endif /*CALIBDATAMESSENGER_INTERFACE_H_*/
