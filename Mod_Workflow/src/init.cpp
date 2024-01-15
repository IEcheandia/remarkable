/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR, SB
 *  @date			2009
 *  @brief			This class represents the initial state in the workflow statemachine.
 */

#include "../include/workflow/stateMachine/init.h"
#include "system/exception.h"
#include "Poco/BasicEvent.h"
#include "Poco/EventArgs.h"
#include "Poco/Expire.h"
#include "Poco/Delegate.h"
#include "Poco/FunctionDelegate.h"
#include "Poco/Thread.h"
#include "common/systemConfiguration.h"

namespace precitec {
namespace workflow {

const Poco::UUID g_oInspectionID    ( "F42DDE6B-C8FF-4CE5-86DE-1A5CB51D633A" );
const Poco::UUID g_oServiceID       ( "a97a5a4c-dcd0-4a77-b933-9d1e20dbe73c" );

Init::Init( StateContext* p_pContext ) : AbstractState(eInit , p_pContext), m_oWinInitialized(false)
{
} // CTor



void Init::GoToNextState(StateContext* p_pContext)
{
    if (SystemConfiguration::instance().getBool("AxisXEnable", false) ||
        SystemConfiguration::instance().getBool("AxisYEnable", false) ||
        SystemConfiguration::instance().getBool("AxisZEnable", false) ||
        SystemConfiguration::instance().getBool("ZCollimatorEnable", false))
    {
        if(p_pContext->getForceHomingOfAxis() )
        {
            wmLogTr(eError, "QnxMsg.VI.AxisDoHomingFirst1", "Axis have no valid home position !\n");
            wmLogTr(eError, "QnxMsg.VI.AxisDoHomingFirst2", "Please do first homing of axis !\n");
            p_pContext->changeState(eNotReady);	// warning: 'this' will be deleted.
        }
        else
        {
            p_pContext->changeState(eOperate);	// warning: 'this' will be deleted.
        }
    }
    else
    {
        p_pContext->changeState(eOperate);	// warning: 'this' will be deleted.
    }
}

void Init::initialize()
{
	const auto pContext	=	m_pContext;	// get member before deleting this

    if (m_pContext->isSimulationStation())
    {
        std::cout << "initialize simulation linux\n" ;
        m_oWinInitialized = handleCalibrationData();
        wmLog(eDebug, "Simulation init handleCalibrationData returned %s\n", m_oWinInitialized ?  "T": "F");
        if (m_pContext->isSimulationStation() && !m_oWinInitialized)
        {
            //in windows, the calibration is initialized by wmHost in reponse to an event
            std::cout << "initialize win " << std::endl;
            //TODO  notify when calibration is initialized
            m_oWinInitialized = true;
        }
        // receive all products from the Win/DB side
        receiveProducts();
        if ( m_oWinInitialized )
        {
            // now change to operate
            pContext->changeState(eOperate);	// warning: 'this' will be deleted.
        }
    }
    else  //not simulation
    {
        bool calibrationInitialized = handleCalibrationData();
        if (!calibrationInitialized)
        {
            wmLog(eError, "Error in receiving calibration data during initialization\n");
        }

        // receive all products from the Win/DB side
        wmLog(eDebug, "Calling receive products, inspectManager calibration data initialized? %s\n",
            m_pContext->inspectManager().getCalibrationData(math::SensorId::eSensorId0).isInitialized()? "Y":"N"
        );
        receiveProducts();

        //ToDo: check if emergency stop feature is turne on!
        SmpKeyValue emergencySignalEnableKv = m_pContext->getDeviceManager().get(g_oServiceID, "EmergencyStopSignalEnable");
        if(emergencySignalEnableKv && emergencySignalEnableKv->value<bool>())
        {
            SmpKeyValue emergencyStateKv = m_pContext->getDeviceManager().get(g_oInspectionID, "IsInEmergencyStopState");
            if(emergencyStateKv && emergencyStateKv->value<bool>())
            {
                pContext->changeState(eEmergencyStop);	// warning: 'this' will be deleted.
            }
            else
            {
                GoToNextState(pContext);
            }
        }
        else
        {
            GoToNextState(pContext);
        }
    }

} // initialize

bool Init::handleCalibrationData()
{
	std::cout << "Workflow: handle Calibration Data -- getting calib..." << std::endl;

	// Inspect Manager IM - Mod_Analyzer
    if (m_pContext->isSimulationStation())
    {
        wmLog(eInfo, "Initialize calibration for simulation \n");
        std::cout << "Simulation : handle calibration data \n" << std::endl;
    }
    
    //aufruf in die App_Calibration - calibrationServer.h start--> calibrate( p_oMethod ) bzw calibrate(eInitCalibrationData )
    std::cout<<"calibrate Aufruf mit eInitCalibrationData: "<<eInitCalibrationData<<std::endl;
    bool calibrationOk = m_pContext->getCalibration().start(eInitCalibrationData);
    if (!calibrationOk)
    {
        wmLog(eError, "Workflow initialization, no calibration available \n");
    }
    //at this point, I know that App_Calibration has succesfully initialized, I just need to wait for the calibration values
    
    return calibrationOk;

}


void Init::exit()
{
	const auto pContext	=	m_pContext;	// get member before deleting this
	pContext->changeState( eShutdown );	// warning: 'this' will be deleted.
	pContext->beginTermination();

} // exit


void Init::receiveProducts()
{
	// Setup state context.
	m_pContext->getProductList().clear();

	try
	{
		// Produktdaten aus DB laden
		TDb<AbstractInterface>& databaseProxy = m_pContext->getDB();
		Poco::UUID station = m_pContext->getStationID();
		ProductList productList;
		productList = databaseProxy.getProductList( station );
		m_pContext->setProductList( productList );
		m_pContext->clearDefaultProduct();

		wmLog( eDebug, "Station ID:  %s\n", m_pContext->getStationID().toString().c_str() );
		wmLog( eDebug, "NumProducts: %i\n", m_pContext->getProductList().size() );

		// Determine default product
		for(ProductList::iterator it=m_pContext->getProductList().begin(); it!=m_pContext->getProductList().end();++it)
		{
			if (it->defaultProduct())
			{
		 		m_pContext->setDefaultProduct( (*it) );
		 		break;
		 	}
		}
		Product defaultProduct = m_pContext->getDefaultProduct();

		if (defaultProduct.productID() == Poco::UUID())
			wmLog( eWarning, "Warning: No default product found!\n" );

		m_pContext->setProductID(defaultProduct.productID(), 0 );
		std::cout << "Init: Getting product " << defaultProduct.productID().toString() <<" <" << defaultProduct.name() << ">" << std::endl;


		// Inform about the products - changeProduct will then build the filter graphs for all the products that might be inspected in the future ...
		for(ProductList::iterator it=m_pContext->getProductList().begin(); it!=m_pContext->getProductList().end();++it)
		{
			try {
				std::cout << "Init: Getting product " << "<" << (*it).name() << std::endl;
				m_pContext->inspectManager().changeProduct( (*it) );
			}
			catch(...) {
				logExcpetion(__FUNCTION__, std::current_exception());
			} // catch
		}




	}
	catch (const std::exception &ex)
	{
		wmLog( eError, "ERROR: Receiving products failed, reason: %s\n", ex.what() );
		throw;
	}
} // receiveProducts


}	// workflow
}	// precitec
