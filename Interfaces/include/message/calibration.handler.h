#ifndef CALIBRATION_HANDLER_H_
#define CALIBRATION_HANDLER_H_

#include  "Poco/Process.h"

#include  "message/calibration.h"
#include  "server/handler.h"
#include  "message/calibration.interface.h"

namespace precitec {

using namespace  system;
using namespace  message;

namespace interface {

	template <>
	class TCalibration<MsgHandler> : public Server<MsgHandler>, public TCalibrationMessageDefinition
	{
	public:

		MSG_HANDLER(TCalibration);

	public:

		void registerCallbacks()
		{
			REGISTER_MESSAGE(Start, start);
		}

		void start(Receiver &receiver)
		{
			int oMethod; receiver.deMarshal( oMethod );
			bool ret = server_->start( oMethod );
			receiver.marshal(ret);
			receiver.reply();
		}
	};

} // namespace interface
} // namespace precitec

#endif /* CALIBRATION_HANDLER_H_ */
