/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR, SB
 *  @date			2009
 *  @brief			This class represents the automatic state in the workflow statemachine.
 */

#include "workflow/stateMachine/automaticMode.h"

#include "common/systemConfiguration.h"

namespace precitec {
using namespace interface;
namespace workflow {



AutomaticMode::AutomaticMode(StateContext* p_pContext) : AbstractState(eAutomaticMode, p_pContext)
{
    if (!m_pContext->isSimulationStation())
    {
        m_pContext->getGrabberStatusProxy().preActivityNotification( p_pContext->getProductNumber() );	// signal coming automatic mode to grabber
    }

    setOperationState(precitec::interface::AutomaticMode);
	m_pContext->getSystemStatus().workingState(precitec::interface::WaitForTrigger);
    // switch on of line lasers and field light is commented out, should be done in hardware paramter set (18.4.2018 EA)
    // this reduces delay at cycle start
	//m_pContext->enableLineLasers( true );
	m_pContext->inspectManager().startAutomaticMode();
	m_pContext->getInspectionOut().setSumErrorLatched(false); 	// Fehlerausgang zuruecksetzen
	m_pContext->getInspectionOut().setInspectCycleAckn(true); 	// Anzeigen, dass Initialisierung des automaticModes abgeschlossen ist
} // CTor


void AutomaticMode::stopAuto()
{
    // switch off line lasers only if it is not continuous mode
    if (SystemConfiguration::instance().getBool("ContinuouslyModeActive", false) == false)
    {
        m_pContext->enableLineLasers( false );
    }
	m_pContext->inspectManager().stopAutomaticMode();

	if(m_pContext->getIsBackupRequested() && m_pContext->getRelatedException() == 512)
	{
		m_pContext->changeState(eNotReady);	// warning: 'this' will be deleted.
	}
	else
	{
		m_pContext->changeState(eOperate);	// warning: 'this' will be deleted.
	}

} // stopAuto



void AutomaticMode::activateSeamSeries( int p_oSeamSeries )
{
	m_pContext->inspectManager().activateSeamSeries(p_oSeamSeries);
} // activateSeamSeries



void AutomaticMode::beginInspect( int p_oSeam, const std::string &label )
{
	if ( !m_pContext->inspectManager().startInspect( m_pContext->getSeamseries(), p_oSeam, label ) )
		m_pContext->inspectManager().stopInspect();
	else
		m_pContext->getSystemStatus().workingState(Triggered);

} // beginInspect



void AutomaticMode::endInspect()
{
	m_pContext->inspectManager().stopInspect();
	m_pContext->getSystemStatus().workingState(Stopped);

} // endInspect



void AutomaticMode::signalNotReady()
{
	if(m_pContext->getRelatedException() == 512)
	{
		m_pContext->setIsBackupRequested(true);
		wmLog( eDebug, "State[AutomaticMode]::signalNotReady! (Backup request)\n");
	}
	else
	{
		wmLog( eDebug, "State[AutomaticMode]::signalNotReady!\n");

		m_pContext->enableLineLasers( false );
		m_pContext->inspectManager().stopAutomaticMode();
		m_pContext->getSystemStatus().workingState(Stopped);

		m_pContext->changeState(eNotReady);	// warning: 'this' will be deleted.
	}

} // signalNotReady



void AutomaticMode::emergencyStop()
{
	wmLog( eDebug, "State[AutomaticMode]::emergencyStop!\n");

	m_pContext->enableLineLasers( false );
	m_pContext->inspectManager().stopAutomaticMode();
	m_pContext->getSystemStatus().workingState(Stopped);

	m_pContext->changeState(eEmergencyStop);	// warning: 'this' will be deleted.

} // emergencyStop



void AutomaticMode::seamPreStart( int p_oSeam )
{
	if ( !m_pContext->inspectManager().seamPreStart( m_pContext->getSeamseries(), p_oSeam ) )
		wmLogTr(eError, "QnxMsg.Workflow.SeamPreStartFail", "Was not able to execute seam pre start\n");

} // seamPreStart



} // workflow
} // precitec
