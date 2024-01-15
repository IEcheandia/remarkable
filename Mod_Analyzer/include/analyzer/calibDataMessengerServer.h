/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Andreas Beschorner (BA)
 * 	@date		2014
 * 	@brief		Server class for the calibration analyzer communication.
 */

#ifndef CALIBDATAMESSENGERSERVER_H_
#define CALIBDATAMESSENGERSERVER_H_

// project includes
#include <Mod_Calibration.h>
#include <message/calibDataMessenger.h>
#include <message/calibDataMessenger.interface.h>
#include <analyzer/resultHandler.h>

namespace precitec {
namespace analyzer {

class CalibrationManager;

/**
 * @ingroup Workflow
 * @brief Server class for the calibDataMessenger interface.
 *
 * @details The server calls forwards the dataCalibChanged signal to the inspectManager, which reacts by reloading the data and setting/computing a new grid.
 */
class MOD_ANALYZER_API CalibDataMessengerServer : public interface::TCalibDataMsg<interface::AbstractInterface>
{
	public:

		/**
		 * @brief CTor.
		 * @param p_pInspectManager   Pointer to inspect manager handling the data reload.
		 */
		CalibDataMessengerServer( InspectManager *p_pInspectManager) : m_pInspectManager(p_pInspectManager)
		{
		}

		/**
		 * @brief DTor.
		 */
		virtual ~CalibDataMessengerServer()
		{
		}

		/**
		 * @brief Start reloading of calibration data for workflow/analzyer.
		 */
		bool calibDataChanged( int p_oSensorID, bool p_oInit )
		{    
            wmLog(eInfo, "Received calibDataChanged %d signal for sensor %d\n", p_oInit, p_oSensorID);            
            bool ok= m_pInspectManager->loadCalibDataAfterSignal(p_oSensorID, p_oInit);
            wmLog(eDebug, "InspectManager has processed loadCalibDataAfterSignal \n");
			return ok;
		}

		bool set3DCoords(int p_oSensorID, Calibration3DCoords p_oCoords, CalibrationParamMap p_oParams) 
		{
			return m_pInspectManager->reloadCalibData(p_oSensorID, p_oCoords, p_oParams);
		}
		
        bool sendCorrectionGrid(int p_oSensorID, CalibrationCameraCorrectionContainer p_oCameraCorrectionContainer, CalibrationIDMCorrectionContainer p_oIDMCorrectionContainer)
        {
            return m_pInspectManager->updateCorrectionGrid(p_oSensorID, p_oCameraCorrectionContainer, p_oIDMCorrectionContainer);
        }
        
		/**
		 * @brief Get a reference to the inspect manager.
		 * @return Reference to instance of InspectManager.
		 */
		InspectManager& getInspectManager()
		{
			return *m_pInspectManager;
		}

	protected:

		InspectManager *m_pInspectManager;	///< Pointer to inspect manager.

};

} // namespace calibation
} // namespace precitec

#endif /* CALIBDATAMESSENGERSERVER_H_ */
