/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Ralph Kirchner, Wolfgang Reichl (WoR), Andreas Beschorner (BA), Stefan Birmanns (SB), Simon Hilsenbeck (HS)
 *  @date       2009
 *  @brief      The context of the state machine. Stores information that is independent of the states, for example the name/ID of the station.
 */

// WM includes
#include "workflow/stateMachine/stateContext.h"
#include "workflow/stateMachine/init.h"
#include "workflow/stateMachine/operate.h"
#include "workflow/stateMachine/automaticMode.h"
#include "workflow/stateMachine/liveMode.h"
#include "workflow/stateMachine/shutdown.h"
#include "workflow/stateMachine/calibrate.h"
#include "workflow/stateMachine/notReady.h"
#include "workflow/stateMachine/productTeachInMode.h"
#include "workflow/stateMachine/emergencyStop.h"
#include "workflow/stateMachine/updateLiveMode.h"
#include "common/systemConfiguration.h"
// Poco includes
#include "Poco/UUID.h"

using Poco::UUID;
namespace precitec {
namespace workflow {

const StateNames StateName;
const Poco::UUID g_oWeldHeadID( "3c57acde-707e-4c7d-a6b5-0e9352568095" );

StateContext::StateContext(
		UUID const& p_oStation,
		TDb<AbstractInterface> &p_pDbProxy,
		analyzer::InspectManager &p_pInspectManager,
		TSystemStatus<AbstractInterface>& p_pSystemStatusProxy,
		TCalibration<AbstractInterface>& p_pCalibrationProxy,
		TInspectionOut<AbstractInterface>& p_pInspectionOutProxy,
		precitec::analyzer::CentralDeviceManager& p_rDeviceManager,
		TProductTeachIn<AbstractInterface>& p_pProductTeachInProxy,
		TWeldHeadMsg<AbstractInterface>& p_pWeldHeadMsgProxy,
		TVideoRecorder<AbstractInterface>& p_rVideoRecorderProxy,
		grabberStatusProxy_t &p_rGabberStatusProxy
	) :	m_oStation( p_oStation ),
	m_oBusy( 0, 2 ),
	m_pDbProxy( &p_pDbProxy ),
	m_pInspectManager( &p_pInspectManager ),
	m_pSystemStatusProxy( &p_pSystemStatusProxy ),
	m_pCalibrationProxy( &p_pCalibrationProxy ),
	m_pInspectionOutProxy( &p_pInspectionOutProxy ),
	m_rDeviceManager( p_rDeviceManager ),
	m_pProductTeachInProxy( &p_pProductTeachInProxy ),
	m_pWeldHeadMsgProxy( &p_pWeldHeadMsgProxy ),
	m_pVideoRecorderProxy ( &p_rVideoRecorderProxy ),
	m_rGabberStatusProxy ( p_rGabberStatusProxy ),
	m_oProductType( 0 ),
	m_oProductNr( 0 ),
	m_oExtendedProductInfo( "no info" ),
	m_oNoHWParaAxis( false),
	m_oSeam( 0 ),
	m_oSeamseries( 0 ),
	m_oRelatedException( 0 ),
	m_oForceHomingOfAxis( true ),
	m_oIsBackupRequested( false ),
	m_isSimulation(false)
{
	reset();
    m_oForceHomingOfAxis = SystemConfiguration::instance().getBool("ForceHomingOfAxis", true);

} // CTor.



StateContext::~StateContext()
{
	m_oBusy.set();	// falls nicht schon sinalisiert

} // DTor.



void StateContext::initialize()
{
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_pCurrentState->initialize();

} //initialize



void StateContext::ready()
{
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_pCurrentState->ready();

} // ready



void StateContext::startLive( const UUID& p_oProductID, int p_oSeamseries, int p_oSeam )
{
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_pCurrentState->startLive( p_oProductID, p_oSeamseries, p_oSeam );

} // startLive



void StateContext::stopLive()
{
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_pCurrentState->stopLive();

} // stopLive



void StateContext::startAuto( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo )
{
	Poco::Mutex::ScopedLock oLock( m_oLock );
	Poco::Mutex::ScopedLock oLockDB( m_oDbChangesMutex );
	m_pCurrentState->startAuto( p_oProductType, p_oProductNr, p_rExtendedProductInfo );

} // startAuto



void StateContext::stopAuto()
{
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_pCurrentState->stopAuto();

} // stopAuto



void StateContext::beginInspect( int p_oSeamNumber, const std::string &label)
{
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_oSeam	= p_oSeamNumber;
	m_pCurrentState->beginInspect( p_oSeamNumber, label );

} // beginInspect



void StateContext::endInspect()
{
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_pCurrentState->endInspect();

} // endInspect



void StateContext::exit()
{
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_pCurrentState->exit();

} // exit



void StateContext::calibrate( unsigned int p_oCalibrationMethod )
{
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_pCurrentState->calibrate( p_oCalibrationMethod );

} // calibrate



void StateContext::startProductTeachIn()
{
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_pCurrentState->startProductTeachIn();

} // startProductTeachIn



void StateContext::abortProductTeachIn()
{
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_pCurrentState->abortProductTeachIn();

} // abortProductTeachIn



void StateContext::seamPreStart( int p_oSeamNumber )
{
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_oSeam	= p_oSeamNumber;
	m_pCurrentState->seamPreStart( p_oSeamNumber );

} // seamPreStart

void StateContext::updateLiveMode()
{
    Poco::Mutex::ScopedLock lock{m_oLock};
    m_pCurrentState->updateLiveMode();
}

void StateContext::signalNotReady( unsigned int p_oErrorCode )
{
	Poco::Mutex::ScopedLock lock( m_oLock );

	m_oRelatedException |= p_oErrorCode;

	if ( m_oRelatedException != eNone )
	{
		// set SystemReady and error field hw signal
		getInspectionOut().setSystemReady( false );
		getInspectionOut().setSystemErrorField( m_oRelatedException );

		wmLog( eInfo, "SignalNotReady called - current error state: %s\n", formatErrorType( LogErrorType(m_oRelatedException) ).c_str() );
		m_pCurrentState->signalNotReady();

	} else {
        //this can happen with wmLog(eFatal) 
        
		// set SystemReady and error field hw signal
		getInspectionOut().setSystemReady( true );
		getInspectionOut().setSystemErrorField( 0 );

		wmLog( eDebug, "SignalNotReady called, but was called without an error state?\n" );
	}

} // signalNotReady



void StateContext::signalReady( unsigned int p_oErrorCode )
{
	Poco::Mutex::ScopedLock oLock( m_oLock );

	wmLog( eInfo, "Signal ready.\n");

	wmLog( eDebug, "Before ackSystemFault: %s\n", formatErrorType( precitec::LogErrorType( p_oErrorCode ) ).c_str() );

	m_oRelatedException ^= p_oErrorCode;

	wmLog( eDebug, "After ackSystemFault: %s\n", formatErrorType( precitec::LogErrorType( m_oRelatedException ) ).c_str() );

	if ( m_oRelatedException == eNone )
	{
		// set SystemReady and error field hw signal
		getInspectionOut().setSystemReady( true );
		getInspectionOut().setSystemErrorField( 0 );

		m_pCurrentState->quitSystemFault();

	} else {

		// set SystemReady and error field hw signal
		getInspectionOut().setSystemReady( false );
		getInspectionOut().setSystemErrorField( m_oRelatedException );

	}

} // signal ready



void StateContext::quitSystemFault()
{
	Poco::Mutex::ScopedLock oLock( m_oLock );
	// inform current state ...
	m_pCurrentState->quitSystemFault();

} // quitSystemFault



int StateContext::getRelatedException()
{
	return m_oRelatedException;

} // getRelatedException


void StateContext::clearRelatedException()
{
	m_oRelatedException = eNone;
}// getRelatedException



UUID StateContext::getStationID() const
{
	return m_oStation;

} // Station



AbstractState* StateContext::createState( State newState )
{
	if (newState == eInit)
		return new Init( this );
	else if (newState == eOperate)
		return new Operate( this );
	else if (newState == eLiveMode)
		return new LiveMode( this );
	else if (newState == eAutomaticMode)
		return new AutomaticMode( this );
	else if (newState == eShutdown)
		return new Shutdown( this );
	else if (newState == eCalibrate)
		return new Calibrate( this );
	else if (newState == eNotReady)
		return new NotReady( this );
	else if (newState == eProductTeachIn)
		return new ProductTeachInMode( this );
	else if (newState == eEmergencyStop)
		return new EmergencyStop( this );
    else if (newState == eUpdateLiveMode)
    {
        return new UpdateLiveMode{this};
    }

	return NULL;

} // createState



void StateContext::reset()
{
	changeState(eInit); 		// Anfang setzen

} // reset



void StateContext::changeState(Poco::SharedPtr<AbstractState> p_pNewState)
{
	wmLog( eDebug, "ChangeState: %s\n", StateName[ p_pNewState->type() ].c_str() );
	m_pCurrentState = p_pNewState;

} // changeState



void StateContext::changeState(State p_oNewState)
{
	// inform analyzer
	inspectManager().setState( p_oNewState );
	// change the state
	changeState( createState( p_oNewState ) );

} // changeState



State StateContext::currentState()
{
	return m_pCurrentState->type();

} // currentState



void StateContext::waitForTermination()
{
	m_oBusy.wait();

} // waitForTermination



void StateContext::beginTermination()
{
	m_oBusy.set();

} // beginTermination



analyzer::InspectManager& StateContext::inspectManager()
{
	return *m_pInspectManager;

} // inspectManager



TDb<AbstractInterface>&	StateContext::getDB()
{
	return *m_pDbProxy;

} // getDb



TSystemStatus<AbstractInterface>& StateContext::getSystemStatus()
{
	return *m_pSystemStatusProxy;

} // getSystemStatus



TCalibration<AbstractInterface>& StateContext::getCalibration()
{
	return *m_pCalibrationProxy;

} // getCalibration



TInspectionOut<AbstractInterface>& StateContext::getInspectionOut()
{
	return *m_pInspectionOutProxy;

} // getInspectionOut



precitec::analyzer::CentralDeviceManager& StateContext::getDeviceManager()
{
	return m_rDeviceManager;

} // getDeviceManager



TProductTeachIn<AbstractInterface>& StateContext::getProductTeachInProxy()
{
	return *m_pProductTeachInProxy;

} // getProductTeachInProxy



TWeldHeadMsg<AbstractInterface>& StateContext::getWeldHeadMsgProxy()
{
	return *m_pWeldHeadMsgProxy;

} // getWeldHeadMsgProxy



TVideoRecorder<AbstractInterface>& StateContext::getVideoRecorderProxy()
{
	return *m_pVideoRecorderProxy;

} // getVideoRecorderProxy

StateContext::grabberStatusProxy_t&	StateContext::getGrabberStatusProxy()
{
	return m_rGabberStatusProxy;
}


void StateContext::setProductType( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo )
{
	m_oProductType = p_oProductType;
	m_oProductNr = p_oProductNr;
	m_oExtendedProductInfo.assign(p_rExtendedProductInfo);

	for(ProductList::iterator it= m_oProducts.begin(); it!=m_oProducts.end();++it)
	{
		if (it->productType() == m_oProductType)
		{
			m_oProductID = it->productID();
			return;
		}
	}

	m_oProductID = UUID::null();

} // setProductID



void StateContext::setProductID( const Poco::UUID& p_oProductID, uint32_t p_oProductType)
{
	m_oProductID = p_oProductID;
	m_oProductType = p_oProductType;

} // setProductID


void StateContext::setNoHWParaAxis( bool p_oNoHWParaAxis)
{
	m_oNoHWParaAxis = p_oNoHWParaAxis;

} // setNoHWParaAxis



UUID StateContext::getProductID()
{
	return m_oProductID;

} // getProductID



uint32_t StateContext::getProductType()
{
	return m_oProductType;

} // getProductType



uint32_t StateContext::getProductNumber()
{
	return m_oProductNr;

} // getProductNumber


const std::string& StateContext::getExtendedProductInfo() const
{
	return m_oExtendedProductInfo;

} // getExtendedProductInfo


bool StateContext::getNoHWParaAxis()
{
	return m_oNoHWParaAxis;

} // getNoHWParaAxis


void StateContext::setSeam( int p_oSeam )
{
	m_oSeam = p_oSeam;

} // setSeam



int StateContext::getSeam() const
{
	return m_oSeam;

} // getSeam



void StateContext::setSeamseries( int p_oSeamseries )
{
	m_oSeamseries = p_oSeamseries;
	m_pCurrentState->activateSeamSeries(p_oSeamseries);

} // setSeamseries



int StateContext::getSeamseries () const
{
	return m_oSeamseries;

} // getSeamseries



void StateContext::clearDefaultProduct( )
{
	m_oDefaultProduct = Product::createNil( m_oStation );

} // clearDefaultProduct



void StateContext::setDefaultProduct( Product const& m_rDefaultProduct )
{
	m_oDefaultProduct = m_rDefaultProduct;

} // setDefaultProduct



Product StateContext::getDefaultProduct() const
{
	return m_oDefaultProduct;

} // defaultProduct



bool StateContext::getForceHomingOfAxis()
{
	return m_oForceHomingOfAxis;

} // getForceHomingOfAxis



ProductList& StateContext::getProductList()
{
	return m_oProducts;

} // getProductList



void StateContext::setProductList( ProductList& p_rProducts )
{
	m_oProducts = p_rProducts;

} // setProductList



void StateContext::dbAltered( DbChange p_oChange )
{
	Poco::Mutex::ScopedLock lock( m_oDbChangesMutex );

    auto check = [&p_oChange] (const auto& change)
    {
        return change.getStatus() == eProduct && change.getProductID() == p_oChange.getProductID();
    };

    if (p_oChange.getStatus() == eProduct && std::any_of(m_oDbChanges.begin(), m_oDbChanges.end(), check))
    {
        return;
    }
	m_oDbChanges.push_back( p_oChange );

} // dbAltered


bool StateContext::getIsBackupRequested()
{
	return m_oIsBackupRequested;
} //getIsBackupRequested


void StateContext::setIsBackupRequested(bool p_oRequest)
{
	m_oIsBackupRequested = p_oRequest;
} //seIsBackupRequested



void StateContext::dbCheck()
{
    Poco::Mutex::ScopedLock stateContextLock( m_oLock );
	Poco::Mutex::ScopedLock lock( m_oDbChangesMutex );

	// Are there any DB changes that were not applied yet?
	if ( m_oDbChanges.size() != 0 )
	{
		ProductList oProducts = getDB().getProductList( getStationID() );
		setProductList( oProducts );
	}
	else
		return;

	// OK, then lets apply the changes ...
	for ( auto oIter = m_oDbChanges.begin(); oIter != m_oDbChanges.end(); ++oIter )
	{
		// A product definition has changed.
		if ( oIter->getStatus() == eProduct )
		{
			for( ProductList::iterator oIterProd = m_oProducts.begin(); oIterProd!=m_oProducts.end(); ++oIterProd )
			{
				// Search for product ID. Attention: Only activated products are send to the QNX station, so it could be that the DB has changed
				// the definition of a product, we cannot update it here, as it is not activated and therefore not found ...
				if ( oIterProd->productID() == oIter->getProductID() )
				{
					try {
						inspectManager().changeProduct( *oIterProd );
					} // try
					catch(...) {
						logExcpetion(__FUNCTION__, std::current_exception());
					} // catch		
				}
			}

		} // if (eProduct)

		// A measureTask definition has changed
		if ( oIter->getStatus() == eMeasureTask )
		{
			wmLog( eError, "State 'eMeasureTask' not implemented.\n" );
		} // if (eMeasureTask)

		// A filter parameter has changed
		if ( oIter->getStatus() == eFilterParameter )
		{
            try {
			    inspectManager().changeFilterParameter( oIter->getMeasureTaskID(), oIter->getFilterID() );
            } // try
			catch(...) {
				logExcpetion(__FUNCTION__, std::current_exception());
			} // catch
		} // if (eFilterParameter)

	} // for

	// Clear the array
	m_oDbChanges.clear();

} // dbCheck



void StateContext::enableLineLasers( bool p_oEnable )
{
    if (isSimulationStation())
    {
        return;
    }

	SmpKeyValue pEnable = SmpKeyValue(new TKeyValue<bool>( "LineLaser1OnOff", p_oEnable ) );
	m_rDeviceManager.force( g_oWeldHeadID, pEnable );
	pEnable = SmpKeyValue(new TKeyValue<bool>( "LineLaser2OnOff", p_oEnable ) );
	m_rDeviceManager.force( g_oWeldHeadID, pEnable );
	pEnable = SmpKeyValue(new TKeyValue<bool>( "FieldLight1OnOff", p_oEnable ) );
	m_rDeviceManager.force( g_oWeldHeadID, pEnable );

	if ( p_oEnable )
	{
		// The default line laser intensity is taken from the Connect.config file, and is used for the calibration and the normal inspection.
		// Only if a hardware parameter set is used, the line laser intensity could be different ...
		wmLog( eDebug, "LineLaser1Intensity  - %d\n", g_oLineLaser1DefaultIntensity );
		wmLog( eDebug, "LineLaser2Intensity  - %d\n", g_oLineLaser2DefaultIntensity );
		wmLog( eDebug, "FieldLight1Intensity - %d\n", g_oFieldLight1DefaultIntensity );

		SmpKeyValue pIntensity = SmpKeyValue(new TKeyValue<int>( "LineLaser1Intensity", (int)( g_oLineLaser1DefaultIntensity ), 0, 100, 90 ) );
		m_rDeviceManager.force( g_oWeldHeadID, pIntensity );
		pIntensity = SmpKeyValue(new TKeyValue<int>( "LineLaser2Intensity", (int)( g_oLineLaser2DefaultIntensity ), 0, 100, 90 ) );
		m_rDeviceManager.force( g_oWeldHeadID, pIntensity );
		pIntensity = SmpKeyValue(new TKeyValue<int>( "FieldLight1Intensity", (int)( g_oFieldLight1DefaultIntensity ), 0, 100, 40 ) );
		m_rDeviceManager.force( g_oWeldHeadID, pIntensity );
	}
} // enableLineLaser1

void StateContext::emergencyStop(){
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_pCurrentState->emergencyStop();
}

void StateContext::resetEmergencyStop(){
	Poco::Mutex::ScopedLock lock( m_oLock );
	m_pCurrentState->resetEmergencyStop();
}

bool StateContext::isSimulationStation() const
{
    return m_isSimulation;
}

void StateContext::setSimulationStation(bool set)
{
    m_isSimulation = set;
}

void StateContext::simulationCalibrationAltered(const int sensorId)
{
	if (!m_isSimulation)
	{
		wmLog(eError, "Attempt to change simulation calibration outside simulation\n");
		return;
	}
	if (sensorId != 0)
	{
		wmLog(eWarning, "Unsupported sensor id %d \n", sensorId);
		return;
	}
	m_isSimulationCalibrationAltered = true;
	if ( currentState() != eOperate )
    {
		wmLog(eWarning, "Changes to calibration will be applied later, when state returns to ready.\n");
	}
}

void StateContext::simulationCalibrationCheck(const int sensorId)
{
	if (!m_isSimulation)
	{
		return;
	}
	
	if (!m_isSimulationCalibrationAltered)
	{
		return;
	}
	wmLog(eWarning, "Calibration will be reinitialized\n");
	m_pInspectManager->resetCalibration(sensorId);
	bool oRet = getCalibration().start( eInitCalibrationData );
	assert(oRet == m_pInspectManager->getCalibrationData(sensorId).hasData());

	//check if the calibration was successful
	if (oRet)
	{
		m_isSimulationCalibrationAltered = false;
		wmLog(eInfo, "Calibration successfully reinitialized \n");
	}
	else
	{
		wmLog(eWarning,"Calibration reinitialization error (called in state %d)\n", currentState() );
		
	}
	assert(!m_isSimulationCalibrationAltered);
}

void StateContext::sendOperationState()
{
    Poco::Mutex::ScopedLock lock( m_oLock );
    m_pCurrentState->sendOperationState();
}

void StateContext::triggerSingle(const interface::TriggerContext &context)
{
    inspectManager().triggerSingle(context);
}

} // namespace workflow
} // namespace precitec
