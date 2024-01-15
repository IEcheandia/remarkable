/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2011
 * 	@brief		Server class for the calibration interface.
 */

#ifndef CALIBRATIONSERVER_H_
#define CALIBRATIONSERVER_H_

// project includes
#include <Mod_Calibration.h>
#include <message/calibration.h>
#include <message/calibration.interface.h>
#include <analyzer/resultHandler.h>

namespace precitec {
namespace calibration {

class CalibrationManager;

/**
 * @ingroup Calibration
 * @brief Server class for the calibration interface.
 *
 * @details The server calls the calibration-manager and thereby starts the calibration. The server is the remote side of the calibration interface - the Workflow state-machine uses the
 * interface when it goes into the calibration state.
 */
class CalibrationServer : public TCalibration<AbstractInterface>
{
	public:

		/**
		 * @brief CTor.
		 * @param p_pCalibrationManager Pointer to calibration manager that is handling the actual calibration mechanism.
		 */
		CalibrationServer( CalibrationManager* p_pCalibrationManager ) :
			m_pCalibrationManager( p_pCalibrationManager )
		{
		};
		/**
		 * @brief DTor.
		 */
		virtual ~CalibrationServer()
		{
		};

	public:

		/**
		 * @brief Start a calibration method. There are different calibration methods implemented, selectable via an enum, which is defined in calibration.interface.h
		 * @param p_oMethod integer representing a calibration method, see. Interfaces/include/message/calibration.interface.h
		 */
		bool start( int p_oMethod )
		{
		#if defined __QNX__ || defined __linux__
			return m_pCalibrationManager->calibrate( p_oMethod );
		#else
			return false;
		#endif
		};

		/**
		 * @brief Get a reference to the calibration manager.
		 * @return Reference to instance of CalibrationManager.
		 */
		CalibrationManager& getCalibrationManager()
		{
			return *m_pCalibrationManager;
		};

	protected:

		CalibrationManager *m_pCalibrationManager;	///< Pointer to calibration manager that is handling the actual calibration mechanism.

};

} // namespace calibation
} // namespace precitec

#endif /* CALIBRATIONSERVER_H_ */
