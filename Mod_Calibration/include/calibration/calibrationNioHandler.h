/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2013
 * 	@brief		NIO handler, derived from calibrationResultHandler. Sets an internal flag if a result is received and therefore the graph has reported an NIO.
 */

#ifndef CALIBRATIONNIOHANDLER_H_
#define CALIBRATIONNIOHANDLER_H_

// project includes
#include <calibration/calibrationResultHandler.h>

namespace precitec {
namespace calibration {

/**
 * @ingroup Calibration
 * @brief NIO handler, derived from calibrationResultHandler. Sets an internal flag if a result is received and therefore the graph has reported an NIO.
 */
class CalibrationNioHandler : public CalibrationResultHandler
{
public:

	/**
	 * @brief CTor.
	 */
	CalibrationNioHandler();
	/**
	 * @brief DTor.
	 */
	virtual ~CalibrationNioHandler();

	/**
	 * @brief Clear the internal result buffer and set the NIO flag back.
	 */
	void clear();

	/**
	 * @brief Was the last image NIO?
	 */
	bool isNIO();

protected:

	/**
	 * The sink-filter proceed routine. If this was ever called, the part/image is NIO.
	 */
	void proceed(const void* sender, fliplib::PipeEventArgs& e);

protected:

	bool m_bNIO;	///< Was the last image NIO?

};

} // namespace calibration
} // namespace precitec

#endif /* CALIBRATIONNIOHANDLER_H_ */
