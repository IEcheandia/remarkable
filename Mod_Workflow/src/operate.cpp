/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Ralph Kirchner, Wolfgang Reichl (WoR), Stefan Birmanns (SB)
 *  @date       2009
 *  @brief      The operate state. In this state the machine is not doing anything, but is ready to switch into the automatic mode where parts are then being inspected.
 */

// WM includes
#include "workflow/stateMachine/operate.h"
#include "workflow/stateMachine/liveMode.h"
#include "workflow/stateMachine/automaticMode.h"
#include "common/product.h"
#include "event/systemStatus.h"
#include "common/defines.h"
#include "system/tools.h"

using namespace Poco;
using Poco::UUID;
namespace precitec {
	using namespace interface;
namespace workflow {


Operate::Operate( StateContext* p_pContext ) : AbstractState( eOperate , p_pContext)
{
	setOperationState( precitec::interface::NormalMode );
	m_pContext->simulationCalibrationCheck(0);
	m_pContext->dbCheck();
	m_pContext->getInspectionOut().setSystemReady(true);
	m_pContext->getInspectionOut().setSystemErrorField(0);
	m_pContext->inspectManager().setState(eOperate);

	wmLog( eInfo, "Bereit für Verarbeitung (Live/Automatic)\n" );
}



void Operate::startLive( const UUID& p_rProductId/*is null on auto run*/, int p_oSeamseries, int p_oSeam)
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



void Operate::startAuto( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo )
{
    m_pContext->setProductType( p_oProductType, p_oProductNr, p_rExtendedProductInfo );

	// get product guid
	const UUID	oProductID 		= m_pContext->getProductID();

	// search product object
	const auto&	rProductList	= m_pContext->getProductList();
	const auto	oItProduct		= std::find_if(std::begin(rProductList), std::end(rProductList),
			[oProductID](const Product& p_rProduct){ return p_rProduct.productID() == oProductID; });
	// if product was found, we can now carry out the product level pre-positioning here.
	// it has to be done here, as it should be within the cycle start handshake, which ends in the automatic state constructor.

	if (oItProduct == std::end(rProductList))
	{
		wmLogTr( eError, "QnxMsg.Workflow.ProductNotFound", "Product %i was not found, please create and activate product!\n", p_oProductType );
		return;
	}

    if (!m_pContext->getNoHWParaAxis())
    {
        const Product& rProduct = *oItProduct;
        if ( rProduct.startPosYAxis() != 0 )
        {
            m_pContext->getWeldHeadMsgProxy().setHeadPos( eAxisY, rProduct.startPosYAxis() );
        }
    }

	// product was found, pre-positioning was successful, now we can activate the product in the inspect manager...
	if ( m_pContext->inspectManager().activate( oProductID, p_oProductNr, p_rExtendedProductInfo ) == false)
	{
		return;	// activate tells reason
	}

	// ok, everything is ready, can now change into automatic mode ...
	m_pContext->changeState(eAutomaticMode);	// warning: 'this' will be deleted.

} // startAuto



void Operate::exit()
{
	const auto pContext	=	m_pContext;	// get member before deleting this
	pContext->changeState(eShutdown);
	pContext->beginTermination();

} // exit



void Operate::calibrate( unsigned int m_oMethod )
{
	wmLog( eDebug, "State[Operate]::calibrate\n");
	const auto pContext	=	m_pContext;	// get member before deleting this
	pContext->changeState(eCalibrate);	// warning: 'this' will be deleted.
	pContext->calibrate( m_oMethod );

} // calibrate



void Operate::signalNotReady()
{
	if(m_pContext->getRelatedException() == 512)
	{
		m_pContext->setIsBackupRequested(true);
		wmLog( eDebug, "State[Operate]::signalNotReady! (Backup request)\n");
	}

	m_pContext->changeState(eNotReady);	// warning: 'this' will be deleted.


} // signalNotReady



void Operate::startProductTeachIn()
{
	m_pContext->changeState(eProductTeachIn);	// warning: 'this' will be deleted.

} // startProductTeachIn

void Operate::emergencyStop()
{
	wmLog( eDebug, "State[Operate]::emergencyStop!\n");
	m_pContext->changeState(eEmergencyStop);	// warning: 'this' will be deleted.
} // emergencyStop


} // namespace workflow
} // namespace precitec
