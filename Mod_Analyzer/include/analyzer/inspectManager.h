/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS, AB, SB, KIR
 * 	@date		2013
 * 	@brief		Manages inspection of image and sample data by means of data processing graphs respecting product structure. 
 *				Dispatches inspection results and data to further receivers.
 */

#ifndef INSPECTMANAGER_H_INCLUDED_20130724
#define INSPECTMANAGER_H_INCLUDED_20130724

// local includes

#include "Mod_Analyzer.h"
#include "analyzer/stdGraphBuilder.h"
#include "analyzer/product.h"
#include "analyzer/resultHandler.h"
#include "analyzer/centralDeviceManager.h"
#include "analyzer/hwParameters.h"
#include "analyzer/inspectTimer.h"
#include "analyzer/referenceCurves.h"
#include "analyzer/graphAssistent.h"
#include "analyzer/deviceParameter.h"
#include "analyzer/processingThread.h"

#include "math/calibration3DCoords.h" 
#include "math/calibrationData.h"
// wm includes

#include "geo/coordinate.h"

#include "filter/productData.h"

#include "common/measureTask.h"
#include "common/product.h"
#include "common/frame.h"
#include "common/seamData.h"
#include "common/product1dParameter.h"
#include "common/productCurves.h"
#include "common/defines.h"

#include "message/db.interface.h"
#include "message/weldHead.interface.h"

#include "event/sensor.h"
#include "event/triggerCmd.interface.h"
#include "event/results.interface.h"
#include "event/recorder.interface.h"
#include "event/recorderPoll.interface.h"
#include "event/systemStatus.interface.h"
#include "event/videoRecorder.interface.h"

#include "fliplib/SynchronePipe.h"
#include "fliplib/NullSourceFilter.h"

#include "Poco/UUID.h"
#include "Poco/Thread.h"

// stdlib includes

#include <map>
#include <set>
#include <array>
#include <queue>

namespace precitec {
namespace analyzer {

class ImageSender;

	/**
	 * @brief Manages inspection of image and sample data by means of data processing graphs respecting product structure. Dispatches inspection results and data to further receivers.
	 * @ingroup Analyzer
	 */
	class MOD_ANALYZER_API InspectManager {
	public:
		typedef interface::TDb<interface::AbstractInterface>			if_tdb_t;				
		typedef interface::TWeldHeadMsg<interface::AbstractInterface>	if_tweld_head_msg_t;	
		typedef interface::TTriggerCmd<interface::AbstractInterface>	if_ttrigger_cmd_t;		
		typedef interface::TResults<interface::AbstractInterface>		if_tresults_t;			
		typedef interface::TRecorder<interface::AbstractInterface>		if_trecorder_t;			
		typedef interface::TSystemStatus<interface::AbstractInterface>	if_tsystem_status_t;	
		typedef interface::TVideoRecorder<interface::AbstractInterface>	if_tvideo_recorder_t;	

        /**
         * Enum describing the processing mode for the current image.
         **/
        enum class ProcessingMode {
            Normal, ///< Normal processing
            Overtriggered, ///< Image is overtriggered, processing of image should be skipped
            MissingImage, ///< Image was not received, processing, overlay and videorecorder should be skipped
            OutOfOrderImage ///< Image has wrong image number and should be ignored (used only for error handling) 
        };

		/**
		 * @brief	CTOR
		 * @param	p_pDbProxy					database msg proxy			- requests product data from win database
		 * @param	p_pWeldHeadMsgProxy			msg proxy for WeldHead		- controls axes
		 * @param	p_pTriggerCmdProxy			trigger command event proxy - triggers image capture
		 * @param	p_pResultProxy				result event proxy			- sends inspection results to win or vicommunicator
		 * @param	p_pRecorderProxy			recorder event proxy		- sends image and overlay to gui
		 * @param	p_pSystemStatusProxy 		system status event proxy	- publishes inspection state to gui
		 * @param	p_pVideoRecorderProxy		video recorder event proxy	- sends image and meta data to video recorder
		 * @param	p_pCentralDeviceManager		Central component that connects the device interface of the qnx processes to the windows / GUI side.
		 */
		InspectManager(
				if_tdb_t*				p_pDbProxy,
				if_tweld_head_msg_t*	p_pWeldHeadMsgProxy,
				if_ttrigger_cmd_t*		p_pTriggerCmdProxy,
				if_tresults_t*			p_pResultProxy,
				if_trecorder_t*			p_pRecorderProxy,
				if_tsystem_status_t*	p_pSystemStatusProxy,
				if_tvideo_recorder_t*	p_pVideoRecorderProxy,
				CentralDeviceManager*	p_pCentralDeviceManager,
                bool simulationStation);

		/**
		 * @brief getter device parameter
		 */
		analyzer::DeviceParameter& getDeviceParameter();

		/**
		 * @brief	caches product measure tasks and graphs
		 * @param	p_rProduct			product data (product id, station id, parameter set ids, ...).
		 */
		void changeProduct(const interface::Product& p_rProduct);
		
		/**
		 * @brief deletes a sum error
		 * @param	p_rProductID		product id the sum error belongs to
		 * @param	p_rSumErrorID		sum error id of error to be delted
		 */
		bool deleteSumError(const Poco::UUID& p_rProductID, const Poco::UUID& p_rSumErrorID);

		/**
		 * @brief sets active product for an inspection cycle
		 * @param	p_rProductId		product id of product to be activated
		 * @param	p_oProductNb		product number of product to be activated
		 * @param	p_rExtendedProductInfo extended product info (e.g. serial number) of product to be activated
		 */
		bool activate(const Poco::UUID& p_rProductId, int p_oProductNb, const std::string &p_rExtendedProductInfo = {});	// Zyklus Start
		
		/**
		 * @brief	Activates and arms a seam series
		 * @param	p_oSeamseries			number of seam series to be activated
		 */
		bool activateSeamSeries(int p_oSeamseries);
		
		/**
		 * @brief	Activates and arms a seam
		 * @param	p_oSeam				number of seam to be activated
		 */
		bool activateSeam(int p_oSeamseries, const std::string &seamLabel = {});
				
		/**
		 * @brief changes a filter parameter
		 * @param	p_rMeasureTaskID		measure task id the filter parameter belongs to
		 * @param	p_rInstanceFilterId		instance filter id the filter parameter belongs to
		 */
		void changeFilterParameter(const Poco::UUID& p_rMeasureTaskID, const Poco::UUID& p_rInstanceFilterId);
		
		/**
		 * @brief pre start of a certain seam in a seam series
		 * @param	p_oSeamseries			number of seam series to be inspected
		 * @param	p_oSeam					number of seam to be inspected
		 */
		bool seamPreStart(int p_oSeamseries, int p_oSeam);

		/**
		 * @brief starts inspection of a certain seam in a seam series
		 * @param	p_oSeamseries			number of seam series to be inspected
		 * @param	p_oSeam					number of seam to be inspected
		 */
		bool startInspect(int p_oSeamseries, int p_oSeam, const std::string &label);
		
		/**
		 * @brief stops inspection of a certain seam in a seam series
		 */
		void stopInspect();

		/**
		 * @brief starts inspection of a seam series
		 * @param	p_oSeamseries			number of seam series to be inspected
		 */
		bool startSeamseries(int p_oSeamseries);

		/**
		 * @brief getter result handler 
		 */
		ResultHandler* getResultHandler() { return &m_oResultHandler; }

		/**
		 * @brief inspects an image frame
		 * @param	p_oSensorId				sensor id of data source 
		 * @param	p_rTriggerContext		trigger context, contains image  number
		 * @param	p_rImage				data to be processed
		 */
		void data(int p_oSensorId, const interface::TriggerContext& p_rTriggerContext, const image::BImage& p_rImage);

		/**
		 * @brief inspects a sample frame
		 * @param	p_oSensorId				sensor id of data source 
		 * @param	p_rTriggerContext		trigger context, contains sample number
		 * @param	p_rSample				data to be processed
		 */
		void data(int p_oSensorId, const interface::TriggerContext& p_rTriggerContext, const image::Sample& p_rSample);
	
		/**
		 * @brief sends product information to gui
		 */
		void updateProductInfo();

		/**
		 * @brief starts an live mode cycle
		 */
		void startLiveMode();

		/**
		 * @brief stops an live mode cycle
		 */
		void stopLiveMode();

		/**
		 * @brief starts an automatic cycle
		 */
		void startAutomaticMode();

		/**
		 * @brief stops an automatic cycle
		 */
		void stopAutomaticMode();

		/**
		 * @brief setter system status proxy
		 * @param	p_rNewProxy		new proxy to be set
		 */
		void setSystemStatusProxy(interface::TSystemStatus<interface::AbstractInterface>& p_rNewProxy){m_pSystemStatusProxy = &p_rNewProxy;}
		
		/**
		 * @brief setter state
		 * @param	p_oState		new state to be set
		 */
		void setState(workflow::State p_oState);	///< should only be called within stateMachine/stateContext when starting/stopping livemode or other modes
		
		/**
		 * @brief updates a hardware parameter
		 * @param	p_rParamSetId	id of hardware parameter set to be updated
		 * @param	p_oKey			key of value within hw parameter set to be updated
		 */
		void updateHwParameter( const Poco::UUID &p_rParamSetId, const Key p_oKey );

        void updateHardwareData(const FilterGraph* graph);
		/**
		 * @brief Set the new status of m_oNoHWParaAxis. This function is called from startAutomaticmode in inspectionServer. 
		 * @param p_oNoHWParaAxis new status of m_oNoHWParaAxis
		 */
		void setNoHWParaAxis( bool p_oNoHWParaAxis);

		/**
		 * @brief Get the current status of m_oNoHWParaAxis.
		 * @return bool - current status of m_oNoHWParaAxis.
		 */
		bool getNoHWParaAxis();

		bool loadCalibDataAfterSignal(int p_oSensorID, bool p_oInit);

		bool reloadCalibData(int p_oSensorID, math::Calibration3DCoords p_o3DCoords, math::CalibrationParamMap p_pCalibrationParameters);
        bool updateCorrectionGrid(int p_oSensorID, coordinates::CalibrationCameraCorrectionContainer p_oCameraCorrectionContainer, coordinates::CalibrationIDMCorrectionContainer p_oIDMCorrectionContainer);
        
        bool assertCalibData(int p_oSensorId); //for test
		math::CalibrationData & getCalibrationData(const int p_oSensorID);
		void resetCalibration(int p_oSensorID);

		/**
		 * @brief reset sum errors (simulation mode only)
		 */
		void resetSumErrors();

        std::shared_ptr<interface::TRecorderPoll<interface::AbstractInterface>> recorderPollServer() const;

        void triggerSingle(const interface::TriggerContext &context);

        void simulationDataMissing(const interface::TriggerContext &context);

        bool isSimulationStation() const
        {
            return m_simulationStation;
        }

	private:
		typedef fliplib::SynchronePipe<ImageFrame>					image_pipe_t;
		typedef fliplib::SynchronePipe<SampleFrame>					sample_pipe_t;
		typedef std::map<Poco::UUID, analyzer::Product>				product_map_t;
		typedef std::set<interface::Product>						product_data_set_t;
		typedef std::set<Poco::UUID>								uuid_set_t;
		typedef std::map<Poco::UUID, interface::MeasureTask>		measureTask_map_t;
        typedef std::array<image::OverlayCanvas, g_oNbParMax>       overlay_buffer_t;
        typedef std::array<ProcessingThread, g_oNbParMax>           worker_threads_t;
        typedef std::array<ImageFrame, g_oNbParMax>                 image_frames_t;
        typedef std::array<std::map<int, SampleFrame>, g_oNbParMax> sample_frames_t;
        typedef std::array<interface::TriggerContext, g_oNbParMax>  trigger_contexts_t;
        typedef std::array<int, g_oNbParMax>  						sensor_ids_t;
        typedef std::array<ProcessingMode, g_oNbParMax>             processing_modes_t;
        typedef std::queue<std::tuple<int, ImageFrame, ProcessingMode>> image_queue_t;
        typedef std::array<std::unique_ptr<sample_pipe_t>, g_oNbParMax> sample_pipes_t;
        typedef std::array<fliplib::NullSourceFilter, g_oNbParMax> null_source_filters_t;

        void processingThreadFinishedCallback();

		/**
		 * @brief builds and caches a filter graph
		 * @param	p_rMeasureTaskId		id of measure task which contains the graph
		 * @param	p_rGraphId				id of graph to be built and cached
		 * @param	p_rParamSetId			id of parameter set to be built and cached
		 * @retuen	iterator to entry in graph map
		 */
		graph_map_t::const_iterator storeGraphAndParamterSet(const Poco::UUID &p_rMeasureTaskId, const Poco::UUID &p_rGraphId,  const Poco::UUID &p_rParamSetId);

		/**
		 * @brief builds and caches a filter graph
		 * @param	p_rMeasureTaskId		id of measure task which contains the graph
		 * @param	p_rGraphId				id of graph to be built and cached
		 * @retuen	iterator to entry in graph map
		 */
		graph_map_t::const_iterator cacheGraph(const Poco::UUID &p_rMeasureTaskId, const Poco::UUID &p_rGraphId);

		/**
		 * @brief builds and caches a paremeter set
		 * @param	p_pGraph					graph containing filter ids
		 * @param	p_rParamSetId				id parameter set
		 */
		void cacheParamSet(const Poco::UUID& p_rParamSetId, const fliplib::FilterGraph* p_pGraph);
		
		/**
		 * @brief changes the seam interval
		 * @param	p_oActualPos				actual position in seam (every seam starts at position 0)
		 */
		void changeSeamInterval(int p_oActualPos);
		
		/**
		 * @brief changes the seam interval depending on current position
		 * @param	p_oImageNumber			seam-related number of image / sample
		 */
		void checkSeamIntervalChange(int p_oImageNumber);

		/**
		 * @brief processes an image frame by means of a graph
		 * @param	p_oSensorId				sensor id of data source 
		 * @param	p_rTriggerContext		trigger context, contains image number
		 * @param	p_rImage				image to be processed
         * @return startProcessing called (false if still waiting for other sources)
		 */	
		bool processImage(int p_oSensorId, const interface::TriggerContext& p_rTriggerContext, const image::BImage& p_rImage, ProcessingMode mode);
		
		/**
		 * @brief processes a sample frame by means of a graph
		 * @param	p_oSensorId				sensor id of data source 
		 * @param	p_rTriggerContext		trigger context, contains sample number
		 * @param	p_rSample				data to be processed
         * * @return startProcessing called (false if still waiting for other sources)
		 */
		bool processSample(int p_oSensorId, const interface::TriggerContext& p_rTriggerContext, const image::Sample& p_rSample);

		/**
		 * @brief arm a hardware parameter
		 * @param	p_rParamSetId		id of hardware parameter set to be armed
		 */
		void armHwParameters( const Poco::UUID &p_rParamSetId );

		/**
		 * @brief creates an instance of image context
		 * @param	p_rTriggerContext		trigger context, contains image / sample number
		 */
		interface::ImageContext makeImageContext(const interface::TriggerContext& p_rTriggerContext) const;

		/**
		 * @brief Releases data pipes and result handler from graph
         * @param	p_pGraph					graph to be modified
		 */
		void releaseDataPipesAndResultHandler(fliplib::FilterGraph* p_pGraph);

		/**
		 * @brief Join all worker threads
         * @param	p_pGraph					graph to be modified
		 */
		void joinWorkers();

		/**
		 * @brief Checks whether there is a queued SampleFrame for every connected sensor
		 * @returns @c true if either there are no samples used in graph or if all used sensor have submitted a Sample
		 */
		bool areAllSamplesQueued();

		/**
		 * @brief Queues one Sample
		 * @param sensorId The sensor for which the sample got submitted
		 * @param triggerContext Trigger context, contains sample number
		 * @param sample The sample
		 */
		void queueSample(int sensorId, const interface::TriggerContext &triggerContext, const image::Sample &sample);

		/**
		 * @brief Dequeues the current set of pending samples.
		 * @returns a map of sensor id (key) and SampleFrame (value)
		 */
		std::map<int, SampleFrame> dequeueSamples();

		/**
		 * Starts the processing on the @p oIdxWorkerCur thread.
		 */
		void startProcessing(size_t oIdxWorkerCur);

        /**
         * Called by activateSeamSeries.
         * Needs to be used by all methods which are already locked.
         **/
        bool activateSeamSeriesInternal(int p_oSeamseries);

        /**
         * Called by stopInspect.
         * Needs to be used by all methods which are already locked.
         **/
		void stopInspectInternal();
        
        void updateInspectManagerTime(const std::chrono::nanoseconds &time, bool increaseFrameCount );
        
        /*debug function: send inspect time as a result  inpute: InspecTime, ProcessTime*/
        void sendLastTimeResult (ResultType oResultType); 

		if_tdb_t&						getDBSrv()				{ return *m_pDbProxy; }				///< getter proxy
		if_tweld_head_msg_t&			getWeldHeadMsgSrv()		{ return *m_pWeldHeadMsgProxy; }	///< getter proxy
		if_ttrigger_cmd_t&				getTriggerCmdSrv()		{ return *m_pTriggerCmdProxy; }		///< getter proxy
		if_tresults_t&					getResultSrv()			{ return *m_pResultProxy; }			///< getter proxy
		if_trecorder_t&					getRecorderSrv()		{ return *m_pRecorderProxy; }		///< getter proxy
		if_tsystem_status_t&			getSystemStatusSrv()	{ return *m_pSystemStatusProxy; }	///< getter proxy
		if_tvideo_recorder_t&			getVideoRecorderSrv()	{ return *m_pVideoRecorderProxy; }	///< getter proxy

		if_tdb_t*            			m_pDbProxy;            		///< database msg proxy				- requests product data from win database
		if_tweld_head_msg_t*			m_pWeldHeadMsgProxy;		///< msg proxy for WeldHead			- controls axes
		if_ttrigger_cmd_t*    			m_pTriggerCmdProxy;    		///< trigger command event proxy	- triggers image capture
		if_tresults_t*       			m_pResultProxy;        		///< result event proxy				- sends inspection results to win or vicommunicator
		if_trecorder_t*      			m_pRecorderProxy;      		///< recorder event proxy			- sends image and overlay to gui
		if_tsystem_status_t*  			m_pSystemStatusProxy;  		///< system status event proxy		- publishes inspection state to gui
		if_tvideo_recorder_t* 			m_pVideoRecorderProxy; 		///< video recorder event proxy		- sends image and meta data to video recorder


		DeviceParameter						m_oDeviceParameter;			///< key value parameter
		CentralDeviceManager* 				m_pCentralDeviceManager;	///< central device manager - is called to distribute the hardware parameters to the actual devices / processes.
		StdGraphBuilder   					m_oGraphBuilder;			///< graph builder - instantiates filters

		Product*		  					m_pActiveProduct;			///< points to active product, set in activate()
		const SeamSeries*		  			m_pActiveSeamSeries;		///< points to active seam series, set in startInspect()
		const Seam*		  					m_pActiveSeam;				///< points to active seam, set in startInspect() 
        std::string m_activeSeamLabel;
		const SeamInterval*		  			m_pActiveSeamInterval;		///< points to active seam interval, set in changeSeamInterval() 
		fliplib::FilterGraph*		  		m_pActiveGraph;				///< points to active Graph, set in changeSeamInterval() 

		Poco::UUID        					m_oActiveProductInstanceId;	///< newly generated id: analog to product instance id in DB

		null_source_filters_t 				m_oNullSourceFilters;        ///< placeholder

		image_pipe_t  						m_oPipeImageFrame;			///< image source pipe
		sample_pipes_t 						m_oPipesSampleFrame;		///< sample source pipe

		ResultHandler     					m_oResultHandler;			///< result handler

		workflow::State 					m_oState;         			///< state of stateContext: livemode, automatic, calibration, ...

		Poco::FastMutex						m_oManSync;   				///< access synchronized
		std::size_t							m_oCurrentPosSeam;			///< position relative to seam [um]
        std::size_t 						m_oNbSeamSignaled;   	    ///< how often graph processing was invoked per seam - 0-based
        std::size_t 						m_oNbSeamJoined;   			///< how often graph processing was finished per seam - 0-based

		product_data_set_t					m_oProductData;				///< product data cache (interface::product)
		uuid_set_t							m_oUuidsBuilt;				///< ids of entities built during changeProduct
		graph_map_t							m_oGraphs;					///< graph instances cache
		product_map_t						m_oProducts;				///< product cache (analyzer::product)
		measureTask_map_t					m_oMeasureTasks;			///< measure task cache, needed for 'changeFilterParameter'
		paramSetMap_t						m_oParameterSetMap;			///< caches parameter sets for quick shift between seam intervals
		HwParameters						m_oHwParameters;			///< holds hw parameter sets
		ReferenceCurves						m_oReferenceCurves;			///< provides and holds reference curves
		ProductData							m_oExternalProductData;		///< external product data, eg velocity
		HardwareData                        m_externalHardwareData;     ///< external harware data. eg xcc
		bool      							m_oNoHWParaAxis;			///< in continuous mode: process no HW parameter and axis commands.
		
        InspectTimer						m_oInspectTimer;			///< measures image- and sample-processing time, frame-to-frame time, overlay time
	    overlay_buffer_t                    m_oCanvasBuffer;            ///< the active graph inserts overlay primitives in the canvas
        worker_threads_t				    m_oWorkers;					///< worker threads, number of threads actually used depends on global parameter
        image_frames_t                      m_oImageFrames;             ///< image frame buffer
        sample_frames_t                     m_oSampleFrames;            ///< sample frame buffer
        sensor_ids_t						m_oImageIds;				///< sensor ids of image frames buffer
        trigger_contexts_t					m_oTriggerContexts;         ///< trigger context buffer
        image_queue_t						m_oImageQueue;				///< queues incoming sensor data. image and sample data get synchronized.
        std::map<int, std::queue<SampleFrame>> m_oSampleQueues; 		///< queues incoming sensor data. image and sample data get synchronized.
        processing_modes_t                  m_oProcessingModes;         ///< The ProcessingMode per worker
        int m_lastProcessedImage = -1;
        int m_lastSkippedImageNumber = -1;
        bool m_simulationStation;
        std::shared_ptr<ImageSender> m_imageSender;
        int  m_oImageNumberOffset = 0;  //only for debug (see processImage)
        int m_oNumImagesSkippedFromSensor = 0;
        int m_oNumImagesSkippedInInspection = 0;
        bool m_hasHardwareCamera = true;
        uint32_t m_triggerCycle = 0;
        std::mutex m_callbackMutex;
        std::map<const SeamSeries*, std::map<int, int>> m_seamReuseCounters;
        bool m_oLiveModeRequiresCamera;
        std::vector<int> m_oSensorIds;
        bool m_continuouslyModeActive;
        bool m_SOUVIS6000_Application;
        bool m_scanlabScannerEnable;
        bool m_IDM_Device1Enable;
        bool m_LWM40_No1_Enable;
        bool m_CLS2_Device1Enable;
        bool m_infiniteNumberOfTriggers;

        Poco::UUID m_lastSetParameterSet;
	}; // InspectManager

} // namespace analyzer
} // namespace precitec

#endif /*INSPECTMANAGER_H_INCLUDED_20130724*/
