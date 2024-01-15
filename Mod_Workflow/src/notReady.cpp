/**
 * @file
 * @copyright	Precitec KG
 * @author		Fabian Agrawal (AL)
 * @date		2011
 * @brief		This class represents the notReady state in the workflow statemachine.
 */

#include "../include/workflow/stateMachine/notReady.h"

namespace precitec {
namespace workflow {


NotReady::NotReady(StateContext* p_pContext) : AbstractState(eNotReady, p_pContext)
{
	//disable LineLaser
	m_pContext->enableLineLasers( false );
// TODO: Linienlaser-Helligkeiten sollen kueftig ueber Device Interface gesetzt werden ! (13.2.13 SB / EA)
//			//disable LaserLight
//			m_pContext->enableLineLaser2( false );
	//set error bit
	m_pContext->getInspectionOut().setSumErrorLatched(true);
	//reset SystemReady signal
	m_pContext->getInspectionOut().setSystemReady(false);
	m_pContext->getInspectionOut().setSystemErrorField( m_pContext->getRelatedException() );
	// Change status back to NotReadyMode
    setOperationState( precitec::interface::NotReadyMode );

	// reset isBackupRequested
	if(m_pContext->getRelatedException() == 512 && m_pContext->getIsBackupRequested())
	{
		m_pContext->setIsBackupRequested(false);
	}

} // CTor



void NotReady::calibrate( unsigned int m_oMethod )
{
	if( m_oMethod >= eHomingY )
	{
		bool oRet = m_pContext->getCalibration().start( m_oMethod );
		if( oRet )
		{
			wmLog( eInfo, "Finished homing successfully.\n" );

			//reset error bit
			m_pContext->getInspectionOut().setSumErrorLatched(false);

			// signal calibration result to fieldbus
			m_pContext->getInspectionOut().setCalibrationFinished( oRet );

            // Change state to operate, if no exception is currently active
			if ( m_pContext->getRelatedException() == eNone ) 
			{
				m_pContext->clearRelatedException();
				m_pContext->changeState(eOperate);
			}

		} else {

			wmLog( eError, "Error while homing...\n" );

			// signal calibration result to fieldbus
			m_pContext->getInspectionOut().setCalibrationFinished( oRet );
		}
	}

} // calibrate



void NotReady::signalNotReady()
{
	//signal possible "NotReady transition" triggered by startBackup request
	m_pContext->getSystemStatus().operationState( precitec::interface::NotReadyMode );
} // signalNotReady



void NotReady::quitSystemFault()
{
	wmLog( eInfo, "Ack. system fault.\n" );
	// set SystemReady and error field hw signal
	m_pContext->clearRelatedException();

	wmLog( eDebug, "NotReady::quitSystemFault()!\n" );

	// change state to operate
	m_pContext->changeState(eOperate);	// warning: 'this' will be deleted.

} // quitSystemFault

void NotReady::emergencyStop()
{
	wmLog( eDebug, "State[NotReady]::emergencyStop!\n");
	m_pContext->changeState(eEmergencyStop);	// warning: 'this' will be deleted.
} // emergencyStop


} // namespace workflow
} // namespace precitec
