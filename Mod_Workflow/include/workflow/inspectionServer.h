/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR
 *  @date			2009
 *  @brief			Signal inspection setup and inspection states.
 */

#ifndef INSPECTIONSERVER_H_
#define INSPECTIONSERVER_H_

#include "Mod_Workflow.h"
#include "event/inspection.h"
#include "event/inspection.interface.h"
#include "stateMachine/stateContext.h"
#include "common/IWatchdogTimeoutNotify.h"
#include "module/moduleLogger.h"

#include <memory>

namespace precitec {
using namespace interface;
namespace workflow {


class MOD_WORKFLOW_API InspectionServer : public TInspection<AbstractInterface>
{
public:

    enum class HostConnectionMonitoring
    {
        Enabled,
        Disabled
    };
	InspectionServer( SmStateContext p_pStateContext );

    

public:

	// Start new automatic cycle.
	void startAutomaticmode( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo);
	// Stop automatic cycle.
	void stopAutomaticmode();
	// Inspection Start -> Naht aktiv
	void start( int p_oSeam );
	// Inspection Ende -> Naht ! aktiv
	void end ( int p_oSeam );
	// Nahtfolge uebernehmen
	void info( int p_oSeamSeries );
	// Linelaser ein/aus
	void linelaser ( bool onoff );
	// Kalibration Start
	void startCalibration();
	// Kalibration Ende
	void stopCalibration();
	// Naht Vor-Start
	void seamPreStart( int p_oSeam );

	


private:

	SmStateContext m_pStateContext;

};

}	// workflow
}	// precitec


#endif /*INSPECTIONSERVER_H_*/
