#ifndef CALIBDATAMESSENGER_SERVER_H_
#define CALIBDATAMESSENGER_SERVER_H_

#include  <map> // wg HandlerList
#include  "Poco/Process.h"
#include  "Poco/Path.h"

#include  "message/calibDataMessenger.interface.h"

namespace precitec {
namespace interface {

	template <>
	class TCalibDataMsg<MsgServer> : public TCalibDataMsg<AbstractInterface>
	{
	public:
		TCalibDataMsg(){}
		virtual ~TCalibDataMsg(){}

		virtual bool calibDataChanged( int p_oSensorID, bool p_oInit )
		{
			std::cout << "DEBUG: TCalibration<MsgServer>::calibDataChanged( " << p_oSensorID << ") called!!" << std::endl;
			return true;
		}

		virtual bool set3DCoords(int p_oSensorID, Calibration3DCoords p_oCoords, CalibrationParamMap p_oParams) 
		{
			std::cout << "DEBUG: TCalibration<MsgServer>::set3DCoords( " << p_oSensorID << ") called!!" << std::endl;
			return true;
		}
		
        virtual bool sendCorrectionGrid(int, CalibrationCameraCorrectionContainer, CalibrationIDMCorrectionContainer )
        {
            std::cout << "DEBUG: TCalibration<MsgServer>::" <<  __FUNCTION__ <<  " called!!" << std::endl;
            return true;
        }

	};

} // namespace interface
} // namespace precitec

#endif /* CALIBDATAMESSENGER_SERVER_H_ */
