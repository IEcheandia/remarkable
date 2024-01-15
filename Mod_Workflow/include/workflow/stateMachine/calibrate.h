/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2011
 * 	@brief		This class represents the calibration state in the workflow state machine.
 */

#ifndef CALIBRATE_H_
#define CALIBRATE_H_

// project includes
#include "stateContext.h"
#include "abstractState.h"
#include "analyzer/inspectManager.h"
#include "Mod_Workflow.h"

namespace precitec {
namespace workflow {

/**
 * @ingroup Workflow
 * @brief This class represents the calibration state in the workflow state machine.
 */
class MOD_WORKFLOW_API Calibrate : public AbstractState
{
public:

	/**
	 * @brief CTor.
	 * @param p_pContext Pointer to StateContext.
	 */
	Calibrate(StateContext* p_pContext);


	// NOT SUPPORTED IN THIS STATE: void initialize()
	// NOT SUPPORTED IN THIS STATE: void ready()
	// NOT SUPPORTED IN THIS STATE: void startLive( const Poco::UUID& p_oProductID, int p_oSeamseries, int p_oSeam )
	// NOT SUPPORTED IN THIS STATE: void stopLive()
	// NOT SUPPORTED IN THIS STATE: void startAuto( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo )
	// NOT SUPPORTED IN THIS STATE: void stopAuto();
	// NOT SUPPORTED IN THIS STATE: void activateSeamSeries( int p_oSeamSeries );
	// NOT SUPPORTED IN THIS STATE: void beginInspect( int p_oSeam );
	// NOT SUPPORTED IN THIS STATE: void endInspect();
	// NOT SUPPORTED IN THIS STATE: void exit()
	void calibrate( unsigned int p_oMethod );
	void signalNotReady();
	// NOT SUPPORTED IN THIS STATE: void startProductTeachIn()
	// NOT SUPPORTED IN THIS STATE: void abortProductTeachIn()
	// NOT SUPPORTED IN THIS STATE: void quitSystemFault()
	virtual void emergencyStop();
	// NOT SUPPORTED IN THIS STATE: virtual void resetEmergencyStop();
	// NOT SUPPORTED IN THIS STATE: void seamPreStart( int p_oSeam );
};

} // namespace workflow
} // namespace precitec

#endif /* CALIBRATE_H_ */
