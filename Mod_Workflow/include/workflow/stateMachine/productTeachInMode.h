#ifndef PRODUCTTEACHINMODE_H_
#define PRODUCTTEACHINMODE_H_

#include "common/product.h"
#include "stateContext.h"
#include "abstractState.h"
#include "system/timer.h"

namespace precitec
{
using namespace interface;

namespace workflow
{

	class ProductTeachInMode : public AbstractState
	{
		public:
		ProductTeachInMode(StateContext* context) : AbstractState(eProductTeachIn, context),
			stopWatch_("ProductTeachInStopWatch")
			{
				setOperationState( precitec::interface::ProductTeachInMode );

			}
		virtual ~ProductTeachInMode() {}


		// NOT SUPPORTED IN THIS STATE: void initialize()
		// NOT SUPPORTED IN THIS STATE: void ready()
		// NOT SUPPORTED IN THIS STATE: void startLive( const Poco::UUID& p_oProductID, int p_oSeamseries, int p_oSeam )
		// NOT SUPPORTED IN THIS STATE: void stopLive()
		void startAuto( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo );
		void stopAuto();
		void activateSeamSeries( int p_oSeamSeries );
		void beginInspect( int p_oSeam, const std::string &label ) override;
		void endInspect();
		// NOT SUPPORTED IN THIS STATE: void exit()
		// NOT SUPPORTED IN THIS STATE: void calibrate( unsigned int p_oMethod )
		void signalNotReady();
		// NOT SUPPORTED IN THIS STATE: void startProductTeachIn()
		void abortProductTeachIn();
		// NOT SUPPORTED IN THIS STATE: void quitSystemFault()
		virtual void emergencyStop();
		// NOT SUPPORTED IN THIS STATE: virtual void resetEmergencyStop();
		// NOT SUPPORTED IN THIS STATE: virtual void seamPreStart( int p_oSeam );


	private:
		system::Timer stopWatch_;
	};

}

}

#endif /*PRODUCTTEACHINMODE_H_*/
