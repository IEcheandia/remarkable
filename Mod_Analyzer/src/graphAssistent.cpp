/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			kir, ab, hs
 *  @date			2009
 *  @brief			Der Graph Assistent stellt Methoden zur Arm / Steuerung der Graphen zur Verfuegung
 */

#include "analyzer/graphAssistent.h"
#include "analyzer/graphVisitors.h"
#include "filter/armStates.h"


using namespace fliplib;
namespace precitec
{	using namespace image;
	using namespace filter;
	using namespace interface;
namespace analyzer
{

void GraphAssistent::init(  )
{
	FilterInitalizer oVisitor;
	apply( oVisitor );
}



void GraphAssistent::dispose(  )
{
	FilterDisposer oVisitor;
	apply( oVisitor );
}



void GraphAssistent::setParameter(  )
{
	ParameterSetter oVisitor;
	apply( oVisitor );
}

bool GraphAssistent::setFilterGraph(fliplib::FilterGraph* graph)
{
	graph_ = graph;
	return (graph_ != nullptr);
}

void GraphAssistent::changeParameterFilterDb(TDb<AbstractInterface>& p_rDbProxy, const Poco::UUID& p_rMeasureTaskId, const Poco::UUID& p_rInstFilterId)
{
	// gewuenschter Filter suchen
	BaseFilter* pFilter = graph_->find(p_rInstFilterId);
	if ( pFilter == nullptr) {
		wmLog( eWarning, "GraphAssistent::changeParameterFilterDb: Could not find filter with id '%s'.\n", p_rInstFilterId.toString().c_str() );

		return;
	} // if

	// Parameter aus DB abfragen
	const ParameterList oParameterList		( p_rDbProxy.getFilterParameter(p_rInstFilterId, p_rMeasureTaskId) );
	// add paremeters to filter
	addParametersToFilter(oParameterList, *pFilter);
	// Parameter uebernehmen
	setParameter();
} // changeParameterFilterDb



void GraphAssistent::setParameterSet( const Poco::UUID& p_rParamSetId, const paramSetMap_t& p_rParamSetMap ) {
    auto oParameterSetSetter = prepareSetParameterSet(p_rParamSetId, p_rParamSetMap);
    if (!oParameterSetSetter)
    {
        return;
    }
	apply( *oParameterSetSetter );

	// Parameter uebernehmen
	setParameter();  // setParameterFast(); // set only parameters that do not require an arming 
} // setParameterSet

void GraphAssistent::delayedSetParameterSet( const Poco::UUID& p_rParamSetId, const interface::paramSetMap_t& p_rParamSetMap)
{
    auto parameterSetter = prepareSetParameterSet(p_rParamSetId, p_rParamSetMap);
    if (!parameterSetter)
    {
        return;
    }
    m_delayedVisitors.emplace_back(std::move(parameterSetter));
    m_delayedVisitors.emplace_back(std::make_unique<ParameterSetter>());
}

std::unique_ptr<fliplib::AbstractFilterVisitor> GraphAssistent::prepareSetParameterSet(const Poco::UUID& p_rParamSetId, const interface::paramSetMap_t& p_rParamSetMap)
{
    if (p_rParamSetId.isNull())
    {
        return {};
    }

    const auto oItParamSetFound{p_rParamSetMap.find(p_rParamSetId)};
    if (oItParamSetFound  == std::end(p_rParamSetMap))
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << ": parameter set with id " << p_rParamSetId.toString() << " not found. Abort.\n";
        wmLog(eWarning, oMsg.str());
        return {};
    }
    return std::make_unique<ParameterSetSetter>(oItParamSetFound->second);
}


void GraphAssistent::setResultHandler ( fliplib::SinkFilter &p_rHandler )
{
	ResultHandlerConnector oVisitor( p_rHandler );
	apply( oVisitor );
}


void GraphAssistent::releaseResultHandler ( fliplib::SinkFilter &p_rHandler )
{
	ResultHandlerReleaser oVisitor( p_rHandler );
	apply( oVisitor );
}



void GraphAssistent::setCanvas( OverlayCanvas* p_pCanvas )
{
	auto oVisitor = prepareSetCanvas( p_pCanvas );
	apply( *oVisitor );
}

void GraphAssistent::delayedSetCanvas( image::OverlayCanvas* p_pCanvas )
{
    m_delayedVisitors.emplace_back(prepareSetCanvas(p_pCanvas));
}

std::unique_ptr<fliplib::AbstractFilterVisitor> GraphAssistent::prepareSetCanvas(image::OverlayCanvas* p_pCanvas) const
{
    return std::make_unique<CanvasSetter>(p_pCanvas);
}

void GraphAssistent::setExternalData(const fliplib::ExternalData& p_rExternalData) {
	auto oVisitor = prepareSetExternalData(p_rExternalData);
	apply(*oVisitor);
}

void GraphAssistent::delayedSetExternalData(const fliplib::ExternalData& p_rExternalData)
{
    m_delayedVisitors.emplace_back(prepareSetExternalData(p_rExternalData));
}

std::unique_ptr<fliplib::AbstractFilterVisitor> GraphAssistent::prepareSetExternalData(const fliplib::ExternalData& p_rExternalData) const
{
    return std::make_unique<ExternalDataSetter>(p_rExternalData);
}

std::vector<int> GraphAssistent::getSensorIds()
{
	SensorIdGetter oVisitor;
	apply( oVisitor );
	return oVisitor.getSensorIds();
}

std::vector<Poco::UUID> GraphAssistent::getFilterIds()
{
    FilterIdGetter oVisitor;
    apply(oVisitor);
    return oVisitor.getFilterIds();
}

SourceFilterType GraphAssistent::getSourceFilterTypes() {
	SourceFilterTypesGetter oVisitor;
	apply( oVisitor );
	const SourceFilterType oSourceFiltersUsed		( oVisitor.getSourceFilterTypes() );
	return oSourceFiltersUsed;
} // getSourceFilterTypes


std::tuple<std::vector<int>, std::vector<int>>  GraphAssistent::getSourceFilterTypesAndIds() {
	SourceFilterTypesAndIdsGetter oVisitor;
	apply( oVisitor );
	return std::make_tuple(oVisitor.getImageSourceFilterIds(), oVisitor.getSampleSourceFilterIds());
} // getSourceFilterTypes

void GraphAssistent::paint()
{
	PaintVisitor oVisitor;
	apply( oVisitor );
}



void GraphAssistent::arm(ArmState p_oArmState) {
	auto armVisitor = prepareArm( p_oArmState );
	apply( *armVisitor );
}

void GraphAssistent::delayedArm(filter::ArmState p_oArmState)
{
    m_delayedVisitors.emplace_back(prepareArm(p_oArmState));
}

std::unique_ptr<fliplib::AbstractFilterVisitor> GraphAssistent::prepareArm(filter::ArmState p_oArmState) const
{
    return std::make_unique<FilterArm>(p_oArmState);
}


void GraphAssistent::fire( )
{
	graph_->fire( );
}



void GraphAssistent::setCount(int p_oCount)
{
	auto oVisitor = prepareCount(p_oCount);
	apply( *oVisitor );
}

void GraphAssistent::delayedSetCount(int p_oCount)
{
    m_delayedVisitors.emplace_back(prepareCount(p_oCount));
}

std::unique_ptr<fliplib::AbstractFilterVisitor> GraphAssistent::prepareCount(int p_oCount) const
{
    return std::make_unique<SetCounterVisitor>(p_oCount);
}


void GraphAssistent::apply (AbstractFilterVisitor& p_rVisitor)
{
	graph_->control( p_rVisitor );
}

void GraphAssistent::apply()
{
    graph_->control(m_delayedVisitors);
    m_delayedVisitors.clear();
}

void GraphAssistent::synchronizeSourceFilters(int imageNumber)
{
    SynchronizeSourceFiltersVisitor visitor(imageNumber);
    // not using ::apply or graph_->control as that uses a mutex and would dead lock the synchronization
    const auto filters = graph_->getFilterMap();
    for (auto it = filters.begin(); it != filters.end(); it++)
    {
        visitor.control(*it->second->getFilter());
    }
}

void GraphAssistent::logProcessingTime()
{
    LogProcessingTimeVisitor visitor;
    if (g_oDebugTimings)
    {
        graph_->controlAccordingToProcessingOrder( visitor );
    }
    else
    {
        graph_->control( visitor );
    }
}

void GraphAssistent::delayedLogProcessingTime()
{
    auto visitor = std::make_unique<LogProcessingTimeVisitor>();
    if (g_oDebugTimings)
    {
        graph_->controlAccordingToProcessingOrder( *visitor );
    }
    else
    {
        m_delayedVisitors.emplace_back(std::move(visitor));
    }
}

void GraphAssistent::ensureImageNumber(int imageNumber)
{
    EnsureImageNumberVisitor visitor{imageNumber};
    apply(visitor);
}

std::map<int,int> GraphAssistent::getProcessedImagesCounter()
{
    CountProcessedImagesVisitor visitor;
    apply(visitor);
    return visitor.getProcessedImagesCounter();
}

void GraphAssistent::logProcessedImagesCount(precitec::LogType p_oLogType)
{
    logProcessedImagesCount(p_oLogType, getProcessedImagesCounter());
}

void GraphAssistent::logProcessedImagesCount(precitec::LogType p_oLogType, const std::map<int,int> & p_rProcessedImagesCounter)
{
    int totFiltersInGraph = 0;
    for (auto & rPair: p_rProcessedImagesCounter)
    {
        totFiltersInGraph += rPair.second; 
    }
    
    if (totFiltersInGraph == 0)
    {
        return;
    }
    
    std::ostringstream oMsg;
    oMsg << "Processed Images in graph: " ;
    
    int imageCounter = p_rProcessedImagesCounter.begin()->first;
    for (auto & rPair : p_rProcessedImagesCounter)
    {  
        int numFilters = 0;
        for ( ; imageCounter < rPair.first; imageCounter++)
        {
            oMsg << "img " << imageCounter << " [" << numFilters << "/" <<  totFiltersInGraph  << "] " ;
        }
        imageCounter  =  rPair.first;
        numFilters = rPair.second;
        oMsg << "img " << imageCounter << " [" << numFilters << "/" <<  totFiltersInGraph  << "] " ;
        imageCounter++;
    }
    oMsg << "\n";
    wmLog(p_oLogType, oMsg.str());
}


} // namespace analyzer
} // namespace precitec
