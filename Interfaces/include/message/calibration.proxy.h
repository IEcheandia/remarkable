#ifndef CALIBRATION_PROXY_H_
#define CALIBRATION_PROXY_H_

#include  "message/calibration.interface.h"
#include  "server/proxy.h"

namespace precitec {
namespace interface {

	using namespace  message;
	using namespace  system;

	template <>
	class TCalibration<MsgProxy> : public Server<MsgProxy>, public TCalibration<AbstractInterface>, public TCalibrationMessageDefinition
	{
	public:

		TCalibration() : PROXY_CTOR(TCalibration), TCalibration<AbstractInterface>()
		{
		}
		TCalibration(SmpProtocolInfo &p) : PROXY_CTOR1(TCalibration,  p), TCalibration<AbstractInterface>()
		{
		}
		virtual ~TCalibration() {}

	public:

		virtual bool start( int iMethod )
		{
			INIT_MESSAGE(Start);
			sender().marshal( iMethod );
			sender().send();
			bool ret = false;
            sender().deMarshal(ret);
			return ret;
		}
	};

} // namespace interface
} // namespace precitec

#endif /* CALIBRATION_PROXY_H_ */
