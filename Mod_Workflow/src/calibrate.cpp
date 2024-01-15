/**
 * @file
 * @copyright	Precitec Vision GmbH & Co. KG
 * @author		Stefan Birmanns (SB)
 * @date		2011
 * @brief		This class represents the calibration state in the workflow statemachine.
 */

#include "../include/workflow/stateMachine/calibrate.h"
#include "system/tools.h"

namespace precitec {
namespace workflow {


Calibrate::Calibrate( StateContext* p_pContext ) : AbstractState( eCalibrate , p_pContext)
{
    setOperationState( precitec::interface::CalibrationMode );

} // CTor.



void Calibrate::calibrate( unsigned int p_oMethod )
{
	bool oNeedsLineLasers = false;
	// turn line laser on
	if ( p_oMethod < eOCT_LineCalibration && p_oMethod != eInitCalibrationData && p_oMethod != eBlackLevelOffset )
	{
		oNeedsLineLasers = true;
		m_pContext->enableLineLasers( true );
	}

	// carry out the actual calibration
	std::cout<<"WF start calibration 1 "<<std::endl;
	bool oRet = m_pContext->getCalibration().start( p_oMethod );
	std::cout<<"WF ended calibration 1 with "<<oRet<<std::endl;

	if ( !oRet )
    {
		wmLog( eDebug, "Calibration %d was not successful ...\n", p_oMethod );
    }
	m_pContext->getInspectionOut().setCalibrationFinished( oRet );

	// turn line laser off
	if ( oNeedsLineLasers )
	{
		m_pContext->enableLineLasers( false );
	}

	//shouldn't need to update calibration data file, calibrationmanager already sends the signal for it
	switch(p_oMethod)
	{
		case eInitCalibrationData:
		case eCalibrateOsIbLine0:
		case eCalibrateOsIbLineTCP:
		case eCalibrateOsIbLine2:
		case eCalibGridAngle:
        case eCalibrateLED:
			//m_pContext->inspectManager().reloadCalibData(0, false);  // todo: for all optical systems!
			break;
		default:
			//the other kind of calibration don't need to reload the calibration data file
			break;
	}

	// change state back to operate
	if ( !oRet )
		wmLogTr(eError, "QnxMsg.Calib.CalibFail", "Error getting calibration data!\n");

	if(m_pContext->getIsBackupRequested())
	{
		m_pContext->changeState(eNotReady);	// warning: 'this' will be deleted.
	}
	else
	{
		m_pContext->changeState(eOperate);	// warning: 'this' will be deleted.
	}
} // calibrate



void Calibrate::signalNotReady()
{
	if(m_pContext->getRelatedException() == 512)
	{
		m_pContext->setIsBackupRequested(true);
		wmLog( eDebug, "State[Calibrate]::signalNotReady! (Backup request)\n");
	}
	else
	{
		wmLog( eDebug, "State[Calibrate]::signalNotReady!\n" );
		m_pContext->changeState(eNotReady);	// warning: 'this' will be deleted.
	}
} // signalNotReady

void Calibrate::emergencyStop()
{
	wmLog( eDebug, "State[Calibrate]::emergencyStop!\n");
	m_pContext->changeState(eEmergencyStop);	// warning: 'this' will be deleted.
} // emergencyStop


} // namespace workflow
} // namespace precitec
