#include "../include/workflow/stateMachine/productTeachInMode.h"

namespace precitec {
using namespace interface;
namespace workflow {


void ProductTeachInMode::startAuto( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo )
{
	// store product information in context
	m_pContext->setProductType( p_oProductType, p_oProductNr, p_rExtendedProductInfo );

	// inform analyzer that we start an automatic mode
	m_pContext->inspectManager().setState(eAutomaticMode);
	m_pContext->getProductTeachInProxy().startAutomatic( m_pContext->getProductType() );
} // startAuto



void ProductTeachInMode::stopAuto()
{
	m_pContext->getProductTeachInProxy().stopAutomatic();

	if(m_pContext->getIsBackupRequested() && m_pContext->getRelatedException() == 512)
	{
		m_pContext->changeState(eNotReady);	// warning: 'this' will be deleted.
	}
	else
	{
		m_pContext->changeState(eOperate);	// warning: 'this' will be deleted.
	}

} // stopAuto



void ProductTeachInMode::activateSeamSeries( int p_oSeamSeries )
{
	m_pContext->setSeamseries( p_oSeamSeries );
	m_pContext->inspectManager().activateSeamSeries(p_oSeamSeries);
} // activateSeamSeries



void ProductTeachInMode::beginInspect( int p_oSeam, const std::string &label )
{
	m_pContext->setSeam( p_oSeam );
	stopWatch_.start();
	m_pContext->getProductTeachInProxy().start(m_pContext->getSeamseries(),m_pContext->getSeam());

} // beginInspect



void ProductTeachInMode::endInspect()
{
	stopWatch_.stop();
	m_pContext->getProductTeachInProxy().end(stopWatch_.us());

} // endInspect



void ProductTeachInMode::signalNotReady()
{
	if(m_pContext->getRelatedException() == 512)
	{
		m_pContext->setIsBackupRequested(true);
		wmLog( eDebug, "State[ProductTeachInMode]::signalNotReady! (Backup request)\n");
	}
	else
	{
		wmLog( eInfo, "State[ProductTeachInMode]::signalNotReady!\n");
		m_pContext->changeState(eNotReady);
	}

} // signalNotReady



void ProductTeachInMode::abortProductTeachIn()
{
	m_pContext->changeState(eOperate);	// warning: 'this' will be deleted.

} // abortProductTeachIn

void ProductTeachInMode::emergencyStop()
{
	wmLog( eDebug, "State[ProductTeachInMode]::emergencyStop!\n");

	m_pContext->changeState(eEmergencyStop);	// warning: 'this' will be deleted.
} // emergencyStop


} // workflow
} // precitec
