#pragma once


#include "../../../Interfaces/include/event/dbNotification.interface.h"
//#include "VI_InspectionControl.h"
#include "message/db.proxy.h"


namespace precitec
{

using namespace interface;

namespace ethercat
{
	// Datenbankaenderungen werden ueber diese Schnittstelle gemeldet.
	class  DbNotificationServer : public TDbNotification<AbstractInterface>
	{
		public:
			explicit DbNotificationServer(TDb<MsgProxy>& p_rDbProxy);
			virtual ~DbNotificationServer();

		
			// Die Produktaten wurden geaendert.
			void setupProduct(const Poco::UUID& productID) override;
		
			// Die Messaufgaben wurden geaendert (inkl. Graphen)
			void setupMeasureTask(const Poco::UUID& measureTaskID) override;

			// die Filter Parameter wurden geaendert
			void setupFilterParameter(const Poco::UUID& measureTaskID, const Poco::UUID& filterID) override;

			void setupHardwareParameter(const Poco::UUID& hwParameterSatzID, const Key key) override;

			void resetCalibration(const int sensorId) override;
            
            // Store change in DBChanges buffer
            void dbAltered( DbChange p_change);
            
            // Apply changes into product
            void dbCheck();
            
            // Fetch all product data (product info, seamseries info, seams info, parameters info) from DB to local 
            void fetchAllProductsData();
            
            // Getter Produtc list (vector of products)
            const ProductList* getProducts()
            {
                return &m_productList;
            }
            
            // Getter measuretask list per product (std::map key = product UUID, value =  measure tasks)
            const std::map<Poco::UUID, std::vector<interface::MeasureTask>>* getMeasureTasksPerProductMap ()
            {
                return &m_MeasureTask_per_Product;
            }
            
            // Getter parameter sets (std::map key = parameterSet UUID, value = ParameterSet)
            const paramSet_t* getParameterSet()
            {
                return &m_ParamSet;
            }
            
            const GraphList getGraph (Poco::UUID measureTaskID, Poco::UUID graphID)
            {
                return m_rDbProxy.getGraph(measureTaskID, graphID);
            }

            void setDBProxy(TDb<MsgProxy>& dbProxy)
            {
                std::cout << "DbNotificationServer setDBProxy: &dbProxy: " << &dbProxy << std::endl;
                sleep(5);
                std::cout << "-------------------------" << std::endl;
                try {
                    m_rDbProxy = dbProxy;
                } catch (const std::exception& e) {
                    std::cout << "DbNotificationServer setDBProxy: Exception: " << e.what() << std::endl;
                    sleep(5);
                }
                
            }

		private:
		//	VI_InspectionControl& m_ptrVI_InspectionControl;
            Poco::Mutex		 						m_DbChangesMutex;		///< Protect the db changes array ...
            std::vector< DbChange >					m_DbChanges;			///< We have to see if the db was altered and have to inform other   
            ProductList                             m_productList;
            ParameterList                           m_parameterList;
            std::map<Poco::UUID, std::vector<interface::MeasureTask>> m_MeasureTask_per_Product; //The UUID (key) is the Product ID which the MeasureTask corresponds to
            paramSet_t m_ParamSet;
            TDb<MsgProxy>& m_rDbProxy;

	};

}	// workflow
}	// precitec





