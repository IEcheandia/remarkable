/**
 * 	@file
 *  @copyright		Precitec GmbH & Co. KG
 *  @author			AL
 *  @date			2014
 *  @brief			This class represents the emergency stop state in the workflow state machine.
 */

#include "common/systemConfiguration.h"
#include "workflow/stateMachine/emergencyStop.h"

namespace precitec {
using namespace interface;
namespace workflow {


EmergencyStop::EmergencyStop(StateContext* p_pContext) : AbstractState(eEmergencyStop, p_pContext)
{
    setOperationState( precitec::interface::EmergencyStop );
	m_pContext->getSystemStatus().workingState(precitec::interface::Stopped);

	m_pContext->enableLineLasers( false );
	//set error bit
	m_pContext->getInspectionOut().setSumErrorLatched(true);
	//reset SystemReady signal
	m_pContext->getInspectionOut().setSystemReady(false);
//	m_pContext->getInspectionOut().setSystemErrorField( m_pContext->getRelatedException() );
	// Change status back to NotReadyMode

    if (SystemConfiguration::instance().getBool("AxisXEnable", false) ||
        SystemConfiguration::instance().getBool("AxisYEnable", false) ||
        SystemConfiguration::instance().getBool("AxisZEnable", false) )
    {
        m_pContext->getWeldHeadMsgProxy().setHeadMode( eAxisY, Offline, false );
    }

} // CTor


void EmergencyStop::resetEmergencyStop()
{
    if (SystemConfiguration::instance().getBool("AxisXEnable", false) ||
        SystemConfiguration::instance().getBool("AxisYEnable", false) ||
        SystemConfiguration::instance().getBool("AxisZEnable", false) )
    {
        if( m_pContext->getForceHomingOfAxis() ){

            wmLogTr(eError, "QnxMsg.VI.AxisDoHomingFirst1", "Axis have no valid home position !\n");
            wmLogTr(eError, "QnxMsg.VI.AxisDoHomingFirst2", "Please do first homing of axis !\n");
            m_pContext->changeState(eNotReady);	// warning: 'this' will be deleted.
            return;
        }
    }

	// if there is a problem with the system, we have to go back to the not-ready state.
	if(m_pContext->getRelatedException() != eNone )
	{
		m_pContext->changeState(eNotReady);	// warning: 'this' will be deleted.
	}
	else
	{
		m_pContext->changeState(eOperate);	// warning: 'this' will be deleted.
	}

} // resetEmergencyStop



} // workflow
} // precitec
