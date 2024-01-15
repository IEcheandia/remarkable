#ifndef NORMALMODE_H_
#define NORMALMODE_H_

// project includes
#include "common/product.h"
#include "stateContext.h"
#include "abstractState.h"
// poco includes
#include "Poco/UUID.h"

namespace precitec {
namespace workflow {

class Operate : public AbstractState
{
public:

	Operate( StateContext* p_pContext );


	// NOT SUPPORTED IN THIS STATE: void initialize()
	// NOT SUPPORTED IN THIS STATE: void ready()
	void startLive( const Poco::UUID& p_oProductID, int p_oSeamseries, int p_oSeam );
	// NOT SUPPORTED IN THIS STATE: void stopLive()
	void startAuto( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo );
	// NOT SUPPORTED IN THIS STATE: void stopAuto();
	// NOT SUPPORTED IN THIS STATE: void activateSeamSeries( int p_oSeamSeries );
	// NOT SUPPORTED IN THIS STATE: void beginInspect( int p_oSeam );
	// NOT SUPPORTED IN THIS STATE: void endInspect();
	void exit();
	void calibrate( unsigned int p_oMethod );
	void signalNotReady();
	void startProductTeachIn();
	// NOT SUPPORTED IN THIS STATE: void abortProductTeachIn()
	// NOT SUPPORTED IN THIS STATE: void quitSystemFault()
	virtual void emergencyStop();
	// NOT SUPPORTED IN THIS STATE: virtual void resetEmergencyStop();
	// NOT SUPPORTED IN THIS STATE: void seamPreStart( int p_oSeam );
};

} // namespace workflow
} // namespace precitec

#endif /*NORMALMODE_H_*/
