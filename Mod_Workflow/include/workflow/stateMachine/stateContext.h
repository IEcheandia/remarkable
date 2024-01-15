/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Ralph Kirchner, Wolfgang Reichl (WoR), Andreas Beschorner (BA), Stefan Birmanns (SB), Simon Hilsenbeck (HS)
 *  @date       2009
 *  @brief      The context of the state machine. Stores information that is independent of the states, for example the name/ID of the station, etc.
 */
#ifndef STATECONTEXT_H_
#define STATECONTEXT_H_

// clib includes
#include <string>
#include <iostream>
// Poco includes
#include <Poco/SharedPtr.h>
#include <Poco/Mutex.h>
#include <Poco/Semaphore.h>
#include <Poco/UUID.h>
// WM includes
#include "Mod_Workflow.h"
#include "abstractState.h"
#include "workflow/productListProvider.h"
#include "message/db.interface.h"
#include "event/dbNotification.interface.h"
#include "event/systemStatus.interface.h"
#include "message/calibration.interface.h"
#include "event/inspectionOut.interface.h"
#include "event/inspection.interface.h"
#include "event/productTeachIn.interface.h"
#include "message/weldHead.interface.h"
#include "event/videoRecorder.interface.h"
#include "common/connectionConfiguration.h"
#include "common/product.h"
#include "analyzer/centralDeviceManager.h"
#include "analyzer/inspectManager.h"
#include "message/grabberStatus.proxy.h"

namespace precitec {

using namespace interface;
namespace workflow {

/**
 * @ingroup Workflow
 * @brief 	The context of the state machine. Stores information that is independent of the states, for example the name/ID of the station, etc.
 */
class MOD_WORKFLOW_API StateContext : public ProductListProvider
{
	friend class Init;
	friend class Operate;
	friend class AutomaticMode;
	friend class LiveMode;
	friend class Shutdown;
	friend class Calibrate;
	friend class NotReady;
	friend class ProductTeachInMode;
	friend class EmergencyStop;
    friend class UpdateLiveMode;

	public:

		typedef interface::TGrabberStatus<interface::AbstractInterface>				grabberStatusProxy_t;

		/**
		 * @brief CTor.
		 */
		StateContext(
				Poco::UUID const& station,
				TDb<AbstractInterface>& dbProxy,
				analyzer::InspectManager& p_rInspectManager,
				TSystemStatus<AbstractInterface>& systemStatusProxy,
				TCalibration<AbstractInterface>& calibrationProxy,
				TInspectionOut<AbstractInterface>& inspectionOutProxy,
				precitec::analyzer::CentralDeviceManager& m_rDeviceManager,
				TProductTeachIn<AbstractInterface>& productTeachInProxy,
				TWeldHeadMsg<AbstractInterface>& weldHeadMsgProxy,
				TVideoRecorder<AbstractInterface>& p_rVideoRecorderProxy,
				grabberStatusProxy_t &p_rGabberStatusProxy );
		/**
		 * @brief DTor.
		 */
		virtual ~StateContext();

		/**
		 * @name Transitions
		 * @brief Transitions into new states - triggered from inspection and inspectionCmd interface.
		 */
		//@{

		void initialize();
		void ready();
		void startLive( const Poco::UUID& productId, int seamseries, int seam );
		void stopLive();
		/// InspectionCmd
		void startAuto( uint32_t p_oProductType, uint32_t p_oProductNumber, const std::string& p_rExtendedProductInfo ) override;
		/// InspectionCmd
		void stopAuto() override;
		void beginInspect( int p_oSeamNumber, const std::string &label = {} ) override;
		void endInspect() override;
		void exit();
		/// InspectionCmd
		void calibrate(unsigned int p_oCalibrationMethod);
		/// InspectionCmd
		void startProductTeachIn();
		/// InspectionCmd
		void abortProductTeachIn();
		void seamPreStart( int p_oSeamNumber );
        void updateLiveMode();

		/**
		 * @brief Signal that a component (camera, grabber, etc.) is not ready.
		 *
		 * The function is called by the InspectionCmd interface.
		 *
		 * @param p_oErrorCode ErrorCode as defined in module/moduleLogger.h
		 */
		void signalNotReady( unsigned int p_oErrorCode );

		/**
		 * @brief Signal that a component is ready again.
		 *
		 * There are two ways to handle a system fault. Ideally the component that is responsible for the system fault (NotReady state in the state machine) gets repaired and
		 * signals by itself that it is ready again by calling the signalReady(). In this case, reseting the system NotReady-state is done in a controlled way, only the
		 * state of this component is affected, if another component was also faulty, the system as a whole will still not be ready to inspect parts again.
		 *
		 * This controlled way of doing things can also be problematic, for example if components have no way of telling if they are still broken or not. In this case there has to be a way
		 * for the user to try to reset the system fault manually and just to hope for best in another automatic cycle. If the components are still broken, they will issue a signalNotReady()
		 * again quickly, and the system will stop again. To force the system into a ready state, call quitSystemFault().
		 *
		 * The function is called by the InspectionCmd interface.
		 *
		 * @param p_oErrorCode ErrorCode as defined in module/moduleLogger.h
		 */
		void signalReady( unsigned int p_oErrorCode );

		/**
		 * @brief Force system into ready state. See systemReady().
		 */
		void quitSystemFault();

		/**
		 * @brief Get the type of the exception that has set the system into the NotReady state.
		 * @return ErrorCode, see module/moduleLogger.h for details.
		 */
		int getRelatedException();

		/// Inspection
		void emergencyStop();

		void resetEmergencyStop();

		//@}

		/**
		 * @name States
		 * @brief Create a new state, change into another state, retrieve current state, etc.
		 */
		//@{

		/**
		 * @brief Create a new state - creates a new abstract state object.
		 * @param p_oNewState State object.
		 * @return Pointer to new abstract state object.
		 */
		AbstractState* createState(State newState);

		/**
		 * @brief Setzt die Statemachine auf Anfangsposition.
		 */
		void reset();

        void triggerSingle(const interface::TriggerContext &context) override;

	private:

		/**
		 * @brief Change the state.
		 * @param p_pNewState Shared-pointer to the new, abstract state.
		 */
		void changeState(Poco::SharedPtr<AbstractState> p_pNewState);
		/**
		 * @brief Change the state.
		 * @param p_oNewState state object.
		 */
		void changeState(State p_oNewState);

	public:

		/**
		 * @brief Get the current state.
		 * @return State object.
		 */
		State currentState() override;

		//@}

		/**
		 * @name Termination
		 */
		//@{

		/**
		 * @brief Blockiert den Thread solange bis der WF beendet wurde.
		 */
		void waitForTermination();
		/**
		 * @brief Deblockiert wartende Threads.
		 */
		void beginTermination();

		//@}

		/**
		 * @name Proxies
		 * @brief Get references to the proxies of the interfaces of the state machine (analyzer, db, etc).
		 */
		//@{

		/**
		 * @brief Liefert den AnalyzerProxy.
		 * @return Reference to analyzer interface proxy.
		 */
		analyzer::InspectManager& inspectManager();
		/**
		 * @brief Liefert den DBproxy.
		 * @return Reference to db interface proxy.
		 */
		TDb<AbstractInterface>&	getDB();
		/**
		 * @brief Liefert den SystemstatusProxy.
		 * @return Reference to system status interface proxy.
		 */
		TSystemStatus<AbstractInterface>& getSystemStatus();
		/**
		 * @brief Liefert den CalibrationProxy.
		 * @return Reference to calibration interface proxy.
		 */
		TCalibration<AbstractInterface>& getCalibration();
		/**
		 * @brief Liefert den InspectionOutProxy.
		 * @return Reference to inspection out interface proxy.
		 */
		TInspectionOut<AbstractInterface>& getInspectionOut();
		/**
		 * @brief Liefert den zentralen device manager.
		 * @return Reference to central device manager.
		 */
		precitec::analyzer::CentralDeviceManager& getDeviceManager();
		/**
		 * @brief Liefert den productteachIn Proxy.
		 * @return Reference to product teach-in interface proxy.
		 */
		TProductTeachIn<AbstractInterface> &getProductTeachInProxy();
		/**
		 * @brief Liefert den weldHeadMsg Proxy.
		 * @return Reference to weldhead message interface proxy.
		 */
		TWeldHeadMsg<AbstractInterface> &getWeldHeadMsgProxy();
		/**
		 * @brief Liefert den VideoRecorder Proxy.
		 * @return Reference to VideoRecorder event interface proxy.
		 */
		TVideoRecorder<AbstractInterface> &getVideoRecorderProxy();
		/**
		 * @brief Liefert den GrabberStatus Proxy.
		 * @return Reference to GrabberStatus interface proxy.
		 */
		grabberStatusProxy_t&	getGrabberStatusProxy();

		//@}

		/**
		 * @name Context
		 * @brief The actual state context information, for example station ID, product type, etc.
		 */
		//@{

		/**
		 * @brief Get the station id.
		 * @return Poco UUID object with the station id.
		 */
		Poco::UUID getStationID() const;
		/**
		 * @brief Set the product-GUID using the number and type. The function has to search for the GUID based on the type information.
		 * This function is called from startLive, startAutomatic, startSimulation in the inspectionCmdServer.
		 * @param p_oProductType unsigned int with the product type.
		 * @param p_oProductNr unsigned int with the number of the product (serial-number).
		 * @param p_rExtendedProductInfo std::string with the extended product information
		 */
		void setProductType( uint32_t p_oProductType, uint32_t p_oProductNr, const std::string& p_rExtendedProductInfo );
		/**
		 * @brief Set the product number and type. This function is called from startLive, startAutomatic, startSimulation in the inspectionCmdServer.
		 * @param p_oProductID Product GUID.
		 * @param p_oType unsigned int with the product type.
		 */
		void setProductID( const Poco::UUID& p_oProductID, uint32_t p_oProductType);
		/**
		 * @brief Set the new status of m_oNoHWParaAxis. This function is called from startAutomaticmode in inspectionServer. 
		 * @param p_oNoHWParaAxis new status of m_oNoHWParaAxis
		 */
		void setNoHWParaAxis( bool p_oNoHWParaAxis);

		/**
		 * @brief Get the current product ID.
		 * @param Poco UUID object with the product GUID.
		 */
		Poco::UUID getProductID();
		/**
		 * @brief Get the current product type.
		 * @return int - current product type.
		 */
		uint32_t getProductType();
        
		/**
		 * @brief Get the current product number (serial number).
		 * @return int - current product number (serial number).
		 */
		uint32_t getProductNumber();
		/**
		 * @brief Get the current extended product info.
		 * @return std::String - current extended product info.
		 */
		const std::string& getExtendedProductInfo() const;
		/**
		 * @brief Get the current status of m_oNoHWParaAxis.
		 * @return bool - current status of m_oNoHWParaAxis.
		 */
		bool getNoHWParaAxis();

		/**
		 * @brief Set the seam that is currently being inspected.
		 * @param p_oSeam int - seam number.
		 */
		void setSeam( int seam );
		/**
		 * @brief Get the seam that is currently being inspected.
		 * @return int - current seam number.
		 */
		int getSeam() const;

		/**
		 * @brief Set seam series that is currently being inspected.
		 * @param p_oSeamseries int - current seam series.
		 */
		void setSeamseries ( int seamseries ) override;
		/**
		 * @brief Get seam series that is currently being inspected.
		 * @return int - current seam series.
		 */
		int getSeamseries () const;

		//@}

		/**
		 * @brief Get a list of all available products.
		 */
		ProductList& getProductList() override;
		/**
		 * @brief Set the list of all products - this function is used internally when the products are loaded from the db.
		 */
		void setProductList(ProductList& prodList);

		void clearDefaultProduct( );

		/**
		 * @brief Set the default product.
		 * @param p_rDefault Reference to new default product.
		 */
		void setDefaultProduct(Product const& p_rDefault );

		/**
		 * @brief Get the default product.
		 * @return Product object.
		 */
		Product getDefaultProduct() const;

		/**
		 * @brief Inform the state machine, that the user has altered the database. The state machine might not apply the changes immediately, though. If the system is inspecting a part, the state machine will wait until the end of the cycle.
		 * @param p_oDbChange the db-change object that identifies the parts that have been changed.
		 */
		void dbAltered( DbChange p_oDbChange );

		/**
		 * @brief We have to check if the DB has changed, and for example inform the analyzer that a product definition was updated. This function has to be called by the state machine at state transitions when it is save to update the internal data structures.
		 */
		void dbCheck();

		/**
		 * @brief Turn the line lasers on or off. This function is only being used as fallback solution, if no hardware parameters sets are in place to control the line-lasers.
		 * @ param p_oEnable if true, then the line lasers are turned on, if false they are turned off.
		 */
		void enableLineLasers( bool p_oEnable );

		bool getForceHomingOfAxis();

		/**
		 * @brief: Is a backup requested?
		 */
		bool getIsBackupRequested();

		/**
		 * @brief: Set new requestedBackup state
		 */
		void setIsBackupRequested(bool p_oRequest);

		/**
		 * clearRelatedException
		 */
		void clearRelatedException();

		bool isSimulationStation() const;
		void setSimulationStation(bool set);

		/**
		 * @brief Inform the state machine, that the calibration needs to be refreshed
		 */
		void simulationCalibrationAltered(const int sensorId);
		/**
		 * @brief We have to check if calibration has changed. This function has to be called by the state machine at state transitions when it is save to update the internal data structures.
		 */
		void simulationCalibrationCheck(const int sensorId);

        void sendOperationState();

	private:

		Poco::SharedPtr<AbstractState>			m_pCurrentState;			///< Shared pointer to the current state.

		Poco::UUID 								m_oStation;					///< Station ID.
		Poco::Mutex 							m_oLock;					///< Poco::Mutex da recursiver Aufruf.
		Poco::Semaphore 						m_oBusy;					///< >0 wenn WF verarbeitet. 0 wenn WF heruntergefahren wird. Da nicht definiert ist, welcher Thread den Shutdown initiert muss ein Semaphore verwendet weden

		TDb<AbstractInterface>*					m_pDbProxy;					///< MessageProxy fuer DB Remotecalls.
		analyzer::InspectManager*	     		m_pInspectManager;			///< Inspect Manager for "remote calls"
		TSystemStatus<AbstractInterface>*		m_pSystemStatusProxy;		///< EventProxy fuer SystemStatus.
		TCalibration<AbstractInterface>*		m_pCalibrationProxy;		///< MessageProxy fuer den Calibration State.
		TInspectionOut<AbstractInterface>*		m_pInspectionOutProxy;		///< EventProxy fuer die Ausgabe an die HW (Kunden).
		precitec::analyzer::CentralDeviceManager& m_rDeviceManager;			///< Reference central device manager.
		TProductTeachIn<AbstractInterface>* 	m_pProductTeachInProxy; 	///< EventProxy fuer das ProduktTeachIn.
		TWeldHeadMsg<AbstractInterface>*		m_pWeldHeadMsgProxy;		///< MessageProxy fuer den WeldHead.
		TVideoRecorder<AbstractInterface>*		m_pVideoRecorderProxy;		///< EventProxy fuer den VideoRecorder.
		grabberStatusProxy_t&					m_rGabberStatusProxy;  		///< send notifications to grabber

		Poco::UUID 								m_oProductID;				///< Produkt GUID.
		uint32_t 								m_oProductType;				///< Produktetype vom Roboter.
		uint32_t 								m_oProductNr;				///< Produktenummer vom Roboter.
		std::string								m_oExtendedProductInfo;		///< extended product information (from customer system)
		bool      								m_oNoHWParaAxis;			///< in continuous mode: process no HW parameter and axis commands.

		int 									m_oSeam;					///< Nahtnummer vom Roboter.
		int 									m_oSeamseries;				///< Nahtfolgenummer vom Roboter; Optional; Default=1.

		Product 								m_oDefaultProduct;			///< Default/live-mode product.
		ProductList 							m_oProducts;				///< Liste mit den Produkten.

		int 									m_oRelatedException;		///< Fehlerbeschreibung.

		bool									m_oForceHomingOfAxis;		///< Gehe direkt in notReady-Zustand nach Init?

		std::vector< DbChange >					m_oDbChanges;				///< We have to see if the db was altered and have to inform other components whenever possible.
		Poco::Mutex		 						m_oDbChangesMutex;			///< Protect the db changes array ...
		bool 									m_oIsBackupRequested;		///< Steht ein Backupvorgang an?
		bool m_isSimulationCalibrationAltered;
		bool m_isSimulation;
};

typedef Poco::SharedPtr<StateContext> SmStateContext;


} // namespace workflow
} // namespace precitec

#endif /*STATECONTEXT_H_*/
