/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Stefan Birmanns (SB)
 *  @date       2012
 */

#include "workflow/dbNotificationServer.h"

using Poco::UUID;
namespace precitec{
using namespace interface;
namespace workflow{



DbNotificationServer::DbNotificationServer(SmStateContext stateContext) :
	stateContext_(stateContext)
{
} // CTor



DbNotificationServer::~DbNotificationServer()
{
} // DTor



void DbNotificationServer::setupProduct(const Poco::UUID& p_rProductID)
{
    DbChange oChange( eProduct );
    oChange.setProductID( p_rProductID );

    // mark the db as altered
    stateContext_->dbAltered( oChange );

    // if we are in eOperate we can apply the changes immediately, if not, the changes are applied by the state machine later ...
    if ( stateContext_->currentState() == eOperate || stateContext_->currentState() == eNotReady )
    	stateContext_->dbCheck();
    else if (stateContext_->currentState() == eLiveMode && stateContext_->getProductID() == p_rProductID)
    {
        stateContext_->updateLiveMode();
    }
    else
    	wmLog( eWarning, "DbNotificationServer::setupProduct() - Changes to product definition will be applied later, when state returns to ready.\n");
} // setupProduct

void DbNotificationServer::setupMeasureTask(const UUID& p_rMeasureTaskID)
{
    DbChange oChange( eMeasureTask );
    oChange.setMeasureTaskID( p_rMeasureTaskID );

    // mark the db as altered
    stateContext_->dbAltered( oChange );

    // if we are in eOperate we can apply the changes immediately, if not, the changes are applied by the state machine later ...
    if ( stateContext_->currentState() == eOperate || stateContext_->currentState() == eNotReady )
    	stateContext_->dbCheck();
    else
    	wmLog( eWarning, "DbNotificationServer::setupProduct() - Changes to measuretask definition will be applied later, when state returns to ready.\n");

} // setupMeasureTask

void DbNotificationServer::setupFilterParameter(const UUID& p_rMeasureTaskID, const UUID& p_rFilterID)
{

    if (stateContext_->isSimulationStation())
    {
        try
        {
            stateContext_->inspectManager().changeFilterParameter(p_rMeasureTaskID, p_rFilterID);
        }
        catch (...)
        {
            logExcpetion(__FUNCTION__, std::current_exception());
        }
    } else
    {
        DbChange oChange( eFilterParameter );
        oChange.setMeasureTaskID( p_rMeasureTaskID );
        oChange.setFilterID( p_rFilterID );
        // mark the db as altered
        stateContext_->dbAltered( oChange );
        // if we are in eOperate we can apply the changes immediately, if not, the changes are applied by the state machine later ...
        if ( stateContext_->currentState() == eOperate || stateContext_->currentState() == eNotReady )
            stateContext_->dbCheck();
        else
            wmLog( eWarning, "DbNotificationServer::setupProduct() - Changes to filter parameters will be applied later, when state returns to ready.\n");
    }
} // setupFilterParameter



void DbNotificationServer::setupHardwareParameter(const UUID& hwParameterSatzID, const Key key)
{
	wmLog( eDebug, "DbNotificationServer::setupHardwareParameter() - HW-Parametersatz: %s Key: %s\n",hwParameterSatzID.toString().c_str(), key.c_str());

	// this is a special function only called during live mode, therefore we can / have to apply the updated hw parameters directly and do not have to cache them and apply them later as in the other functions here ...
	stateContext_->inspectManager().updateHwParameter( hwParameterSatzID, key );

} // setupHardwareParameter


void DbNotificationServer::resetCalibration(const int sensorId)
{
	if (stateContext_->isSimulationStation())
	{
		wmLog( eDebug, "DbNotificationServer::resetCalibration(%d) \n",sensorId);
		stateContext_->simulationCalibrationAltered(sensorId);
		//the calibration will be actually reset when the state is eOperate 
	}
	else
	{
		wmLog(eWarning, "DbNotificationServer::resetCalibration can only be used in simulation \n");
	}
    
}

}	// namespace workflow
}	// namespace precitec
