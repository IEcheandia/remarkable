/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Stefan Birmanns (SB)
 *  @date       2012
 */


#include "../include/viInspectionControl/dbNotificationServer.h"
#include "../include/viInspectionControl/VI_InspectionControl.h"
#include "common/connectionConfiguration.h"

using Poco::UUID;
namespace precitec
{

using namespace interface;

namespace ethercat{


DbNotificationServer::DbNotificationServer(TDb<MsgProxy>& p_rDbProxy) :
	m_rDbProxy(p_rDbProxy)
{
    std::cout << "DBProxy ptr: "  << p_rDbProxy << std::endl;
} // CTor

DbNotificationServer::~DbNotificationServer()
{
}


void DbNotificationServer::setupProduct(const Poco::UUID& p_rProductID)
{
    
    DbChange oChange( eProduct );
    oChange.setProductID( p_rProductID );
 //   m_ptrVI_InspectionControl.setupProduct(p_rProductID);
    std::cout << "DBNotification server product changed" << std::endl;
    
    // mark the db as altered
    dbAltered( oChange );
    // apply changes on products, measuretasks and parameeters
    dbCheck();
   
} // setupProduct

void DbNotificationServer::dbAltered( DbChange p_change)
{
    Poco::Mutex::ScopedLock lock( m_DbChangesMutex );
    auto check = [&p_change] (const auto& change)
    {        
        return change.getStatus() == eProduct && change.getProductID() == p_change.getProductID();
    };  
    
    if (p_change.getStatus() == eProduct && std::any_of(m_DbChanges.begin(), m_DbChanges.end(), check))
    {       
        return;
    }
	m_DbChanges.push_back( p_change );
}

void DbNotificationServer::dbCheck()
{
 
	Poco::Mutex::ScopedLock lock( m_DbChangesMutex );
    Poco::UUID oStationUUID;  
    std::cout << "Antes de leer desde DB, hay product? " << m_productList.size() << std::endl;

	// Are there any DB changes that were not applied yet?
	if ( m_DbChanges.size() != 0 )
	{
        oStationUUID = Poco::UUID( ConnectionConfiguration::instance().getString("Station.UUID", Poco::UUID::null().toString() ) );
		m_productList = m_rDbProxy.getProductList( oStationUUID );             
	}
	else
		return;
    
    if(m_productList.size() != 0)
    {
        std::cout << "en DBcheck hay product list, size: " << m_productList.size() << std::endl;
    }

	// OK, then lets apply the changes ...
	for ( auto oIter = m_DbChanges.begin(); oIter != m_DbChanges.end(); ++oIter )
	{
		// A product definition has changed.
		if ( oIter->getStatus() == eProduct )
		{           
            auto oIterProd = std::find_if(m_productList.begin(), m_productList.end(), [oIter](const auto& product) 
            {
                    return product.productID() == oIter->getProductID();
            });           
            
            if(oIterProd != m_productList.end())
            {
#if DEBUG_SCANMASTER
               wmLog(eDebug, "DBCheck: productID:   %s\n", oIterProd->productID().toString().c_str());
#endif
               wmLog(eDebug, "DBCheck: productType: %d, name: %s\n", oIterProd->productType(), oIterProd->name().c_str());
               Poco::UUID oProductID = oIterProd->productID();

               // get MeasureTaskList info for active product              
               m_MeasureTask_per_Product [oProductID] = m_rDbProxy.getMeasureTasks(oStationUUID, oProductID);                    
               wmLog(eDebug, "DBCheck: getMeasureTasks returns m_oSM_MeasureTasks.size() <%d>\n", m_MeasureTask_per_Product [oProductID].size());
                    
                for (MeasureTaskList::iterator oIterMeasTask = m_MeasureTask_per_Product [oIterProd->productID()].begin(); oIterMeasTask!=m_MeasureTask_per_Product [oIterProd->productID()].end(); ++oIterMeasTask)
                {
                    if (oIterMeasTask->level() == 1) // level: seam
                    {
                        Poco::UUID hwParametersUUID = oIterMeasTask->hwParametersatzID();
                        if (!hwParametersUUID.isNull())
                        {
                            m_ParamSet[hwParametersUUID] = m_rDbProxy.getHardwareParameterSatz(hwParametersUUID);                       
                        }
                    } // if it is a seam
                }	// end for IterMeasTask            
             } // if product found
		} // if (eProduct)
	} // for

	// Clear the array
	m_DbChanges.clear();

} // dbCheck

void DbNotificationServer::fetchAllProductsData()
{
    wmLog(eDebug, "Start fetching all product data \n");
    std::cout << "Start fetching Product data from DataBase..." << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();    
    sleep(20);
  
 //   m_productList.clear();
    
    Poco::UUID stationUUID = Poco::UUID( ConnectionConfiguration::instance().getString("Station.UUID", Poco::UUID::null().toString() ) );
	m_productList = m_rDbProxy.getProductList( stationUUID );
   std::cout << "DbNotificationServer::fetchAllProductsData(), Product list size: " << m_productList.size() << std::endl;
   sleep(2);
    for( ProductList::iterator oIterProd = m_productList.begin(); oIterProd!=m_productList.end(); ++oIterProd )
    {
        auto o_measureTaskList = m_rDbProxy.getMeasureTasks(stationUUID,   oIterProd->productID());       
        m_MeasureTask_per_Product [oIterProd->productID()] = o_measureTaskList;
        
    //    std::cout << "Product: " << oIterProd->productType() << std::endl;
        for(auto& task : o_measureTaskList)
        {
            if(task.level() == 1) // seam
            {
                Poco::UUID hwParametersUUID = task.hwParametersatzID();
                if (!hwParametersUUID.isNull())
                {
                    auto oParamList = m_rDbProxy.getHardwareParameterSatz(hwParametersUUID);                   
                    m_ParamSet[hwParametersUUID] = oParamList;
                }
            }
        }
       
    }  
    
  
  auto endTime = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
  
    if(m_productList.empty() )
    {
         wmLog(eDebug, "Something went wrong with fetching, products empty - Fetching Execution time: %d ms\n", static_cast<int>(duration.count()));
    }
    else
    {        
         wmLog(eDebug, "Finishing fetching data from DB - Fetching Execution time: %d ms\n", static_cast<int>(duration.count()));
    }
    
}



void DbNotificationServer::setupMeasureTask(const UUID& p_rMeasureTaskID)
{
 
} // setupMeasureTask

void DbNotificationServer::setupFilterParameter(const UUID& p_rMeasureTaskID, const UUID& p_rFilterID)
{

} // setupFilterParameter



void DbNotificationServer::setupHardwareParameter(const UUID& hwParameterSatzID, const Key key)
{

} // setupHardwareParameter


void DbNotificationServer::resetCalibration(const int sensorId)
{
 
}

}	// namespace ethercat
}	// namespace precitec

