
// WM includes
#include "workflow/stateMachine/updateLiveMode.h"
#include "workflow/stateMachine/stateContext.h"

namespace precitec
{
namespace workflow
{

UpdateLiveMode::UpdateLiveMode(StateContext* p_pContext)
    : AbstractState(eUpdateLiveMode , p_pContext)
{
    // don't send a state change to Gui, to make it look like we stay in live mode all the time
    m_pContext->inspectManager().stopInspect();
    m_pContext->inspectManager().stopLiveMode();
    m_pContext->dbCheck();
}

// copied from Operate
void UpdateLiveMode::startLive( const Poco::UUID& p_rProductId/*is null on auto run*/, int p_oSeamseries, int p_oSeam)
{
	// store current product ID and seamseries, seam information in context.
	m_pContext->setProductID( m_pContext->getDefaultProduct().productID(), 0/*is live mode product type*/ );

	const auto oProductActivated	=	m_pContext->inspectManager().activate( m_pContext->getProductID(), m_pContext->getProductNumber() );

	if (oProductActivated == false)
	{
		return;
	}

	const auto pContext	=	m_pContext;	// get member before deleting this

	pContext->changeState( eLiveMode );	// warning: 'this' will be deleted.
	pContext->setSeamseries(p_oSeamseries);
	pContext->beginInspect(p_oSeam);
} // startLive

}
}
