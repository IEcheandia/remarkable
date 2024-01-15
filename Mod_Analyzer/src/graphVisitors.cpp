/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			kir, ab, hs
 *  @date			2009
 *  @brief			Filtergraph Visitoren zur Konfiguration / Steuerung der Filter
 */

#include "analyzer/graphVisitors.h"

#include <string>
#include <sstream>

#include "event/results.h"
#include "filter/sensorFilterInterface.h"
#include "event/results.interface.h"

#include "module/moduleLogger.h"

using namespace fliplib;
using Poco::UUID;
namespace precitec 	{
	using namespace interface;
	using namespace filter;
namespace analyzer	{


// ---------------------------------------------
// FilterDisposer

void FilterDisposer::control (FilterControlInterface& filter)
{
	filter.dispose();
}


// ---------------------------------------------
// FilterInitalizer
void FilterInitalizer::control (FilterControlInterface& filter)
{
	filter.init();
}


// ---------------------------------------------
// FilterArm
FilterArm::FilterArm (ArmState p_oArmState) : m_oArmState{ p_oArmState} {
}

void FilterArm::control (FilterControlInterface& p_rFilter) {
	p_rFilter.arm(ArmStateBase{ m_oArmState });

    auto& rFilter = static_cast< fliplib::BaseFilter& >( p_rFilter );

	if (m_oArmState == eSeamEnd || m_oArmState == eSeamIntervalChange) {
		rFilter.resetSignalCntGroupEvent();
	} // if
    if (m_oArmState == eSeamStart) {
        rFilter.resetTimerCnt();
    } // if
}


// ---------------------------------------------
// ParameterSetter
void ParameterSetter::control (FilterControlInterface& filter)
{
	BaseFilter& refFilter = static_cast< BaseFilter& >( filter );

	if (refFilter.getParameters().isUpdated())
	{
		// Filterparameter holen. Sobald Parameter gelesen werden, wird das Updateflag automatisch
		// auf gelesen gesetzt.
		filter.setParameter();
		refFilter.getParameters().confirm();
	}
}


// ---------------------------------------------
// ParameterSetSetter
void ParameterSetSetter::control (FilterControlInterface& p_rFilter)
{
	BaseFilter& rFilter = static_cast< BaseFilter& >( p_rFilter );

	const UUID&				rFilterId				( rFilter.id() ); 
	const auto				oItParamListFilterFound	( m_rParamSet.find(rFilterId) ); 
	if (oItParamListFilterFound  == std::end(m_rParamSet)) {
		std::stringstream oMsg;
		oMsg << __FUNCTION__ << ": Parameter set for filter id " << rFilterId.toString() << " not found. Skip.\n";
		wmLog(eDebug, oMsg.str());
		return;
	} // if
				
	const ParameterList& rParameterList		( oItParamListFilterFound->second );
	// add parameters to filter
	addParametersToFilter(rParameterList, rFilter);
} // control


// ---------------------------------------------
// SensorIdGetter
void SensorIdGetter::control (FilterControlInterface& p_rFilter) {
	const BaseFilter&		 rBaseFilter		( static_cast<BaseFilter&>(p_rFilter) );
	if (rBaseFilter.getFilterType() != BaseFilterInterface::SOURCE) {
		return;
	} // if
	const SourceFilter&		 rSourceFilter		( static_cast<const SourceFilter&>(rBaseFilter) );
	sensorIds_.push_back(rSourceFilter.getSensorID());
}

// ---------------------------------------------
// FilterIdGetter
void FilterIdGetter::control(FilterControlInterface& filter)
{
    const BaseFilter& rBaseFilter(static_cast<BaseFilter&>(filter));
    m_filterIds.push_back(rBaseFilter.filterID());
}

// ---------------------------------------------
// SourceFilterTypesGetter
void SourceFilterTypesGetter::control (FilterControlInterface& p_rFilter) {
	// check if source filter

	const BaseFilter&			rBaseFilter		( static_cast<BaseFilter&>(p_rFilter) );
	if (rBaseFilter.getFilterType() != BaseFilterInterface::SOURCE) {
		return;
	} // if

	const SourceFilter&			rSourceFilter		( static_cast<const SourceFilter&>(rBaseFilter) );
	static const UUID			oImgSourceId		( "CABA9D15-30DD-4a0c-93E5-3000D68068F7" );  // same as in  ...\DatabaseSkripts\FilterImageSource\ImageSource.sql and ...\Filter_ImageSource\imageSource.cpp
	if (rSourceFilter.filterID() == oImgSourceId) {
		m_oSourceFilterUsed = SourceFilterType(m_oSourceFilterUsed | eImage);
	} // if
	else { // there are only one image source filters and some sample source filters
		m_oSourceFilterUsed = SourceFilterType(m_oSourceFilterUsed | eSample);
	} // else
} // control


// ---------------------------------------------
// SourceFilterTypesGetter
void SourceFilterTypesAndIdsGetter::control (FilterControlInterface& p_rFilter) {
	// check if source filter

	const BaseFilter&			rBaseFilter		( static_cast<BaseFilter&>(p_rFilter) );
	if (rBaseFilter.getFilterType() != BaseFilterInterface::SOURCE) {
		return;
	} // if

	const SourceFilter&			rSourceFilter		( static_cast<const SourceFilter&>(rBaseFilter) );
	static const UUID			oImgSourceId		( "CABA9D15-30DD-4a0c-93E5-3000D68068F7" );  // same as in  ...\DatabaseSkripts\FilterImageSource\ImageSource.sql and ...\Filter_ImageSource\imageSource.cpp
	if (rSourceFilter.filterID() == oImgSourceId) {
		m_oSensorIdsImage.push_back(rSourceFilter.getSensorID());
	} // if
	else { // there are only one image source filters and some sample source filters
		m_oSensorIdsSample.push_back(rSourceFilter.getSensorID());
	} // else
} // 


// ---------------------------------------------
// PaintVisitor
void PaintVisitor::control (FilterControlInterface& filter)
{
	filter.paint();
}


// ---------------------------------------------
// CanvasSetter
void CanvasSetter::control (FilterControlInterface& filter)
{
	// Canvas dem Filter zuweisen. Der Canvas wird nicht vom Filter verwaltet!
	filter.setCanvas( m_pCanvas );
}


// ---------------------------------------------
// ExternalDataSetter
void ExternalDataSetter::control(FilterControlInterface& p_rFilter)
{
	p_rFilter.setExternalData( &m_rExternalData );
}


// Verlinke ResultFilter mit dem ResultFilter via ResultFilterInterface
void ResultHandlerConnector::control (FilterControlInterface& filter)
{
	BaseFilter& sender = static_cast< BaseFilter& >( filter );

	if (sender.getFilterType() == BaseFilterInterface::RESULT)
	{
		for(BaseFilter::Iterator it =sender.begin();it != sender.end(); ++it)
		{
			BasePipe *pipe = it.getPipe();

			// Sender mit Receiver verbinden
			//std::stringstream oSt; oSt << "'" << sender.name()
			//				 << "' wird mit resultHandler '" << resultHandler_.name()
			//				 << "' via pipe '" << pipe->name() << "' verbunden.\n";
			//wmLog( eDebug, oSt.str() );
			resultHandler_.connectPipe(pipe, 1);
		}
	}
}



SetCounterVisitor::SetCounterVisitor(int p_oCount) : m_oCount { p_oCount }
{}

void SetCounterVisitor::control (fliplib::FilterControlInterface& p_rFilter)
{
    auto& rFilter = static_cast< fliplib::BaseFilter& >( p_rFilter );
    rFilter.setCounter(m_oCount);
}



// haenge alle result-/nio-sender ab
void ResultHandlerReleaser::control (FilterControlInterface& filter)
{
	BaseFilter& sender = static_cast< BaseFilter& >( filter );

	if (sender.getFilterType() == BaseFilterInterface::RESULT)
	{
		for(BaseFilter::Iterator it =sender.begin();it != sender.end(); ++it)
		{
			//std::stringstream oSt; oSt << "'" << sender.name()
			//				 << "' wird von resultHandler '" << resultHandler_.name()
			//				 << "' via pipe '" << it.getPipe()->name() << "' getrennt.\n";
			//wmLog( eDebug, oSt.str() );
			auto pPipe = it.getPipe();
			resultHandler_.disconnectPipe(pPipe);
		}
	}
}

void SynchronizeSourceFiltersVisitor::control(fliplib::FilterControlInterface &filter)
{
    BaseFilter& baseFilter = static_cast< BaseFilter& >( filter );
    if (baseFilter.getFilterType() == fliplib::BaseFilterInterface::SOURCE)
    {
        baseFilter.synchronizeOnImgNb(m_imageNumber);
    }
}

void SkipImageProcessingVisitor::control(fliplib::FilterControlInterface &filter)
{
    BaseFilter& baseFilter = static_cast< BaseFilter& >( filter );
    baseFilter.skipImageProcessing(m_imageNumber);
}

void ResetFilterIndexesVisitor::control(fliplib::FilterControlInterface &filter)
{
    BaseFilter& baseFilter = static_cast< BaseFilter& >( filter );
    baseFilter.setGraphIndex(m_graphIndex);
    ++m_graphIndex;
    baseFilter.resetProcessingIndex(); //in order to start from 0, also BaseFilter::resetProcessingCounter must be called 
}

void SetAlwaysEnableTimingsVisitor::control(fliplib::FilterControlInterface & filter)
{
    BaseFilter& baseFilter = static_cast< BaseFilter& >( filter );
    baseFilter.alwaysEnableTiming(m_oAlwaysEnableTiming);
}

void CountProcessedImagesVisitor::control(fliplib::FilterControlInterface & filter)
{
    BaseFilter& baseFilter = static_cast< BaseFilter& >(filter);
    m_oProcessedImagesCounter[baseFilter.readCounter()]++; 
}

void addParametersToFilter(const ParameterList& p_rParameterList, BaseFilter& p_rFilter) {
	// Alle Parameter in Filter aktualisieren
	for(auto oItParamter = std::begin(p_rParameterList); oItParamter != std::end(p_rParameterList); ++oItParamter) {
		p_rFilter.getParameters().add( (*oItParamter)->name(), (*oItParamter)->typeToStr(), (*oItParamter)->any() );
	} // for
} // addParametersToFilter


} // namespace analyzer
} // namespace precitec

