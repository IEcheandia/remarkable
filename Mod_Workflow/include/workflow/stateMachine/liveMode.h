/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR
 *  @date			2009
 *  @brief			This class represents the live mode state in the state machine. In case the machine starts a new automatic cycle, the state machine
 *  				will transition directly into the automatic mode state.
 */

#ifndef LIVEMODE_H_
#define LIVEMODE_H_

// project includes
#include "stateContext.h"
#include "abstractState.h"

namespace precitec {
namespace workflow {

/**
 * @ingroup Workflow
 * @brief	This class represents the live mode state in the state machine.
 */
class LiveMode : public AbstractState
{
public:

	LiveMode(StateContext* p_pContext);


	// NOT SUPPORTED IN THIS STATE: void initialize()
	// NOT SUPPORTED IN THIS STATE: void ready()
	// NOT SUPPORTED IN THIS STATE: void startLive( const Poco::UUID& p_oProductID, int p_oSeamseries, int p_oSeam )
	void stopLive();
	void startAuto( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo );
	// NOT SUPPORTED IN THIS STATE: void stopAuto();
	void activateSeamSeries( int p_oSeamSeries );
	void beginInspect( int p_oSeam, const std::string &label ) override;
	void endInspect();
	// NOT SUPPORTED IN THIS STATE: void exit()
	// NOT SUPPORTED IN THIS STATE: void calibrate( unsigned int p_oMethod )
	void signalNotReady();
	// NOT SUPPORTED IN THIS STATE: void startProductTeachIn()
	// NOT SUPPORTED IN THIS STATE: void abortProductTeachIn()
	// NOT SUPPORTED IN THIS STATE: void quitSystemFault()
	virtual void emergencyStop();
	// NOT SUPPORTED IN THIS STATE: virtual void resetEmergencyStop();
	// NOT SUPPORTED IN THIS STATE: virtual void seamPreStart( int p_oSeam );

    void updateLiveMode() override;
};

} // namespace workflow
} // namespace precitec

#endif /*LIVEMODE_H_*/
