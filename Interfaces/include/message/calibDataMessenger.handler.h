#ifndef CALIBDATAMESSENGER_HANDLER_H_
#define CALIBDATAMESSENGER_HANDLER_H_

#include <vector>
#include  "Poco/Process.h"

#include  "message/calibDataMessenger.h"
#include  "server/handler.h"
#include  "message/calibDataMessenger.interface.h"

namespace precitec {

using namespace  system;
using namespace  message;

namespace interface {

	template <>
	class TCalibDataMsg<MsgHandler> : public Server<MsgHandler>, public TCalibDataMsgDefinition
	{
	public:

		MSG_HANDLER(TCalibDataMsg);

	public:

		void registerCallbacks()
		{
			REGISTER_MESSAGE(CalibDataChanged, calibDataChanged);
			REGISTER_MESSAGE(Set3DCoords, set3DCoords);
			REGISTER_MESSAGE(SendCorrectionGrid, sendCorrectionGrid);
		}

		void calibDataChanged(Receiver &receiver)
		{
			int oSensorID; receiver.deMarshal( oSensorID );
			bool oInit; receiver.deMarshal( oInit );
			bool ret = server_->calibDataChanged( oSensorID, oInit );
			receiver.marshal(ret);
			receiver.reply();
		}

		void set3DCoords(Receiver &receiver)
		{
			int oSensorID; receiver.deMarshal( oSensorID );
			Calibration3DCoords oCoords; receiver.deMarshal( oCoords);
			CalibrationParamMap oParams; receiver.deMarshal(oParams);
			bool ret = server_->set3DCoords(oSensorID, oCoords, oParams);
			
			receiver.marshal(ret);
			receiver.reply();
		}
		
        void sendCorrectionGrid(Receiver & receiver)
        {
            int oSensorID; receiver.deMarshal(oSensorID);
            CalibrationCameraCorrectionContainer oCameraCorrectionContainer; receiver.deMarshal( oCameraCorrectionContainer );
            CalibrationIDMCorrectionContainer oIDMCorrectionContainer; receiver.deMarshal( oIDMCorrectionContainer );
            bool ret = server_->sendCorrectionGrid(oSensorID, oCameraCorrectionContainer , oIDMCorrectionContainer);
            receiver.marshal(ret);
            receiver.reply();
        }
	};

} // namespace interface
} // namespace precitec

#endif /* CALIBDATAMESSENGER_HANDLER_H_ */
