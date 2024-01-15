/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Alexander Egger (EA)
 *  @date       2019
 */

#include <common/connectionConfiguration.h>

#include "TCPCommunication/DbNotificationServer.h"

namespace precitec
{

using namespace interface;

namespace tcpcommunication
{

using Poco::UUID;

DbNotificationServer::DbNotificationServer(TCPCommunication& p_rTCPCommunication) :
        m_rTCPCommunication(p_rTCPCommunication)
{
} // CTor


DbNotificationServer::~DbNotificationServer()
{
} // DTor

void DbNotificationServer::setupProduct(const Poco::UUID& p_rProductID)
{
wmLog(eDebug, "DbNotificationServer::setupProduct\n");
    m_rTCPCommunication.setupProduct(p_rProductID);

#if 0
    DbChange oChange( eProduct );
    oChange.setProductID( p_rProductID );

    // mark the db as altered
    stateContext_->dbAltered( oChange );

    // if we are in eOperate we can apply the changes immediately, if not, the changes are applied by the state machine later ...
    if ( stateContext_->currentState() == eOperate || stateContext_->currentState() == eNotReady )
        stateContext_->dbCheck();
    else
        wmLog( eWarning, "DbNotificationServer::setupProduct() - Changes to product definition will be applied later, when state returns to ready.\n");
#endif
} // setupProduct

void DbNotificationServer::setupMeasureTask(const UUID& p_rMeasureTaskID)
{
wmLog(eDebug, "DbNotificationServer::setupMeasureTask\n");
#if 0
    DbChange oChange( eMeasureTask );
    oChange.setMeasureTaskID( p_rMeasureTaskID );

    // mark the db as altered
    stateContext_->dbAltered( oChange );

    // if we are in eOperate we can apply the changes immediately, if not, the changes are applied by the state machine later ...
    if ( stateContext_->currentState() == eOperate || stateContext_->currentState() == eNotReady )
        stateContext_->dbCheck();
    else
        wmLog( eWarning, "DbNotificationServer::setupProduct() - Changes to measuretask definition will be applied later, when state returns to ready.\n");
#endif
} // setupMeasureTask

void DbNotificationServer::setupFilterParameter(const UUID& p_rMeasureTaskID, const UUID& p_rFilterID)
{
wmLog(eDebug, "DbNotificationServer::setupFilterParameter\n");
#if 0
    DbChange oChange( eFilterParameter );
    oChange.setMeasureTaskID( p_rMeasureTaskID );
    oChange.setFilterID( p_rFilterID );

    // mark the db as altered
    stateContext_->dbAltered( oChange );
    if (stateContext_->isSimulationStation())
    {
        stateContext_->dbCheck();
    } else
    {
        // if we are in eOperate we can apply the changes immediately, if not, the changes are applied by the state machine later ...
        if ( stateContext_->currentState() == eOperate || stateContext_->currentState() == eNotReady )
            stateContext_->dbCheck();
        else
            wmLog( eWarning, "DbNotificationServer::setupProduct() - Changes to filter parameters will be applied later, when state returns to ready.\n");
    }
#endif
} // setupFilterParameter

void DbNotificationServer::setupHardwareParameter(const UUID& hwParameterSatzID, const Key key)
{
wmLog(eDebug, "DbNotificationServer::setupHardwareParameter\n");
#if 0
    wmLog( eDebug, "DbNotificationServer::setupHardwareParameter() - HW-Parametersatz: %s Key: %s\n",hwParameterSatzID.toString().c_str(), key.c_str());

    // this is a special function only called during live mode, therefore we can / have to apply the updated hw parameters directly and do not have to cache them and apply them later as in the other functions here ...
    stateContext_->inspectManager().updateHwParameter( hwParameterSatzID, key );
#endif
} // setupHardwareParameter

void DbNotificationServer::resetCalibration(const int sensorId)
{
wmLog(eDebug, "DbNotificationServer::resetCalibration\n");
#if 0
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
#endif
}

} // namespace tcpcommunication
} // namespace precitec

