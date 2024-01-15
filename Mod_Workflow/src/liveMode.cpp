/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR, Stefan Birmanns (SB)
 *  @date			2009
 *  @brief
 */

#include "workflow/stateMachine/liveMode.h"
#include "common/product.h"

namespace precitec {
using namespace interface;
namespace workflow {


LiveMode::LiveMode(StateContext* p_pContext) : AbstractState(eLiveMode, p_pContext)
{
	m_pContext->inspectManager().startLiveMode();
    setOperationState( precitec::interface::LiveMode );
} // CTor



void LiveMode::startAuto( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo )
{
	// stop live mode and change immediately into automatic mode
	const auto pContext	=	m_pContext;
	stopLive();	// warning: 'this' will be deleted.
	pContext->startAuto( p_oProductType, p_oProductNr, p_rExtendedProductInfo );
} // startAuto



void LiveMode::stopLive()
{
	endInspect();
	m_pContext->inspectManager().stopLiveMode();

	if (m_pContext->getRelatedException() == eNone)
	{
		m_pContext->changeState(eOperate);	// warning: 'this' will be deleted.
	}
} // stopLive



void LiveMode::activateSeamSeries( int p_oSeamSeries )
{
	m_pContext->inspectManager().activateSeamSeries(p_oSeamSeries);
} // activateSeamSeries



void LiveMode::beginInspect( int p_oSeam, const std::string &label )
{
	if (!m_pContext->inspectManager().startInspect( m_pContext->getSeamseries(), m_pContext->getSeam(), label ))
		m_pContext->inspectManager().stopInspect();
} // beginInspect



void LiveMode::endInspect()
{
	m_pContext->inspectManager().stopInspect();
} // endInspect



void LiveMode::signalNotReady()
{
	if(m_pContext->getRelatedException() == 512)
	{
		wmLog( eDebug, "State[LiveMode]::signalNotReady! (Backup request)\n");
		m_pContext->setIsBackupRequested(true);
	}
	else
	{
		wmLog( eError, "State[LiveMode]::signalNotReady!\n" );
	}

	endInspect();
	stopLive();
	m_pContext->changeState(eNotReady);	// warning: 'this' will be deleted.

} // signalNotReady

void LiveMode::emergencyStop()
{
	wmLog( eDebug, "State[LiveMode]::emergencyStop!\n");
	endInspect();
	m_pContext->inspectManager().stopLiveMode();
	m_pContext->changeState(eEmergencyStop);	// warning: 'this' will be deleted.
} // emergencyStop

void LiveMode::updateLiveMode()
{
    auto stateContext = m_pContext;
    const auto seamSeries = stateContext->getSeamseries();
    const auto seam = stateContext->getSeam();
    stateContext->changeState(eUpdateLiveMode);
    stateContext->startLive({}, seamSeries, seam);
}

} // namespace workflow
} // namespace precitec
