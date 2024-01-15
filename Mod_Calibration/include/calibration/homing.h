/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Fabian Agrawal (AL), Stefan Birmanns (SB)
 * 	@date		2011
 * 	@brief		Homing of the weld-head axis.
 */

#ifndef HOMING_H_
#define HOMING_H_

#include <message/calibration.interface.h>
#include <calibration/calibrationProcedure.h>

namespace precitec {
namespace calibration {

/**
 * @ingroup Calibration
 * @brief Homing of the weld-head axis.
 */
class Homing : public CalibrationProcedure
{
public:

	/**
	 * @brief CTor.
	 * @param p_rCalibrationManager Reference to the calibration manager.
	 */
	Homing( CalibrationManager& p_rCalibrationManager ) : CalibrationProcedure( p_rCalibrationManager )
	{
	};

	/**
		 * Execute calibration procedure
		 */
		virtual bool calibrate() { return false; };

	/**
	 * @brief Execute calibration procedure.
	 * @return True if the procedure was successful.
	 */
	bool calibrate(interface::CalibType p_oCalibType);
};

} // namespace calibration
} // namespace precitec

#endif /* HOMING_H_ */
