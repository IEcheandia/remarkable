#ifndef CALIBRATION_SERVER_H_
#define CALIBRATION_SERVER_H_

#include  <map> // wg HandlerList
#include  "Poco/Process.h"
#include  "Poco/Path.h"

#include  "message/calibration.interface.h"

namespace precitec {
namespace interface {

	template <>
	class TCalibration<MsgServer> : public TCalibration<AbstractInterface>
	{
	public:

		virtual bool start( int iMethod )
		{
			std::cout << "DEBUG: TCalibration<MsgServer>::start( " << iMethod << ") called!!" << std::endl;
		}
	};

} // namespace interface
} // namespace precitec

#endif /* CALIBRATION_SERVER_H_ */
