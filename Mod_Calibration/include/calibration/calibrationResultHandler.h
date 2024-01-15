/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB), Andreas Beschorner (AB)
 * 	@date		2013
 * 	@brief		Result handler similar to the analyzer resultHandler.cpp, but without sending results to windows.
 */

#ifndef CALIBRATIONRESULTHANDLER_H_
#define CALIBRATIONRESULTHANDLER_H_

// project includes
#include <Mod_Calibration.h>
#include <fliplib/Fliplib.h>
#include <fliplib/SinkFilter.h>
#include <fliplib/PipeEventArgs.h>
#include <fliplib/SynchronePipe.h>
#include <event/results.h>

namespace precitec {
namespace calibration {

/**
 * @ingroup Calibration
 * @brief Result handler similar to the analyzer resultHandler.cpp, but without sending results to windows.
 */
class MOD_CALIBRATION_API CalibrationResultHandler : public fliplib::SinkFilter
{
public:

	/**
	 * CTor.
	 */
	CalibrationResultHandler();
	/**
	 * DTor.
	 */
	virtual ~CalibrationResultHandler();

	/**
	 * Clear the internal result buffer.
	 */
	void clear();

	/**
	 * Get the results.
	 */
	std::vector< interface::ResultArgs* > getResults();

protected:

	/**
	 * The sink-filter proceed routine.
	 */
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

protected:

	std::vector< interface::ResultArgs* > m_oResults;		///< Buffer with the results.
};

} // namespace calibration
} // namespace precitec

#endif /* CALIBRATIONRESULTHANDLER_H_ */
