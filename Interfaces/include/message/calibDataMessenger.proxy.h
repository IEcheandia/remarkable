#ifndef CALIBDATAMESSENGER_PROXY_H_
#define CALIBDATAMESSENGER_PROXY_H_

#include  "message/calibDataMessenger.interface.h"
#include  "server/proxy.h"

namespace precitec {
namespace interface {

	using namespace  message;
	using namespace  system;

	template <>
	class TCalibDataMsg<MsgProxy> : public Server<MsgProxy>, public TCalibDataMsg<AbstractInterface>, public TCalibDataMsgDefinition
	{
	public:

		TCalibDataMsg() : PROXY_CTOR(TCalibDataMsg), TCalibDataMsg<AbstractInterface>()
		{
		}
		TCalibDataMsg(SmpProtocolInfo &p) : PROXY_CTOR1(TCalibDataMsg, p), TCalibDataMsg<AbstractInterface>()
		{
		}
		virtual ~TCalibDataMsg() {}

	public:

		virtual bool calibDataChanged( int p_oSensorID, bool p_oInit )
		{
			INIT_MESSAGE(CalibDataChanged);
			sender().marshal( p_oSensorID );
			sender().marshal( p_oInit );
			sender().send();
			bool ret = false;
			sender().deMarshal(ret);
			return ret;
		}

		virtual bool set3DCoords(int p_oSensorID, Calibration3DCoords p_oCoords, CalibrationParamMap p_oParams) 
		{
			INIT_MESSAGE(Set3DCoords);
			sender().marshal( p_oSensorID );
			sender().marshal( p_oCoords);
			sender().marshal( p_oParams);
			sender().send();
			bool ret = false;
			sender().deMarshal(ret);
			return ret;
		}
		
        virtual bool sendCorrectionGrid(int p_oSensorID, CalibrationCameraCorrectionContainer p_oCameraCorrectionGridInitializer, CalibrationIDMCorrectionContainer p_oIDMCorrectionContainer)
        {
            INIT_MESSAGE(SendCorrectionGrid);
			sender().marshal( p_oSensorID );
			sender().marshal( p_oCameraCorrectionGridInitializer );
			sender().marshal( p_oIDMCorrectionContainer );
			sender().send();
			bool ret = false;
			sender().deMarshal(ret);
			return ret;
        }

	};

} // namespace interface
} // namespace precitec

#endif /* CALIBDATAMESSENGER_PROXY_H_ */
