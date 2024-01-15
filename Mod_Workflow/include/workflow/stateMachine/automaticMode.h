/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR, SB
 *  @date			2009
 *  @brief			This class represents the automatic state in the workflow state machine.
 */

#ifndef AUTOMATICMODE_H_
#define AUTOMATICMODE_H_

// project includes
#include "common/product.h"
#include "stateContext.h"
#include "abstractState.h"

namespace precitec {
using namespace interface;
namespace workflow {

/**
 * @ingroup Workflow
 * @brief This class represents the automatic state in the workflow state machine.
 */
class AutomaticMode : public AbstractState
{
public:

	AutomaticMode( StateContext* p_pContext );


	// NOT SUPPORTED IN THIS STATE: void initialize()
	// NOT SUPPORTED IN THIS STATE: void ready()
	// NOT SUPPORTED IN THIS STATE: void startLive( const Poco::UUID& p_oProductID, int p_oSeamseries, int p_oSeam )
	// NOT SUPPORTED IN THIS STATE: void stopLive()
	// NOT SUPPORTED IN THIS STATE: void startAuto( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo )
	void stopAuto();
	void activateSeamSeries( int p_oSeamSeries );
	void beginInspect( int p_oSeam, const std::string &label  ) override;
	void endInspect();
	// NOT SUPPORTED IN THIS STATE: void exit()
	// NOT SUPPORTED IN THIS STATE: void calibrate( unsigned int p_oMethod )
	void signalNotReady();
	// NOT SUPPORTED IN THIS STATE: void startProductTeachIn()
	// NOT SUPPORTED IN THIS STATE: void abortProductTeachIn()
	// NOT SUPPORTED IN THIS STATE: void quitSystemFault()
	virtual void emergencyStop();
	// NOT SUPPORTED IN THIS STATE: virtual void resetEmergencyStop();
	void seamPreStart( int p_oSeam );
};

} // namespace workflow
} // namespace precitec

#endif /*AUTOMATICMODE_H_*/
