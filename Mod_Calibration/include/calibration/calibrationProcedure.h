/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2011
 * 	@brief		Base-class representing a single calibration procedure.
 */

#ifndef CALIBRATIONPROCEDURE_H_
#define CALIBRATIONPROCEDURE_H_

#include <Mod_Calibration.h>

namespace precitec {
namespace calibration {

class CalibrationManager;

/**
 * @ingroup Calibration
 * @brief Base-class representing a single calibration procedure.
 *
 * @details This base-class represents a single calibration procedure, which is controlled and executed via the CalibrationManager. The calibration interface distinguishes between the procedures via an
 * enum, which can be found in Interfaces/message/calibration.interface.h. To introduce a new procedure one has to derive a new class from CalibrationProcedure and has to implement the calibrate() function. The
 * enum needs to be expanded and the calibrate() function needs to start the procedure. Please keep the header files of the manager and the interface clean, they are visible to the windows side and wmNative is easily affected by changes.
 */
class CalibrationProcedure
{
public:

	/**
	 * CTor
	 * \param p_rCalibrationManager reference to the calibration-manager
	 */
	CalibrationProcedure( CalibrationManager& p_rCalibrationManager ) :
		m_rCalibrationManager( p_rCalibrationManager )
	{
	}

	virtual ~CalibrationProcedure() {}


	/**
	 * Execute calibration procedure
	 */
	virtual bool calibrate()=0;

protected:

	CalibrationManager& m_rCalibrationManager;	///< Reference to calibration manager, which is used to receive images, encoder values, etc.
};

} // namespace analyzer
} // namespace precitec

#endif /* CALIBRATIONPROCEDURE_H_ */
