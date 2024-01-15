/**
 * 	@file
 *  @copyright		Precitec GmbH & Co. KG
 *  @author			AL
 *  @date			2014
 *  @brief			This class represents the emergency stop state in the workflow state machine.
 */

#ifndef EMERGENCYSTOP_H_
#define EMERGENCYSTOP_H_

// project includes
#include "stateContext.h"
#include "abstractState.h"

namespace precitec {
using namespace interface;
namespace workflow {

/**
 * @ingroup Workflow
 * @brief This class represents the automatic state in the workflow state machine.
 */
class EmergencyStop : public AbstractState
{
public:

	EmergencyStop( StateContext* p_pContext );


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
	// NOT SUPPORTED IN THIS STATE: void calibrate( unsigned int p_oMethod )
	// NOT SUPPORTED IN THIS STATE: void signalNotReady();
	// NOT SUPPORTED IN THIS STATE: void startProductTeachIn()
	// NOT SUPPORTED IN THIS STATE: void abortProductTeachIn()
	// NOT SUPPORTED IN THIS STATE: void quitSystemFault()
	// NOT SUPPORTED IN THIS STATE: virtual void emergencyStop();
	virtual void resetEmergencyStop();
	// NOT SUPPORTED IN THIS STATE: void seamPreStart( int p_oSeam );
};

} // namespace workflow
} // namespace precitec

#endif /*EMERGENCYSTOP_H__*/
