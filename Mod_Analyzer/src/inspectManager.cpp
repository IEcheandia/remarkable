/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS, AB, SB, KIR
 * 	@date		2013
 * 	@brief		Manages inspection of image and sample data by means of data processing graphs respecting product structure.
 *				Distributes inspection results and data to further receivers.
 */

// local  includes

#include "analyzer/inspectManager.h"

// wm includes

#include "common/geoContext.h"
#include "overlay/overlayCanvas.h"
#include "common/connectionConfiguration.h"
#include "common/systemConfiguration.h"
#include "common/triggerInterval.h"
#include "analyzer/sumError.h"
#include "event/systemStatus.h"
#include "util/calibDataSingleton.h"
#include "system/tools.h"
#include "util/camGridData.h"
#include "analyzer/signalAdapter.h"
#include "analyzer/imageSender.h"

#include "module/moduleLogger.h"

#include "filter/armStates.h"
#include "filter/sensorFilterInterface.h"

#include "fliplib/GraphBuilderFactory.h"

// poco includes
#include "Poco/Environment.h"

// stdlib includes

#include <sys/resource.h>
#include <sstream>


using namespace Poco;
using Poco::UUID;
using namespace fliplib;

namespace precitec {
	using namespace system;
	using namespace workflow;
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	using namespace filter;
	using math::SensorModel;
	using math::SensorModelDescription;
namespace analyzer {

static const auto	g_oDefaultGraphID		= UUID			( "F9BB3465-CFDB-4DE2-8C8D-EB974C667ACA" ); // see "SET @NewDefaultGraphID = 'F9BB3465-CFDB-4DE2-8C8D-EB974C667ACA'" in Update.sql

InspectManager::InspectManager(
		if_tdb_t*				p_pDbProxy,
		if_tweld_head_msg_t*	p_pWeldHeadMsgProxy,
		if_ttrigger_cmd_t*		p_pTriggerCmdProxy,
		if_tresults_t*			p_pResultProxy,
		if_trecorder_t*			p_pRecorderProxy,
		if_tsystem_status_t*	p_pSystemStatusProxy,
		if_tvideo_recorder_t*	p_pVideoRecorderProxy,
		CentralDeviceManager*	p_pCentralDeviceManager,
        bool simulationStation) :
	m_pDbProxy						{ p_pDbProxy },
	m_pWeldHeadMsgProxy				{ p_pWeldHeadMsgProxy },
	m_pTriggerCmdProxy				{ p_pTriggerCmdProxy },
	m_pResultProxy					{ p_pResultProxy },
	m_pRecorderProxy				{ p_pRecorderProxy },
	m_pSystemStatusProxy			{ p_pSystemStatusProxy },
	m_pVideoRecorderProxy			{ p_pVideoRecorderProxy },
    m_oDeviceParameter{simulationStation},
	m_pCentralDeviceManager			{ p_pCentralDeviceManager },

	m_pActiveProduct				{ nullptr },
	m_pActiveSeamSeries				{ nullptr },
	m_pActiveSeam					{ nullptr },
	m_pActiveSeamInterval			{ nullptr },
	m_pActiveGraph					{ nullptr },
	m_oPipeImageFrame				{ &m_oNullSourceFilters[0], SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE },
	m_oPipesSampleFrame				{ },
	m_oResultHandler				{ m_pResultProxy },
	m_oState						{ State::eInit },
	m_oManSync						{},
	m_oCurrentPosSeam				{ 0 },
	m_oNbSeamSignaled               { 0 },
	m_oNbSeamJoined         		{ 0 },
	m_oReferenceCurves				{ m_pDbProxy },
	m_oNoHWParaAxis                 { false},
    m_simulationStation{simulationStation},
    m_imageSender(std::make_shared<ImageSender>()),
    m_hasHardwareCamera(SystemConfiguration::instance().getBool("HardwareCameraEnabled", true)),
    m_continuouslyModeActive(SystemConfiguration::instance().getBool("ContinuouslyModeActive", false)),
    m_SOUVIS6000_Application(SystemConfiguration::instance().getBool("SOUVIS6000_Application", false)),
    m_scanlabScannerEnable(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::Scanner2DEnable)),
    m_IDM_Device1Enable(SystemConfiguration::instance().getBool("IDM_Device1Enable", false)),
    m_LWM40_No1_Enable(SystemConfiguration::instance().getBool("LWM40_No1_Enable", false)),
    m_CLS2_Device1Enable(SystemConfiguration::instance().getBool("CLS2_Device1Enable", false)),
    m_infiniteNumberOfTriggers(SystemConfiguration::instance().getBool("SCANMASTER_Application", false) && SystemConfiguration::instance().getBool("SCANMASTER_GeneralApplication", false)),
    m_lastSetParameterSet(Poco::UUID::null())
{
#ifndef NDEBUG
	std::stringstream oMsg;
	oMsg << __FUNCTION__ << " " << __DATE__ << " " << __TIME__ << '\n';
	wmLog(eDebug, oMsg.str());
#endif

    assert(g_oNbPar >= 0);
    assert(g_oNbPar <= g_oNbParMax);

#if defined __QNX__ || defined __linux__
    if(Environment::processorCount() < g_oNbPar)
    {
    	std::ostringstream osstr;
    	osstr<<Environment::processorCount()<<"  Processors";
    	std::string str=osstr.str();
    	wmLog(eInfo,"Only %s\n", str.c_str());
    }
#endif
    for (std::size_t i = 0; i < g_oNbParMax; i++)
    {
        m_oPipesSampleFrame[i].reset(new sample_pipe_t(&m_oNullSourceFilters[i], SensorFilterInterface::SENSOR_SAMPLE_FRAME_PIPE));
    }

    if (!m_simulationStation)
    {
        struct rlimit niceLimit;
        niceLimit.rlim_cur = 30;
        niceLimit.rlim_max = 30;
        if (setrlimit(RLIMIT_NICE, &niceLimit) != 0)
        {
            wmLog(eError, "Failed to adjust nice rlimit\n");
        }
        system::raiseRtPrioLimit();
    }

	for (size_t i = 0; i < Environment::processorCount() && i < g_oNbParMax; i++)
	{
        if (m_simulationStation)
        {
            m_oWorkers.at(i).setNice(1);
            m_oWorkers.at(i).setBatch(true);
        }
        else
        {
            m_oWorkers.at(i).setNice(-10);
        }
        m_oWorkers.at(i).setName(std::string{"Processing#"} + std::to_string(i));
		m_oWorkers.at(i).startThread();
		if (!m_simulationStation)
		{
			m_oWorkers[i].setCPUAffinity(i);
		}
		m_oWorkers.at(i).setWorkDoneCallback(std::bind(&InspectManager::processingThreadFinishedCallback, this));
	}

    m_imageSender->setRecorder(m_pRecorderProxy);
    m_imageSender->setHasFramegrabber(m_hasHardwareCamera);
    if (m_simulationStation)
    {
        m_imageSender->setSimulation();
    }
} // InspectManager()



DeviceParameter& InspectManager::getDeviceParameter()
{
	return m_oDeviceParameter;
} // getDeviceParameter



void InspectManager::changeProduct(const interface::Product& p_rProduct) {
	system::Timer oTimer; oTimer.start();
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};
	wmLog( eDebug, "*************** Updating product '%s'... ***************\n", p_rProduct.name().c_str() );
	poco_assert(nullptr == m_pActiveSeam);			// if not null inspection not stopped and stopInspection() might encounter invalidated ptrs
	poco_assert(nullptr == m_pActiveSeamInterval);	// if not null inspection not stopped and stopInspection() might encounter invalidated ptrs

	// definitions may have change maintaining their id (key), so discard old ones

	m_oProductData.erase(p_rProduct);				// delete old definition if existant
	m_oProducts.erase(p_rProduct.productID());		// delete old definition if existant
	m_oUuidsBuilt.clear();

	auto&			rProductData			= *m_oProductData.emplace(p_rProduct).first;										// insert ProductData into cache
	auto&			rProduct				= m_oProducts.emplace(p_rProduct.productID(), analyzer::Product{rProductData}).first->second;								// insert Product into cache
	const auto		oMeasureTasks			= getDBSrv().getMeasureTasks(p_rProduct.stationId(), p_rProduct.productID());
	auto			pLastAddedSeamSeries	= (SeamSeries*)	nullptr;
	auto			pLastAddedSeam			= (Seam*)		nullptr;

	// get sum errors from db and store them

	auto	 oProdParams =	getDBSrv().getProductParameter(rProductData.productID());

    auto productCurves = getDBSrv().getProductCurves(rProductData.productID());
    std::vector<ReferenceCurveSet> productReferenceCurves;
    for (const auto& curveId : productCurves.m_curveSets)
    {
        productReferenceCurves.emplace_back(getDBSrv().getReferenceCurveSet(rProductData.productID(), curveId));
    }

    wmLog( eDebug, "InspectManager::changeProduct() getProductParameter returns oProdParams.size() <%d>\n", oProdParams.size());

    rProduct.lStopSetValues(oProdParams);

    rProduct.seClearAll();
    rProduct.createSumErrors(oProdParams, productReferenceCurves);

	// add the product hardware parameter set to the cache

	m_oHwParameters.erase(rProductData.hwParameterSatzID()); // delete old definition if existant
	m_oHwParameters.cacheHwParamSet(rProductData.hwParameterSatzID(), getDBSrv());
	m_oUuidsBuilt.insert(rProductData.hwParameterSatzID());

	// process measure task data depending on task level

	for (auto oItMeasureTask = std::begin(oMeasureTasks); oItMeasureTask != std::end(oMeasureTasks); ++oItMeasureTask) {
		m_oMeasureTasks[oItMeasureTask->taskID()]	=	*oItMeasureTask; // overwrite if existing

		const auto&	rMTask				=	m_oMeasureTasks[oItMeasureTask->taskID()];	// get the reference to cache
		const auto&	rParamSetId			=	rMTask.parametersatzID();
		const auto&	rHwParamSetId		=	rMTask.hwParametersatzID();

		if (m_oUuidsBuilt.find(rHwParamSetId) == std::end(m_oUuidsBuilt)) {
			m_oHwParameters.erase(rHwParamSetId); // force cache
			m_oUuidsBuilt.insert(rHwParamSetId);
			m_oHwParameters.cacheHwParamSet(rHwParamSetId, getDBSrv());
	    } // if

		switch (oItMeasureTask->level()) {
		case 0:		// seam series
			pLastAddedSeamSeries		=	rProduct.addSeamSeries(rMTask);
			break;
		case 1: {	// seam - may contain a graph
			const auto	oCItGraph		=	storeGraphAndParamterSet(rMTask.taskID(), rMTask.graphID(), rParamSetId);

			poco_assert(pLastAddedSeamSeries != nullptr);
			pLastAddedSeam				=	pLastAddedSeamSeries->addSeam(rMTask, oCItGraph);
			} // case expression
			break;
		case 2 : {	// seam interval - may contain a graph
			const auto	oCItGraph		=	storeGraphAndParamterSet(rMTask.taskID(), rMTask.graphID(), rParamSetId);

			if (p_rProduct.defaultProduct() == false) {
				poco_assert(pLastAddedSeam != nullptr);
				pLastAddedSeam->addSeamInterval(rMTask, oCItGraph);
			} // if
			else {
				pLastAddedSeamSeries		=	rProduct.addSeamSeries(rMTask);
				pLastAddedSeam				=	pLastAddedSeamSeries->addSeam(rMTask, oCItGraph); // live mode: set normal graph as dummy for crosswise action
				pLastAddedSeam->addSeamInterval(rMTask, oCItGraph);
			} // else

			} // case expression
			break;
		default :
			poco_bugcheck_msg("Invalid measure task level"); break;
		} // switch

	} // for

	// Inform user that the product definition was updated

	if (p_rProduct.defaultProduct() == false) { // "Default" occurs for every live mode cycle 3 times though the user has not made any changes
		wmLogTr( eInfo, "QnxMsg.Workflow.ProdUpdated", "Product '%s' updated.\n", p_rProduct.name().c_str() );
	} // if
	oTimer.elapsed(); wmLog( eDebug, "*************** Product '%s' updated. %d ms\n", p_rProduct.name().c_str(), oTimer.ms() );
	m_pSystemStatusProxy->productUpdated(p_rProduct.productID());
} // changeProduct



bool InspectManager::deleteSumError(const UUID& p_rProductID, const UUID& p_rSumErrorID) {
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};
	poco_assert(m_oProducts.find(p_rProductID) != std::end(m_oProducts));
	return m_oProducts.find(p_rProductID)->second.seDelete(p_rSumErrorID);
} // deleteSumError



bool InspectManager::activate(const UUID& p_rProductId, int p_oProductNb, const std::string &p_rExtendedProductInfo) {
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};

	auto oItProdFound = m_oProducts.find(p_rProductId);

	if((oItProdFound == m_oProducts.end()) || (&oItProdFound->second == nullptr/*AL simu fix?*/)){
		wmLogTr(eWarning, "QnxMsg.Workflow.NoProduct", "Product %i with ID '%s' not found.\n", p_oProductNb, p_rProductId.toString().c_str());
		return false;
	} // if

	m_pActiveProduct				= 	&oItProdFound->second; assert(m_pActiveProduct != nullptr);
	m_pActiveProduct->m_oProductNb	=	p_oProductNb;
    m_pActiveProduct->setExtendedProductInfo(p_rExtendedProductInfo);
	m_oActiveProductInstanceId		=	UUIDGenerator::defaultGenerator().create();

    // new cycle (new startLive or new startAuto) -> set product in result / nio handler and viceversa
	m_pActiveProduct->setResultProxy(m_oResultHandler.proxy());
	m_oResultHandler.setProduct(m_pActiveProduct);

	wmLog(eDebug, "%s - %s\tProductId: %s\tp_oProductNb: %i activated.\n", __FUNCTION__, m_pActiveProduct->m_rProductData.name().c_str(), p_rProductId.toString().c_str(), p_oProductNb);

	return true;
} // activate



bool InspectManager::activateSeamSeries(int p_oSeamseries) {
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};
    return activateSeamSeriesInternal(p_oSeamseries);
} // activateSeamSeries

bool InspectManager::activateSeamSeriesInternal(int p_oSeamseries)
{
	const auto	oSeamSeriesFound	= m_pActiveProduct->m_oSeamSeries.find(p_oSeamseries);
	if (oSeamSeriesFound == std::end(m_pActiveProduct->m_oSeamSeries)) {
		wmLogTr(eWarning, "QnxMsg.Workflow.NoSeamSeries", "Product '%s' does not contain seam series '%i'.\n", m_pActiveProduct->m_rProductData.name().c_str(), p_oSeamseries + 1); // GUI +1
		return false;
	} // if
	m_pActiveSeamSeries				= &oSeamSeriesFound->second; assert(m_pActiveSeamSeries != nullptr);

    // HW parameters only if WM is not in continuous mode
    if (!m_continuouslyModeActive)
    {
        armHwParameters(m_pActiveSeamSeries->m_oHwParamSetId); // arm on seam series and seam level
        //wmLog(eDebug, "HW armed on seam series level: '%s'\n", m_pActiveSeamSeries->m_oHwParamSetId.toString().c_str());
    }

	m_pActiveProduct->seArm( eSeamSeriesStart, interface::tSeamIndex( p_oSeamseries, -1, -1 ) );

    m_pActiveSeam = nullptr;
    m_activeSeamLabel = {};

	return true;
}



bool InspectManager::activateSeam(int p_oSeam, const std::string &seamLabel) {
   // const auto oScopedLock = atomLock( &m_oManSync ); // no lock, because called internally from startInspect

	const auto	oSeamFound			= m_pActiveSeamSeries->m_oSeams.find(p_oSeam);
	if (oSeamFound == std::end(m_pActiveSeamSeries->m_oSeams)) {
		wmLogTr(eWarning, "QnxMsg.Workflow.NoSeam", "Seam series '%i' does not contain seam '%i'.\n", m_pActiveSeamSeries->m_oNumber + 1, p_oSeam + 1); // GUI +1
		return false;
	} // if
	m_pActiveSeam					= &oSeamFound->second;
    m_activeSeamLabel = seamLabel;
    if (!m_simulationStation)
    {
        auto seamSeriesIt = m_seamReuseCounters.insert(std::make_pair(m_pActiveSeamSeries, std::map<int, int>{})).first;
        auto it = seamSeriesIt->second.find(p_oSeam);
        if (it == seamSeriesIt->second.end())
        {
            seamSeriesIt->second.emplace(p_oSeam, 0);
        }
        else
        {
            (it->second)++;
            // max number of seams
            int label = std::max_element(m_pActiveSeamSeries->m_oSeams.begin(), m_pActiveSeamSeries->m_oSeams.end(), [] (const auto &a, const auto &b) { return a.first < b.first; })->first;
            // plus all reused seams
            for (const auto &reuse : seamSeriesIt->second)
            {
                label += reuse.second;
            }
            if (m_activeSeamLabel.empty())
            {
                m_activeSeamLabel = std::to_string(label);
            }
        }
    }

    // HW parameters only if WM is not in continuous mode
    if (!m_continuouslyModeActive)
    {
        armHwParameters(m_pActiveSeam->m_pMeasureTask->hwParametersatzID()); // arm on seam series and seam level
        //wmLog(eDebug, "HW armed on seam level: '%s'\n", m_pActiveSeam->m_pMeasureTask->hwParametersatzID().toString().c_str());
    }

	return true;
} // activateSeam



// Parameter eines Filters aendern: aus DB auslesen, in Filter setzen, Filter armen
void InspectManager::changeFilterParameter(const UUID& p_rMeasureTaskID, const UUID& p_rInstanceFilterId) {

	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};
	const auto	oMeasureTaskFound		=	m_oMeasureTasks.find(p_rMeasureTaskID);

	poco_assert(oMeasureTaskFound != std::end(m_oMeasureTasks));

	const auto&	rMeasureTask			=	oMeasureTaskFound->second;

	if(rMeasureTask.name() == "Default" && !m_simulationStation) { // workaround: return if qnx and default product
		return; // currently not useful for qnx side and default produdt, cause product change is always triggered after param change
	} // if

	const auto&	rGraphId				=	rMeasureTask.graphID();
	const auto	oGraphFound				=	m_oGraphs.find(rGraphId);

	poco_assert(oGraphFound != std::end(m_oGraphs));

	auto		pGraph					=	oGraphFound->second.get();
	auto		oGraphAssistent			=	GraphAssistent ( pGraph );

	oGraphAssistent.changeParameterFilterDb(getDBSrv(), p_rMeasureTaskID, p_rInstanceFilterId);

	const auto	oParameterList	=	getDBSrv().getFilterParameter(p_rInstanceFilterId, p_rMeasureTaskID);
    wmLog( eDebug, "InspectManager::changeFilterParameter() start with oParameterList.size() <%d>\n", oParameterList.size());

	m_oReferenceCurves.requestAndStoreRefCurves(oParameterList); // request and store reference curve(s) if graph contains one or more ref curve filters
	oGraphAssistent.arm(eSeamStart);
	oGraphAssistent.arm(eSeamIntervalChange);

    wmLogTr( eInfo, "QnxMsg.Workflow.ParamUpdated", "Filter parameter updated.\n");
	m_pSystemStatusProxy->filterParameterUpdated(p_rMeasureTaskID, p_rInstanceFilterId);
} // changeFilterParameter


// Vorstart der Inspektion. Dazu wird die Naht gesucht, welche jetzt inspiziert werden soll
bool InspectManager::seamPreStart(int p_oSeamseries, int p_oSeam) {
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};

	try {
		poco_assert(nullptr != m_pActiveProduct);

		// if active seam series pointer is null, the seam series signal did not occur before the seam signal. this is OK for small applications with few seams, therefore we have to activate the seam series here manually ...
		if ( m_pActiveSeamSeries == nullptr )
		{
			const auto oActivateSeamSeriesOk = activateSeamSeriesInternal( 0 );
			if ( oActivateSeamSeriesOk == false ) {
				return false;
			} // if

            // no warning if WM is in continuous mode or in souvis6000 mode
            if (m_continuouslyModeActive || m_SOUVIS6000_Application)
            {
            }
            else
            {
                // warn user that this is not OK, especially once he configures hw-parameters on seam series level ...
                wmLogTr(eWarning, "QnxMsg.Workflow.NoSSSignal", "Seam '%i' activated without seam-series signal since last cycle!\n", p_oSeam + 1); 	// GUI +1
            }
		} // if

		const auto	oActivateSeamOk			=	activateSeam(p_oSeam);
		if (oActivateSeamOk == false) {
			return false;
		} // if

	} // try
	catch(...) {
		logExcpetion(__FUNCTION__, std::current_exception());
	} // catch

	return true;
} // seamPreStart


// Starte die Inspektion. Dazu wird die Naht gesucht, welche jetzt inspiziert werden soll, Trigger aufsetzen
bool InspectManager::startInspect(int p_oSeamseries, int p_oSeam, const std::string &label) {
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};

	try {
		poco_assert(nullptr != m_pActiveProduct);
		m_oLiveModeRequiresCamera = false;
		if (m_oState != State::eLiveMode ) {
			wmLogTr( eInfo, "QnxMsg.Workflow.InspectStart", "Inspection started (%i,%i).\n", p_oSeamseries, p_oSeam);
		} // if

		// if active seam series pointer is null, the seam series signal did not occur before the seam signal. this is OK for small applications with few seams, therefore we have to activate the seam series here manually ...
		if ( m_pActiveSeamSeries == nullptr )
		{
			const auto oActivateSeamSeriesOk = activateSeamSeriesInternal( 0 );
			if ( oActivateSeamSeriesOk == false ) {
				return false;
			} // if

            // no warning if WM is in continuous mode or in souvis6000 mode
            if (m_continuouslyModeActive || m_SOUVIS6000_Application)
            {
            }
            else
            {
                // warn user that this is not OK, especially once he configures hw-parameters on seam series level ...
                wmLogTr(eWarning, "QnxMsg.Workflow.NoSSSignal", "Seam '%i' activated without seam-series signal since last cycle!\n", p_oSeam + 1); 	// GUI +1
            }
		} // if

		if (m_pActiveSeam == nullptr) // seam was not defined in function seamPreStart
		{
			const auto	oActivateSeamOk			=	activateSeam(p_oSeam, label);
			if (oActivateSeamOk == false) {
				return false;
			} // if
		}

        // change priority of worker threads - hardware parameters are armed at this point
        if (!m_simulationStation)
        {
            for (auto &worker : m_oWorkers)
            {
                const bool liveMode = m_oState == State::eLiveMode;
                worker.setRtPriority(!liveMode && m_oDeviceParameter.getRealTimeGraphProcessing(), liveMode ? 0 : 1);
            }
        }


		// Send to host ids of measureTasks of all seamintervals of seam to be inspected. Also set external product data in filters and arm seam start.

		const auto&	rProductId				=	m_pActiveProduct->m_rProductData.productID();
		const auto	oProductNb				=	m_pActiveProduct->m_oProductNb;
		const auto	oNbTriggersPerSeam		=	m_pActiveSeam->m_oLength/*[um]*/ / m_pActiveSeam->m_oTriggerDelta/*[um]*/;
		auto		oMTaskIds				=	std::vector<UUID>();
		auto		oSourceFiltersUsed		=	SourceFilterType{};
		auto		oSensorIds				=	std::vector<int>{};

		m_oExternalProductData	= ProductData (
			p_oSeamseries,
			p_oSeam,
			m_pActiveSeam->m_oVelocity,
			m_pActiveSeam->m_oTriggerDelta,
			m_pActiveSeam->m_oLength / m_pActiveSeam->m_oTriggerDelta,
			m_oReferenceCurves.getRefCurveMap(),
			m_pActiveSeam->m_oLength,
			m_pActiveSeam->m_oDirection,
            m_pActiveSeam->m_oThicknessLeft,
            m_pActiveSeam->m_oThicknessRight,
            m_pActiveSeam->m_oTargetDifference,
            m_pActiveSeam->m_oRoiX,
            m_pActiveSeam->m_oRoiY,
            m_pActiveSeam->m_oRoiW,
            m_pActiveSeam->m_oRoiH);	///< external product data, eg velocity
	
		oMTaskIds.reserve(m_pActiveSeam->m_oSeamIntervals.size());
		oSensorIds.reserve(16); // some image and sample filters

		for(auto oItSeamInterval = std::begin(m_pActiveSeam->m_oSeamIntervals); oItSeamInterval != std::end(m_pActiveSeam->m_oSeamIntervals); ++oItSeamInterval) {
			// set parameter set and external data in filters of seam interval and collect measure task ids

            auto			oActiveGraphAssistent	=	GraphAssistent	( oItSeamInterval->getGraph() );
			const auto&		rParamSetId				=	oItSeamInterval->m_pMeasureTask->parametersatzID();
            if (g_oDebugTimings)
            {
                BaseFilter::resetProcessingCounter();
                ResetFilterIndexesVisitor oResetFilterIndexesVisitor;
                oItSeamInterval->getGraph()->control(oResetFilterIndexesVisitor);
            }

            if (m_oDeviceParameter.getLogAllFilterProcessingTime())
            {
                SetAlwaysEnableTimingsVisitor oSetAlwaysEnableTimingsVisitor (true);
                oItSeamInterval->getGraph()->control(oSetAlwaysEnableTimingsVisitor);
            }
            updateHardwareData(oItSeamInterval->getGraph());
            oActiveGraphAssistent.delayedSetExternalData(m_externalHardwareData);
            oActiveGraphAssistent.delayedSetExternalData(m_oExternalProductData);			// nb: do this before arm of seam start and setParameterSet
            if (m_lastSetParameterSet != rParamSetId)
            {
                oActiveGraphAssistent.delayedSetParameterSet(rParamSetId, m_oParameterSetMap);	// will be done again on seam interval change, but might be needed in arm functions
                m_lastSetParameterSet = rParamSetId;
            }

            oActiveGraphAssistent.delayedArm(eSeamStart);						// after setExternalData, so external product data is already set in filters. arm(eSeamStart) needed here, called again in changeSeamInterval()
			oMTaskIds.push_back(oItSeamInterval->m_pMeasureTask->taskID());

            // set canvas in filters
			oActiveGraphAssistent.delayedSetCanvas(m_oCanvasBuffer.data());
            oActiveGraphAssistent.apply();
            // collect sensor ids of all source filters within seam

			oSourceFiltersUsed		=	SourceFilterType( oSourceFiltersUsed | oActiveGraphAssistent.getSourceFilterTypes() );
			const auto&	rSensorIds	=	oActiveGraphAssistent.getSensorIds();
			oSensorIds.insert(std::end(oSensorIds), std::begin(rSensorIds), std::end(rSensorIds));
		} // for

		changeSeamInterval(0/*first interval*/);

		UUID oSeamId = m_pActiveSeam->m_pMeasureTask->taskID();
		getResultSrv().inspectionStarted(rProductId, m_oActiveProductInstanceId, oProductNb, oMTaskIds, Range(), p_oSeam, m_pActiveSeam->m_oTriggerDelta, oSeamId, m_activeSeamLabel);
        m_imageSender->clear();
		getVideoRecorderSrv().seamStart(SeamData( p_oSeamseries, p_oSeam, m_pActiveSeam->m_oTriggerDelta, m_activeSeamLabel)); // signal seam start
		m_pActiveProduct->seArm( eSeamStart, interface::tSeamIndex( p_oSeamseries, p_oSeam, -1 ) ); // signal seam start to the sum errors

		auto		oContext				=	TriggerContext	( 0, 0, 0, oProductNb, p_oSeamseries, p_oSeam ); // seam start at img nb 0
		oContext.setCycleCount(++m_triggerCycle);
		const auto		oTriggerSource			=	int				( m_pActiveProduct->m_rProductData.triggerSource() );
		const auto&		rActiveMeasureTask		=	*m_pActiveSeamInterval->m_pMeasureTask; // nb: trigger delta and velocity are equal for all seam intervals
		
		const auto oTimeDelta_ns = static_cast<unsigned int>( rActiveMeasureTask.getTimeDelta() * 1000.0)/*[us]*/ * 1000u; /*[us]->[ns]*/  // (here the time is cast to the integer values of us )
		const auto		oTriggerInterval		=	TriggerInterval ( oTimeDelta_ns, 
																	  m_infiniteNumberOfTriggers ? INT_MAX : oNbTriggersPerSeam,  m_pActiveSeam->m_oTriggerDelta, m_oState);
		m_oInspectTimer.resetAccumulatedTimings();

        if (oSensorIds.empty() && m_oState == eLiveMode)
        {
            // no sensor ids && live mode -> trigger all sensors - due to host init bug
            if (m_hasHardwareCamera)
            {
                oSensorIds.push_back(eImageSensorDefault);
                m_oLiveModeRequiresCamera = true;
            }
            if ((m_IDM_Device1Enable) || (m_CLS2_Device1Enable))
            {
                oSensorIds.push_back(eIDMSpectrumLine);
                oSensorIds.push_back(eIDMWeldingDepth);
            }
            if (m_LWM40_No1_Enable)
            {
                oSensorIds.push_back(eLWM40_1_Plasma);
                oSensorIds.push_back(eLWM40_1_Temperature);
                oSensorIds.push_back(eLWM40_1_BackReflection);
                oSensorIds.push_back(eLWM40_1_AnalogInput);
            }
            oSensorIds.push_back(eExternSensorDefault);
        }
            
        g_oUseScanmasterPosition = false;
        
        if (m_scanlabScannerEnable)
        {
            auto & rCalibData = system::CalibDataSingleton::getCalibrationData(math::SensorId::eSensorId0);
            auto isImageSensorId = [](int sensor){return sensor >= eImageSensorMin && sensor <= eImageSensorMax;};
            
            if ( (rCalibData.hasCameraCorrectionGrid() && std::find_if(oSensorIds.begin(), oSensorIds.end(), isImageSensorId) != oSensorIds.end()) 
                || (rCalibData.hasIDMCorrectionGrid() && std::find(oSensorIds.begin(), oSensorIds.end(), eIDMWeldingDepth) != oSensorIds.end())
            )
            {
                //enable the scanner position sensor even if not directly used in the graph, so that the correct calibration can be used 
                if (std::find(oSensorIds.begin(), oSensorIds.end(), eScannerXPosition) == oSensorIds.end())
                {
                    oSensorIds.push_back(eScannerXPosition);
                }                
                if (std::find(oSensorIds.begin(), oSensorIds.end(), eScannerYPosition) == oSensorIds.end())
                {
                    oSensorIds.push_back(eScannerYPosition);
                }
                // in signal adapter we will need to check the scanmaster position
                g_oUseScanmasterPosition = true;
            }
        }
            

		wmLog(eDebug, "%s - trigger distance [ms]: %f\tnb of triggers: %u\n", __FUNCTION__, oTriggerInterval.triggerDistance()*0.000001, oTriggerInterval.nbTriggers() );

        if (!m_simulationStation)
        {
            m_imageSender->reserveForSamples(oNbTriggersPerSeam * oSensorIds.size());
            getTriggerCmdSrv().burst(oSensorIds, oContext, oTriggerSource, oTriggerInterval);
        }
        m_oSensorIds = std::move(oSensorIds);
	} // try
	catch(...) {
		logExcpetion(__FUNCTION__, std::current_exception());
	} // catch

	return true;
} // startInspect

void InspectManager::triggerSingle(const interface::TriggerContext &context)
{
    const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};
    getTriggerCmdSrv().single(m_oSensorIds, context);
}

void InspectManager::stopInspect() {
    const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};
    stopInspectInternal();
} // stopInspect

void InspectManager::stopInspectInternal()
{
	try {

		getTriggerCmdSrv().cancel(std::vector<int>(1, -1));     // cancel all sensors

		joinWorkers();	                                        // join threads and record last frames
		if (m_oState != State::eLiveMode ) {
			wmLogTr( eInfo, "QnxMsg.Workflow.InspectStop", "Inspection stopped.\n");
		} // if

        if (!m_pActiveGraph)
        {
            // nothing to stop
            return;
        }
		auto		oActiveGraphAssistent	=	GraphAssistent( m_pActiveGraph );
        wmLog(eDebug,"StopInspect: number of signaled images= %d, number of images skipped from sensor = %d, number of images skipped in inspection = %d \n",
            m_oNbSeamSignaled, m_oNumImagesSkippedFromSensor, m_oNumImagesSkippedInInspection
        );
        oActiveGraphAssistent.delayedLogProcessingTime();
        oActiveGraphAssistent.delayedArm(eSeamEnd);
        oActiveGraphAssistent.apply();

		// process ResultsHandler again - arm might cause another result
		auto args = fliplib::PipeGroupEventArgs(0, nullptr, m_lastProcessedImage);
		m_oResultHandler.proceedGroup(nullptr, args);
		// ok, now signal also the sum-errors, that we are at the end of the seam ...
    	m_pActiveProduct->seArm( eSeamIntervalEnd, 	interface::tSeamIndex( m_pActiveSeamSeries->m_oNumber, m_pActiveSeam->m_oNumber, m_pActiveSeamInterval->m_oNumber ) );					// signal the sum-errors, that the old interval has ended
		m_pActiveProduct->seArm( eSeamEnd, 			interface::tSeamIndex( m_pActiveSeamSeries->m_oNumber, m_pActiveSeam->m_oNumber, -1 ) );

		releaseDataPipesAndResultHandler(m_pActiveGraph); 		// done on seam interval change and on stopInspect

		m_oNbSeamSignaled	= 0;                                // reset counter
		m_oNbSeamJoined		= 0;                                // reset counter
		m_oResultHandler.setCounter(0);                         // also done for graph on seam interval arm
		m_oResultHandler.setLastImageProcessed(0);
        m_lastProcessedImage = -1; 
        m_lastSkippedImageNumber = -1;
        m_oNumImagesSkippedFromSensor = 0;
        m_oNumImagesSkippedInInspection = 0;

		m_pActiveGraph			=	nullptr;
		m_pActiveSeamInterval	=	nullptr;
		m_pActiveSeam			=	nullptr;
        m_lastSetParameterSet = Poco::UUID::null();

		// empty data queues

		while (!m_oImageQueue.empty())
		{
			m_oImageQueue.pop();
		}
		m_oSampleQueues.clear();

		getVideoRecorderSrv().seamEnd(); // signal seam end
		getResultSrv().inspectionEnded(std::vector<UUID>());

		m_oInspectTimer.logAccumulatedTimings();

	} // try
	catch(...) {
		logExcpetion(__FUNCTION__, std::current_exception());
	} // catch
}



// Starte eine Nahtfolge.
bool InspectManager::startSeamseries(int p_oSeamseries) {
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};

	poco_assert(nullptr != m_pActiveProduct);
	if (m_oState != State::eLiveMode ) {
		wmLogTr( eInfo, "QnxMsg.Workflow.SseriesStart", "Seamseries started.\n");
	} // if

    const auto	oActivatSeamSeriesOk	=	activateSeamSeriesInternal(p_oSeamseries);
	if (oActivatSeamSeriesOk == false) {
		return false;
	} // if

	return true;
} // startSeamseries



void InspectManager::data(int p_oSensorId, const TriggerContext& p_rTriggerContext, const BImage& p_rImage) {
    system::ElapsedTimer timer;
    const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};

	if (m_oState == eCalibrate) {
		return;
	} // if
	if (m_pActiveSeamInterval == nullptr) {
		wmLog(eDebug, "Could not process image %i. Inspection has been stopped.\n", p_rTriggerContext.imageNumber());
		return;
	} // if

    if (!m_simulationStation && m_pActiveSeam && m_triggerCycle != p_rTriggerContext.cycleCount())
    {
        // simulation uses single instead of burst. Thus keeping a cycle doesn't make sense.
        wmLog(eDebug, "Could not process image %i. It is from previous inspection cycle.\n", p_rTriggerContext.imageNumber());
        return;
    }
    
    bool validImageNumber = true;
    
    ProcessingMode mode{ProcessingMode::Normal};
    if (!m_simulationStation)
    {
        int differenceFromExpectedImageNumber = p_rTriggerContext.imageNumber() - ( m_lastProcessedImage + 1);
        if (differenceFromExpectedImageNumber > 0)
        {
            wmLog(eError, "Number of images skipped = %d, expected image number %d, got %d \n", 
                  differenceFromExpectedImageNumber, 
                m_lastProcessedImage+1, p_rTriggerContext.imageNumber());

            mode = ProcessingMode::MissingImage;
            
            for (int i = m_lastProcessedImage + 1; i < p_rTriggerContext.imageNumber(); i++)
            {
                if (i > m_lastSkippedImageNumber)
                {
                    m_oNumImagesSkippedFromSensor++;
                    m_lastSkippedImageNumber = i;
                }
            }
        }
        
        if (differenceFromExpectedImageNumber < 0)
        {
            wmLog(eError, "Inconsistency with received image number! ImageNumber in context %d Last Processed Image %d\n", p_rTriggerContext.imageNumber(), m_lastProcessedImage);
            validImageNumber = false; //the image number can only go forward (except during simulation), this is an exceptional case 
        }
    }
    //update seam position 
	checkSeamIntervalChange(p_rTriggerContext.imageNumber());

    // simulation is always overtriggered
	const auto	oTimeDelta	=	static_cast<int>(m_pActiveSeamInterval->m_pMeasureTask->getTimeDelta() * 1000/*[us]*/);
	if (!m_simulationStation)
    {
        auto debugOvertriggered = [oTimeDelta]
        {
            std::ostringstream oSt;
            oSt << "System too slow for trigger freq.: " << oTimeDelta / 1000 /*to [ms]*/ << " [ms]. Check processing time, exposure, ...\n";
            wmLog(g_oDebugTimings ? eWarning: eDebug, oSt.str());
        };
        auto warningImageSkipped = [&p_rTriggerContext]
        {
            if (g_oDebugTimings)
            {
                wmLog(eDebug, "Image %d skipped in inspection.\n", p_rTriggerContext.imageNumber() );
            }
            wmLogTr(eWarning, "QnxMsg.Workflow.ImageSkipped", "Image skipped in inspection.\n");
        };
        switch (m_oInspectTimer.checkOvertriggering(oTimeDelta, p_rTriggerContext.imageNumber(),
            m_oDeviceParameter.getConservativeCheckOvertriggering(), (int)(1000*m_oDeviceParameter.getToleranceOvertriggering_ms())))
        {
        case InspectTimer::OvertriggeringResult::EverythingOk:
            break;
        case InspectTimer::OvertriggeringResult::Dangerous:
            debugOvertriggered();
            break;
        case InspectTimer::OvertriggeringResult::Critical:
            mode = ProcessingMode::Overtriggered;
            debugOvertriggered();
            warningImageSkipped();
            break;
        case InspectTimer::OvertriggeringResult::Overtriggered:
        {
            mode = ProcessingMode::Overtriggered;
            debugOvertriggered();
            warningImageSkipped();
            wmLogTr(eWarning, "QnxMsg.Workflow.OvertrigImg", "Image inspection overtriggered.\n");
            break;
        }
        }
        
        //safety check in parallel case: skip image if all the multiple images are still being processed
        if ( mode != ProcessingMode::Overtriggered && g_oNbPar > 1)
        {
            auto oProcessedImagesCount = GraphAssistent(m_pActiveGraph).getProcessedImagesCounter();
            auto oNumImagesBeingProcessed = std::count_if(oProcessedImagesCount.begin(), oProcessedImagesCount.end(), [](std::pair<int,int> entry){return entry.second> 0;});
            if (oNumImagesBeingProcessed >= (int) g_oNbPar)
            {
                mode = ProcessingMode::Overtriggered;
                GraphAssistent::logProcessedImagesCount(precitec::eWarning, oProcessedImagesCount);
                warningImageSkipped();
            }
        }

	}

	if (mode == ProcessingMode::Overtriggered)
    {
        m_oNumImagesSkippedInInspection++;
        m_lastSkippedImageNumber = p_rTriggerContext.imageNumber();
    }
    
    if (!validImageNumber)
    {
        mode = ProcessingMode::OutOfOrderImage;
    }
	bool processingStarted = processImage(p_oSensorId, p_rTriggerContext, p_rImage, mode);

    updateInspectManagerTime(timer.elapsed(), processingStarted);

    
    // this is only for the gui in live-mode, to visualize the current processing time. does not work for oct-track, yet.
    if(m_pActiveProduct->m_rProductData.defaultProduct() && p_rTriggerContext.imageNumber() % 10 == 0)
    {
        updateProductInfo();
    }
    
} // data



void InspectManager::data(int p_oSensorId, const TriggerContext& p_rTriggerContext, const Sample& p_rSample) {
    system::ElapsedTimer timer;
    const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};

	if (m_oState == eCalibrate) {
		return;
	} // if
	if (m_pActiveGraph == nullptr) {
		wmLog(eDebug, "Could not process sample %i. Inspection has been stopped.\n", p_rTriggerContext.imageNumber());
		return;
	} // if

    if (!m_simulationStation && m_pActiveSeam && m_triggerCycle != p_rTriggerContext.cycleCount())
    {
        // simulation uses single instead of burst. Thus keeping a cycle doesn't make sense.
        wmLog(eDebug, "Could not process sample %i. It is from previous inspection cycle.\n", p_rTriggerContext.imageNumber());
        return;
    }

	checkSeamIntervalChange(p_rTriggerContext.imageNumber());
	bool processingStarted = processSample(p_oSensorId, p_rTriggerContext, p_rSample);
    
    updateInspectManagerTime(timer.elapsed(), processingStarted);
    
    // this is only for the gui in live-mode, to visualize the current processing time. not tested oct-track, yet.
    if(m_pActiveProduct->m_rProductData.defaultProduct() && p_rTriggerContext.imageNumber() % 10 == 0)
    {
        updateProductInfo();
    }
    
;
} // data



void InspectManager::updateProductInfo() {
	poco_assert(m_pActiveProduct != nullptr);
	poco_assert(m_pActiveSeamInterval != nullptr);

	int productSeriesNr = m_pActiveProduct->m_oProductNb;
	int productHwNr = m_pActiveProduct->m_rProductData.productType();
	if(m_pActiveProduct->m_rProductData.defaultProduct())
	{
		productSeriesNr = -1;
	}

	auto	oProductInfo	=	ProductInfo	( m_pActiveProduct->m_rProductData.productID(), m_pActiveProduct->m_rProductData.name(), *m_pActiveSeamInterval->m_pMeasureTask,productHwNr,productSeriesNr );
    oProductInfo.m_oProcessingTime = m_oInspectTimer.processingTime();
    oProductInfo.m_oSeamLabel = m_activeSeamLabel;
	getSystemStatusSrv().signalProductInfo(oProductInfo);
} // updateProductInfo



void InspectManager::startLiveMode() {
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};
	//m_pCentralDeviceManager->lock( true ); // NOT done because line laser is often adjusted during live mode

	poco_assert(m_pActiveProduct != nullptr);

	// create seamdata for video recorder folder creation

	seamDataVector_t oSeamData;
	for(auto oItSeamSeries = std::begin(m_pActiveProduct->m_oSeamSeries); oItSeamSeries != std::end(m_pActiveProduct->m_oSeamSeries); ++oItSeamSeries) {
		for(auto oItSeam = std::begin(oItSeamSeries->second.m_oSeams); oItSeam != std::end(oItSeamSeries->second.m_oSeams); ++oItSeam) {
			oSeamData.push_back(SeamData( oItSeamSeries->second.m_oNumber, oItSeam->second.m_oNumber, oItSeam->second.m_oTriggerDelta ));
		} // for
	} // for

	const auto&	rProductData		=	m_pActiveProduct->m_rProductData;
	const auto	oProductInstData	=	ProductInstData	(	rProductData.name(),
															rProductData.productID(),
															m_oActiveProductInstanceId,
															rProductData.productType(),
															m_pActiveProduct->m_oProductNb,
															std::string{},
															oSeamData);

	getVideoRecorderSrv().startLiveMode(oProductInstData);
	getResultSrv().inspectionAutomaticStart(m_pActiveProduct->m_rProductData.productID(), m_oActiveProductInstanceId, {});
} // startLiveMode



void InspectManager::stopLiveMode() {
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};
	getVideoRecorderSrv().stopLiveMode();

	if ( m_pActiveProduct != nullptr )
	{
		getResultSrv().inspectionAutomaticStop(m_pActiveProduct->m_rProductData.productID(), m_oActiveProductInstanceId);
	}

	//m_pCentralDeviceManager->lock( false ); // NOT done because line laser is often adjusted during live mode

	m_pActiveProduct->setExtendedProductInfo({});
	m_pActiveProduct		=	nullptr;
	m_pActiveSeamSeries		=	nullptr;
	m_pActiveSeam			=	nullptr;
    m_activeSeamLabel = {};
    m_seamReuseCounters.clear();
} // stopLiveMode



void InspectManager::startAutomaticMode() {
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};

	// lock the device manager
	m_pCentralDeviceManager->lock( true );

	poco_assert(m_pActiveProduct != nullptr);

	// create seamdata for video recorder folder creation

	seamDataVector_t oSeamData;
	for(auto oItSeamSeries = std::begin(m_pActiveProduct->m_oSeamSeries); oItSeamSeries != std::end(m_pActiveProduct->m_oSeamSeries); ++oItSeamSeries) {
		for(auto oItSeam = std::begin(oItSeamSeries->second.m_oSeams); oItSeam != std::end(oItSeamSeries->second.m_oSeams); ++oItSeam) {
			oSeamData.push_back(SeamData( oItSeamSeries->second.m_oNumber, oItSeam->second.m_oNumber, oItSeam->second.m_oTriggerDelta ));
		} // for
	} // for

	// reset sum errors

	m_pActiveProduct->seResetAll();

	// send product info to video recorder

	const auto&	rProductData		=	m_pActiveProduct->m_rProductData;
	const auto	oProductInstData	=	ProductInstData	(	rProductData.name(),
															rProductData.productID(),
															m_oActiveProductInstanceId,
															rProductData.productType(),
															m_pActiveProduct->m_oProductNb,
															m_pActiveProduct->extendedProductInfo(),
															oSeamData);
	getVideoRecorderSrv().startAutomaticMode(oProductInstData);

    if (!getNoHWParaAxis())
    {
        armHwParameters(m_pActiveProduct->m_rProductData.hwParameterSatzID());
        wmLog(eDebug, "HW armed on product level: '%s'\n", m_pActiveProduct->m_rProductData.hwParameterSatzID().toString().c_str());
    }

	getResultSrv().inspectionAutomaticStart(m_pActiveProduct->m_rProductData.productID(), m_oActiveProductInstanceId, m_pActiveProduct->extendedProductInfo());
} // startAutomaticMode



void InspectManager::stopAutomaticMode() {
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};

	// stop sensors if not yet done by stop inspect or if called from not-ready
	getTriggerCmdSrv().cancel(std::vector<int>(1, -1)); // cancel all sensors

	if (m_pActiveGraph != nullptr)
	{
		stopInspectInternal();
		wmLog(eWarning, "'stopAutomaticMode' called with active seam. Inspection stopped.\n");
	} // if

	// unlock the device manager
	if (m_pCentralDeviceManager != nullptr) // nullptr may happen on stopSimulation with invalid graph. Reason unknown.
		m_pCentralDeviceManager->lock( false );

	getVideoRecorderSrv().stopAutomaticMode();

	if ( m_pActiveProduct != nullptr )
	{
		getResultSrv().inspectionAutomaticStop(m_pActiveProduct->m_rProductData.productID(), m_oActiveProductInstanceId);
	}

	m_pActiveProduct->setExtendedProductInfo({});
	m_pActiveProduct		=	nullptr;
	m_pActiveSeamSeries		=	nullptr;
	m_pActiveSeam			=	nullptr;
    m_activeSeamLabel = {};
    m_seamReuseCounters.clear();

} // stopAutomaticMode



void InspectManager::setState(State p_oState) {
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};

	poco_assert(p_oState < State::numStates);
	m_oState = p_oState;
	m_oResultHandler.setState(p_oState);
} // setState



void InspectManager::updateHwParameter( const UUID &p_rParamSetId, const Key p_oKey )
{
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};

	// OK, lets get the individual key value in this specific hardware parameter set from the database ...
	ParameterList oList = getDBSrv().getHardwareParameter( p_rParamSetId, p_oKey );
	// Apply new hardware parameter to devices
	m_oHwParameters.applyHwParam( oList[0], m_pCentralDeviceManager );
	// Exchange the key
	m_oHwParameters.updateHwParamSet( p_rParamSetId, oList[0] );
} // updateHwParameter

void InspectManager::updateHardwareData(const FilterGraph* graph)
{
    const UUID filterId{"3041d376-aebf-11ed-afa1-0242ac120002"};;
    const auto& filterMap = graph->getFilterMap();
    for (const auto& filterHandler: filterMap)
    {
        const auto filter = filterHandler.second->getFilter();
        if (filter->filterID() == filterId)
        {
            const auto& parameters = filter->getParameters();
            const auto appId = parameters.getParameter("AppName").convert<unsigned int>();
            const auto parameterName = parameters.getParameter("ParameterName").convert<std::string>();
            const auto parameterValue = m_pCentralDeviceManager->getHardwareParameter(appId, parameterName);
            if (parameterValue.has_value())
            {
                m_externalHardwareData.set(appId, parameterName, parameterValue.value());
            }
        }
    }
}

void InspectManager::setNoHWParaAxis( bool p_oNoHWParaAxis)
{
	m_oNoHWParaAxis = p_oNoHWParaAxis;

} // setNoHWParaAxis



bool InspectManager::getNoHWParaAxis()
{
	return m_oNoHWParaAxis;

} // getNoHWParaAxis



void InspectManager::resetSumErrors() {
	const Poco::ScopedLock<Poco::FastMutex> oScopedLock{m_oManSync};

	wmLog(eDebug, "Reset sumErrors...\n");
    if (m_pActiveProduct)
    {
        m_pActiveProduct->seResetAll();
    }
}


// calibration

math::CalibrationData & InspectManager::getCalibrationData(const int p_oSensorID)
{
	return system::CalibDataSingleton::getCalibrationDataReference(math::SensorId(p_oSensorID));
}


//called by  CalibrationManager::sendCalibDataChangedSignal via CalibrationCoordinatesPublishServer
bool InspectManager::loadCalibDataAfterSignal(int p_oSensorID, bool p_oInit )
{
    wmLog(eDebug, "InspectManager::loadCalibDataAfterSignal(sensorId %d, init %d ) \n", p_oSensorID, p_oInit);
    math::CalibrationData &rCalibData = getCalibrationData(p_oSensorID);
    assert(rCalibData.validSensorID(p_oSensorID));
    
    if (rCalibData.isInitialized() )
    {
        wmLog(eInfo, "InspectManager: overwriting calibration (id %d)\n", p_oSensorID);
    }
    if ( m_simulationStation )
    {
        wmLog(eDebug, "Linux simulation\n");
    }
    
    if (p_oInit)
    {
        std::cout << "Setting sensormodel to undefined, waiting for set3dcoords\n";
        rCalibData.setSensorModel(SensorModel::eUndefined); 
        //I am waiting for set3DCoords
    }
    else
    {
        wmLog(eWarning, "InspectManager loadCalibDataAfterSignal called without initialization, this should happen only when modifying TCP\n");
        assert(false);
        //rCalibData.loadConfig();
        assert(assertCalibData(p_oSensorID));
        
    }
    return true;
   
}

//called by  CalibrationManager::sendCalibDataChangedSignal via CalibrationCoordinatesPublishServer
//overwrite present calibration data with what is provided here
bool InspectManager::reloadCalibData(int p_oSensorID, math::Calibration3DCoords p_o3DCoords, math::CalibrationParamMap p_oCalibrationParameters)
{
    wmLog(eDebug, "InspectManager::reloadCalibData\n");
    std::cout << "Received 3dcoords for sensor " << p_oSensorID << "\n";

    math::CalibrationData &rCalibData = getCalibrationData(p_oSensorID);
    
    if (rCalibData.getSensorId() != p_oSensorID)
    {
        wmLog(eError, "Error in calibdatasingleton , wrong sensor id\n");
        assert(false && "error in calibdatasingleton sensorid");
        return false;
    }

    bool ok = rCalibData.reload(p_o3DCoords, p_oCalibrationParameters);

    std::ostringstream oMsg;

    rCalibData.showData(oMsg);

    std::cout << oMsg.str() << std::endl;

    wmLog(eDebug, "Calib Data Reloaded %s" + oMsg.str(), ok? "OK":"ERROR");



    assert(ok);    
    assert(rCalibData.isInitialized());
    //assert(assertCalibData(p_oSensorID));
    
    return ok;
        

}

bool InspectManager::updateCorrectionGrid(int p_oSensorID, coordinates::CalibrationCameraCorrectionContainer p_oCameraCorrectionContainer, coordinates::CalibrationIDMCorrectionContainer p_oIDMCorrectionContainer)
{
    math::CalibrationData &rCalibData = getCalibrationData(p_oSensorID);
    // TODO: special value to reset correction?
    rCalibData.setCalibrationCorrectionContainer( p_oCameraCorrectionContainer );
    rCalibData.setCalibrationIDMCorrectionContainer(p_oIDMCorrectionContainer);

    // FIXME in this moment calibrationData of calibmanager and inspect manager can go out of sync 
    // (scanner coordinate unknown)
    
    return true;

}

bool InspectManager::assertCalibData(int p_oSensorID)

{

    const math::CalibrationData &rCalibData = getCalibrationData(p_oSensorID);

    bool ok=  rCalibData.hasData(); 
    //should be
    //rCalibData.checkCalibrationValuesConsistency(rCalibData.CALIB_VALUES_TOL_MIN, eWarning , /*checkCoordinates*/ true);

    assert(ok);
    return ok;
}


// graph handling


graph_map_t::const_iterator InspectManager::storeGraphAndParamterSet(const Poco::UUID &p_rMeasureTaskId, const Poco::UUID &p_rGraphId,  const Poco::UUID &p_rParamSetId) {
	const auto	oCItGraph					=	cacheGraph(p_rMeasureTaskId, p_rGraphId); // by default id of empty graph if no crosswise action

	if (m_oUuidsBuilt.find(p_rParamSetId) == std::end(m_oUuidsBuilt) && m_oGraphs.find(p_rGraphId) != std::end(m_oGraphs)) {
		m_oParameterSetMap.erase(p_rParamSetId); // overwrite if existing
		m_oUuidsBuilt.insert(p_rParamSetId);
		cacheParamSet(p_rParamSetId, oCItGraph->second.get()); 	// cache parameter set if not yet cached. // takes around 20 ms
	} // if

	return oCItGraph;
} // storeGraphAndParamterSet



graph_map_t::const_iterator InspectManager::cacheGraph(const UUID &p_rMeasureTaskId, const UUID &p_rGraphId) {
	// build graph if not yet in cache

	poco_assert(p_rGraphId.isNull() == false);
	const auto oFoundBuilt	= m_oUuidsBuilt.find(p_rGraphId); // build every graph once per product - filter instance could have changed

	if (oFoundBuilt == std::end(m_oUuidsBuilt)) {
		const auto oGraphList = getDBSrv().getGraph(p_rMeasureTaskId, p_rGraphId);
        UpFilterGraph oUpGraph;
		if(oGraphList.empty() == true)
        {
            throw Exception{ std::string{ "getDBSrv().getGraph(" } + p_rMeasureTaskId.toString() + ", "+ p_rGraphId.toString() + ") failed."};
        } // if
        else
        {
            oUpGraph = m_oGraphBuilder.build(oGraphList.front());
        }

		poco_assert(oUpGraph.get() != nullptr);  // nullptr may happen for different reasons, eg wrong dll or bad db state

		m_oUuidsBuilt.insert(p_rGraphId);
		m_oGraphs[p_rGraphId]	=	std::move(oUpGraph);
	} // if

	return m_oGraphs.find(p_rGraphId);
} // cacheGraph



void InspectManager::cacheParamSet(const UUID& p_rParamSetId, const FilterGraph* p_pGraph) {
	if (p_rParamSetId.isNull()) {
		return; // do not cache live mode parameter set
	} // if
	paramSet_t oParamSet;
	const auto oItFirstFilter		( std::begin(p_pGraph->getFilterMap()) );
	const auto oItLastFilter		( std::end(p_pGraph->getFilterMap()) );
    auto parameters = getDBSrv().getParameterSatzForAllFilters(p_rParamSetId);
	for(auto oItFilter = oItFirstFilter; oItFilter != oItLastFilter; ++oItFilter) { // could be another visitor with much state
		const UUID&			rFilterId				( oItFilter->first );
        auto it = std::find_if(parameters.begin(), parameters.end(),
            [&rFilterId] (const FilterParametersContainer &c)
            {
                return c.filterID() == rFilterId;
            }
        );
        ParameterList oParameterListFilter{};
        if (it != parameters.end())
        {
            oParameterListFilter = it->parameters();
        }
		oParamSet.emplace(rFilterId, oParameterListFilter);

		m_oReferenceCurves.requestAndStoreRefCurves(oParameterListFilter); // request and store reference curve(s) if graph contains one or more ref curve filters
	} // for
	const bool	oParamSetNotFound		( m_oParameterSetMap.find(p_rParamSetId) == std::end(m_oParameterSetMap) );
	if (oParamSetNotFound == true) {
		m_oParameterSetMap.emplace(p_rParamSetId, oParamSet);
	} // if
} // cacheParamSet



void InspectManager::changeSeamInterval(int p_oActualPos) {
	std::ostringstream oTime; // debug
	{ // timer scope
    const auto  oFirstSeamIntervalNb    = int{ -1 };
	const auto	oTimer					= system::ScopedTimer(__FUNCTION__, oTime);
	const auto	pOldSeamInterval		= m_pActiveSeamInterval;
	const auto	oOldSeamIntervalNb		= pOldSeamInterval == nullptr ? oFirstSeamIntervalNb : pOldSeamInterval->m_oNumber;

	poco_assert_dbg(m_pActiveSeam != nullptr);

	int oCountSeamIntervals = m_pActiveSeam->m_oSeamIntervals.size();
	for (int i = 0; i < oCountSeamIntervals; ++i)
	{
		const auto pSeaminterval				= &m_pActiveSeam->m_oSeamIntervals[i];
        poco_assert(pSeaminterval != nullptr);

		if(p_oActualPos >= pSeaminterval->m_oStartPositionInSeam && p_oActualPos <= pSeaminterval->m_oEndPositionInSeam)
		{
			//yeah, seaminterval found
			m_pActiveSeamInterval = pSeaminterval;
			m_pActiveGraph						= m_pActiveSeamInterval->getGraph();

			break;
		}
	} // for

	// if old and new seam interval are identical, we do not have to do anything ...
	if (oOldSeamIntervalNb != oFirstSeamIntervalNb && oOldSeamIntervalNb == m_pActiveSeamInterval->m_oNumber) { return; } 
	
	if (m_oState != eLiveMode) {
		wmLogTr(eInfo, "QnxMsg.Workflow.ChangeInterval", "Seam interval change: %i -> %i.\n", oOldSeamIntervalNb + 1, m_pActiveSeamInterval->m_oNumber + 1); // GUI +1 rule
	} // if

    // for previous interval, if existant, release input and output handlers

    if (oOldSeamIntervalNb != oFirstSeamIntervalNb)
    {
    	joinWorkers();	                                                        // before changing parameter set or even the whole graph, wait for work in progress
        releaseDataPipesAndResultHandler(pOldSeamInterval->getGraph()); 		// done on seam interval change and on stopInspect

    } // if

	const auto&	rActiveMeasureTask		= *m_pActiveSeamInterval->m_pMeasureTask;
	const auto& rParamSetId				= rActiveMeasureTask.parametersatzID();
	const auto& rHwParamSetId			= rActiveMeasureTask.hwParametersatzID();
	auto		oActiveGraphAssistent	= GraphAssistent( m_pActiveGraph );

    if (m_lastSetParameterSet != rParamSetId)
    {
        oActiveGraphAssistent.delayedSetParameterSet(rParamSetId, m_oParameterSetMap);	// update parameter set.
        m_lastSetParameterSet = rParamSetId;
    }
    oActiveGraphAssistent.delayedArm(eSeamIntervalChange);							// arm filters
    oActiveGraphAssistent.delayedSetCount(m_oNbSeamJoined);						// if graph has changed, the filter internal sync counter has to be updated

    oActiveGraphAssistent.apply();

    if (oOldSeamIntervalNb != oFirstSeamIntervalNb)
    {
    	m_pActiveProduct->seArm( eSeamIntervalEnd, interface::tSeamIndex(m_pActiveSeamSeries->m_oNumber, m_pActiveSeam->m_oNumber, oOldSeamIntervalNb ) );					// signal the sum-errors, that the old interval has ended
    }
    m_pActiveProduct->seArm( eSeamIntervalStart, interface::tSeamIndex(m_pActiveSeamSeries->m_oNumber, m_pActiveSeam->m_oNumber, m_pActiveSeamInterval->m_oNumber) );	// signal the sum-errors, that the new interval has startet

    // HW parameters only if WM is not in continuous mode
    if (!m_continuouslyModeActive)
    {
        armHwParameters(rHwParamSetId);
        //wmLog(eDebug, "HW armed on seam interval level: '%s'\n", rHwParamSetId.toString().c_str());
    }

	// get sensor ids for image and sample filters and connect data pipes accordingly

	const auto oSensorIdsUsed   =   oActiveGraphAssistent.getSourceFilterTypesAndIds();
	for (auto oSensorId : std::get<0/*image*/>(oSensorIdsUsed))
	{
		oActiveGraphAssistent.setInputPipe<ImageFrame>( oSensorId, &m_oPipeImageFrame );
	}

	oActiveGraphAssistent.setResultHandler( m_oResultHandler );
	m_oResultHandler.setCounter(m_oNbSeamJoined);							// if graph has changed, the filter internal sync counter has to be updated
	m_oResultHandler.setLastImageProcessed(m_oNbSeamJoined);
	m_oResultHandler.resetSignalCntGroupEvent(); 							// done for graph on seam interval change

	updateProductInfo();

	} // timer scope

	wmLog(eDebug, oTime.str());
} // changeSeamInterval



void InspectManager::checkSeamIntervalChange(int p_oImageNumber) {
	m_oCurrentPosSeam			= m_pActiveSeam->m_oTriggerDelta * p_oImageNumber;

	if ((int)m_oCurrentPosSeam < m_pActiveSeamInterval->m_oStartPositionInSeam || (int)m_oCurrentPosSeam > m_pActiveSeamInterval->m_oEndPositionInSeam){
		changeSeamInterval((int)m_oCurrentPosSeam);
	} // if
} // checkSeamIntervalChange



bool InspectManager::processImage(int p_oSensorId, const TriggerContext& p_rTriggerContext, const BImage& p_rImage, ProcessingMode mode) {
#if !defined(NDEBUG)
	wmLog(eDebug, "\t%s::BImage[%i]\t%i\n", __FUNCTION__, p_oSensorId, p_rTriggerContext.imageNumber());
#endif
	poco_assert_dbg(m_pActiveGraph != nullptr);

	//	1)	enqueue new frame
	//
	const auto		oImageContext			= makeImageContext(p_rTriggerContext);
    const auto	    oIdxWorkerCur           = p_rTriggerContext.imageNumber() % g_oNbPar;

    m_oImageQueue.push(std::make_tuple(p_oSensorId, ImageFrame{ oImageContext, p_rImage }, mode));  // performance: use emplace on newer gcc

	//	2)	if there is no sample source, c ontinue, otherwise dequeue sample or return (wait for sample)
	//
	if (!areAllSamplesQueued())
	{
		if (m_oImageQueue.size() < 3)
		{
			// still waiting for samples
			// in case we get three images in a row, we assume there is a graph error and continue
			return false;
		}
	}

	//	4)	now dequeue image and process data in worker thread. Save meta information.
	//
	m_oSampleFrames[oIdxWorkerCur]		= std::move(dequeueSamples());
	m_oTriggerContexts[oIdxWorkerCur]	=	std::move(p_rTriggerContext);
	const auto image = m_oImageQueue.front();
	m_oImageIds[oIdxWorkerCur]			=	std::get<0>(image);
    m_oImageFrames[oIdxWorkerCur]   	=   std::get<1>(image);
    m_oProcessingModes[oIdxWorkerCur]   =   std::get<2>(image);
    m_oImageQueue.pop();

    if (g_oDebugTimings) 
    {
        wmLog(eDebug, "ProcessImage: start worker %d (nb signaled %d) \n", oIdxWorkerCur, m_oNbSeamSignaled);
        GraphAssistent(m_pActiveGraph).logProcessedImagesCount(eDebug);
    }
	startProcessing(oIdxWorkerCur);
    return true;
} // processImage



bool InspectManager::processSample(int p_oSensorId, const TriggerContext& p_rTriggerContext, const Sample& p_rSample) {
#if !defined(NDEBUG)
	wmLog(eDebug, "\t%s::Sample[%i]\t%i\n", __FUNCTION__, p_oSensorId, p_rTriggerContext.imageNumber());
#endif
	poco_assert_dbg(m_pActiveGraph != nullptr);

	//	1)	enqueue new frame
	//
    const std::size_t	    oIdxWorkerCur           = p_rTriggerContext.imageNumber() % g_oNbPar;

    queueSample(p_oSensorId, p_rTriggerContext, p_rSample);
    if (!areAllSamplesQueued())
    {
    	const bool overSampled = std::any_of(m_oSampleQueues.begin(), m_oSampleQueues.end(),
			[] (const std::pair<int, std::queue<SampleFrame>> &queue)
    		{
    			return queue.second.size() >= 3;
    		});
		if (!overSampled)
		{
			// still waiting for samples
			// in case we get three samples in a row for the same sensor, we assume there is a graph error and continue
			return false;
		}
    }

	//	2)	if there is no image source, continue, otherwise dequeue image or return (wait for image)
    if (m_oImageQueue.empty() == false)
    {
        const auto image = m_oImageQueue.front();
        m_oImageIds[oIdxWorkerCur]			=	std::get<0>(image);
        m_oImageFrames[oIdxWorkerCur]   	=   std::get<1>(image);
        m_oProcessingModes[oIdxWorkerCur]   =   std::get<2>(image);
        m_oImageQueue.pop();
    } // if
    else
    {
        if (m_oPipeImageFrame.linked() || m_oLiveModeRequiresCamera )
        {
            return false; // no image available, exit (wait for image because imageSource in graph)
        }
    } // if

	//	4)	now dequeue sample and process data in worker thread. Save meta information.
	//
	m_oSampleFrames[oIdxWorkerCur]		= 	std::move(dequeueSamples());
    m_oTriggerContexts[oIdxWorkerCur]	=	std::move(p_rTriggerContext);


    if (g_oDebugTimings)
    {
        wmLog(eDebug, "ProcessSample: start worker %d (nb signaled %d) \n", oIdxWorkerCur, m_oNbSeamSignaled);
        GraphAssistent(m_pActiveGraph).logProcessedImagesCount(eDebug);
    }

	startProcessing(oIdxWorkerCur);
    return true;
} // processSample

void InspectManager::startProcessing(size_t oIdxWorkerCur)
{
	std::unique_ptr<SignalAdapter> signalAdapter{ new SignalAdapter{ oIdxWorkerCur, &m_oInspectTimer, &m_oPipeImageFrame, m_oPipesSampleFrame[oIdxWorkerCur].get(), m_pActiveGraph }};
	signalAdapter->setResultHandler(&m_oResultHandler);
    auto &triggerContext = m_oTriggerContexts[oIdxWorkerCur];
	signalAdapter->setImageNumber(triggerContext.imageNumber());
    signalAdapter->setImage(m_oImageFrames[oIdxWorkerCur]);
    signalAdapter->setSamples(m_oSampleFrames[oIdxWorkerCur]);
    auto processingMode = m_oProcessingModes[oIdxWorkerCur];
    signalAdapter->setProcessingMode(processingMode);
    signalAdapter->setImageSensorId(m_oImageIds[oIdxWorkerCur]);
    signalAdapter->setOverlayCanvas(&m_oCanvasBuffer[oIdxWorkerCur]);
    signalAdapter->setImageSender(m_imageSender);
    signalAdapter->setHardwareCamera(m_hasHardwareCamera);
    triggerContext.setSeamSeriesNumber(m_pActiveSeamSeries->m_oNumber);
    triggerContext.setSeamNumber(m_pActiveSeam->m_oNumber);
    signalAdapter->setTriggerContext(triggerContext);
    signalAdapter->setLastProcessedImage(m_lastProcessedImage);
    if (!m_simulationStation)
    {
        signalAdapter->setVideoRecorder(m_pVideoRecorderProxy);
    }
    if (m_oWorkers[oIdxWorkerCur].scheduleWork(std::move(signalAdapter), std::chrono::microseconds(int(m_pActiveSeamInterval->m_pMeasureTask->getTimeDelta() * 1000))))
    {
        m_lastProcessedImage = triggerContext.imageNumber();
        ++m_oNbSeamSignaled;
    }
    else
    {
        wmLog(eError, "Failed to pass SignalAdapter for %d to ProcessingThread as the ProcessingThread is still processing previous SignalAdapter.\n", triggerContext.imageNumber());
        processingMode = ProcessingMode::Overtriggered;
        if (m_lastSkippedImageNumber < triggerContext.imageNumber())
        {
            // already set by ::data
            m_oNumImagesSkippedInInspection++;
            m_lastSkippedImageNumber = triggerContext.imageNumber();
        }
    }

    double value = 0.0;
    switch (processingMode)
    {
    case ProcessingMode::Overtriggered:
        value = 1.0;
        break;
    case ProcessingMode::MissingImage:
        value = 2.0;
        break;
    case ProcessingMode::OutOfOrderImage:
        value = 3.0;
        break;
    case ProcessingMode::Normal:
    default:
        break;
    }
    // send ProcessingMode as result
    const auto oImageContext = makeImageContext( triggerContext );
    ResultDoubleArray oProcessingModeResult (
            UUID::null(),
            InspectManagerImageProcessingMode,
            ResultType::AnalysisOK,
            oImageContext,
            GeoDoublearray{oImageContext, Doublearray{1, value}, ResultType::AnalysisOK},
            Range1d(),
            false);
    // pass to sum error handling
    m_oResultHandler.sendResult( oProcessingModeResult );
    // and send to result proxy
    m_pResultProxy->result( oProcessingModeResult );

}

void InspectManager::queueSample(int sensorId, const TriggerContext &triggerContext, const image::Sample &sample)
{
	const auto imageContext = makeImageContext(triggerContext);
	auto it = m_oSampleQueues.find(sensorId);
	if (it != m_oSampleQueues.end())
	{
		it->second.push(SampleFrame{sensorId, imageContext, sample});
	}
	else
	{
		std::queue<SampleFrame> queue;
		queue.push(SampleFrame{sensorId, imageContext, sample});
		m_oSampleQueues.insert(std::make_pair(sensorId, std::move(queue)));
	}
}

std::map<int, SampleFrame> InspectManager::dequeueSamples()
{
	std::map<int, SampleFrame> samples;
	for (auto it = m_oSampleQueues.begin(); it != m_oSampleQueues.end(); it++)
	{
		if (it->second.empty())
		{
			continue;
		}
		const SampleFrame frame = it->second.front();
		it->second.pop();
		samples.insert(std::make_pair(it->first, std::move(frame)));
	}
	return samples;
}

bool InspectManager::areAllSamplesQueued()
{
	if (!m_pActiveGraph)
	{
		return false;
	}
	auto sensorIds = std::get<1/*sample*/>(GraphAssistent(m_pActiveGraph).getSourceFilterTypesAndIds());
    
    if (g_oUseScanmasterPosition)
    {
        sensorIds.push_back(eScannerXPosition);
        sensorIds.push_back(eScannerYPosition);
    }
    
	return std::all_of(sensorIds.begin(), sensorIds.end(),
		[this] (int sensorId)
		{
			auto it = m_oSampleQueues.find(sensorId);
			if (it == m_oSampleQueues.end())
			{
				return false;
			}
			else
			{
				return !it->second.empty();
			}
		}
	);
}


void InspectManager::armHwParameters( const UUID &p_rParamSetId ) {
	if ( p_rParamSetId == m_oHwParameters.m_oDefaultHwSetID ) {
		wmLog( eDebug, "Default HW Parameter Set was activated, ignoring for now ...\n");
		return;
	}
    if (m_simulationStation)
    {
        return;
    }

	m_oHwParameters.activateHwParamSet( p_rParamSetId, m_pCentralDeviceManager );
} // armHwParameters



ImageContext InspectManager::makeImageContext(const TriggerContext& p_rTriggerContext) const {
	const auto&		rMeasureTask			= *m_pActiveSeamInterval->m_pMeasureTask;
	auto 			oImageContext			= ImageContext 		( p_rTriggerContext );
	auto			oTaskContext			= TaskContext		( new Trafo, m_oActiveProductInstanceId, new MeasureTask( rMeasureTask) );
	auto			oMeasureTaskPos			= eMiddleImage;

	if (p_rTriggerContext.imageNumber() == 0) {
		oMeasureTaskPos	= eFirstImage;
	} // if
	else if (p_rTriggerContext.imageNumber() == rMeasureTask.lastImage()) {
		oMeasureTaskPos	= eLastImage;
	} // else if

	oImageContext.setImageNumber(p_rTriggerContext.imageNumber());
	oImageContext.setMeasureTaskPositionFlag(oMeasureTaskPos);							// if 1st middle or last img within task
	oImageContext.setPosition(m_oCurrentPosSeam);										// for whatever reason...
    oImageContext.setTime(int(m_oCurrentPosSeam / double(m_pActiveSeam->m_oVelocity) * 1000));

	oTaskContext.setProductId(m_pActiveProduct->m_rProductData.productID());
	oTaskContext.setProductInstanceId(m_oActiveProductInstanceId);

	oImageContext.setTaskContext(oTaskContext);

	return oImageContext;
} // makeImageContext



void InspectManager::releaseDataPipesAndResultHandler(fliplib::FilterGraph* p_pGraph)
{
        m_oPipeImageFrame.uninstallAll();
        for (const auto &pPipe : m_oPipesSampleFrame)
        {
            pPipe->uninstallAll();
        }

        GraphAssistent{ p_pGraph }.releaseResultHandler( m_oResultHandler );
        m_oResultHandler.clearInPipes();    // also clear in-pipes
} // releaseDataPipesAndResultHandler



void InspectManager::joinWorkers()
{
    for (std::size_t i = 0; i < g_oNbPar; i++)
    {
        m_oWorkers.at(i).join();
    }
} // joinWorkers


void InspectManager::resetCalibration ( int p_oSensorID )
{
    wmLog(eInfo, "InspectManager reset calibration\n");
    math::CalibrationData &rCalibData = getCalibrationData ( p_oSensorID );
    rCalibData.resetConfig();
    assert(!rCalibData.isInitialized());
}

std::shared_ptr<interface::TRecorderPoll<interface::AbstractInterface>> InspectManager::recorderPollServer() const
{
    return m_imageSender;
}

void InspectManager::sendLastTimeResult(ResultType oResultType)
{
        unsigned int numMeasurements;
        double time;
        switch(oResultType)
        {
            case ResultType::ProcessTime:
                    std::tie(numMeasurements,time) = m_oInspectTimer.getLastImageTime_us();
                    break;
            case ResultType::InspectManagerTime:
                    std::tie(numMeasurements,time) = m_oInspectTimer.getLastInspectManagerTime_us();
                    break;
            default:
                return;
                break;
        }
        
        if (numMeasurements == 0)
        {
            return;
        }
        //minimal image context: only position and value for the plotter
        unsigned int imageNumber = numMeasurements - 1;
        ImageContext oImageContext;
        if (m_pActiveSeam)
        {
            oImageContext.setPosition(m_pActiveSeam->m_oTriggerDelta  * imageNumber);
        }
        else
        {
            oImageContext.setPosition(imageNumber);
        }
        oImageContext.setImageNumber(imageNumber);
        
        ResultDoubleArray oTimeResult(
                    UUID::null(),
                    oResultType,
                    ResultType::AnalysisOK,
                    oImageContext,
                    GeoDoublearray(oImageContext, geo2d::TArray<double>{1, time ,255}, ResultType::AnalysisOK, 1.0 ),
                    Range1d(),
                    false); // nio
        m_oResultHandler.sendResult(oTimeResult);
}


void InspectManager::updateInspectManagerTime(const std::chrono::nanoseconds &time, bool increaseFrameCount )
{
    m_oInspectTimer.updateInspectManagerTime(time, increaseFrameCount);

    if (g_oDebugTimings && increaseFrameCount)
    {        
        sendLastTimeResult(ResultType::InspectManagerTime);
        
        //try to send the previous processing time, but it's just an estimate because  we are not synchronized 
        //with the instances of SignalAdapters (we could be missing times, or send the same time twice)
        sendLastTimeResult(ResultType::ProcessTime);
    }
}

void InspectManager::processingThreadFinishedCallback()
{
    std::lock_guard<std::mutex> lock{m_callbackMutex};
    m_oNbSeamJoined++;
}

void InspectManager::simulationDataMissing(const TriggerContext &context)
{
    if (!m_simulationStation)
    {
        return;
    }
    wmLog(eInfo, "Skipping image %d due to missing data in storage\n", context.imageNumber());
    getRecorderSrv().simulationDataMissing(makeImageContext(context));
}

} // namespace analyzer
} // namespace precitec
