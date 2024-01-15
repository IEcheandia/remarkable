///////////////////////////////////////////////////////////
//  SourceFilter.cpp
//  Implementation of the Class SourceFilter
//  Created on:      17-Okt-2007 17:45:05
//  Original author: Sevitec
///////////////////////////////////////////////////////////
#include <string>
#include "fliplib/SourceFilter.h"

using fliplib::SourceFilter;

const std::string SourceFilter::PARAM_SOURCEFILTER_SENSORID = std::string("sensorid");

SourceFilter::SourceFilter(const std::string& name) :
	BaseFilter(name),
	sensorID_(-1)
{
	parameters_.add( PARAM_SOURCEFILTER_SENSORID, "int", sensorID_ );
}

SourceFilter::SourceFilter(const std::string& name, const Poco::UUID & filterID) :
	BaseFilter(name, filterID),
	sensorID_(-1)
{
	parameters_.add( PARAM_SOURCEFILTER_SENSORID, "int", sensorID_ );
}

SourceFilter::SourceFilter(const std::string& name, const Poco::UUID & filterID, int sensorID) :
	BaseFilter(name, filterID),
	sensorID_(sensorID)
{
	parameters_.add( PARAM_SOURCEFILTER_SENSORID, "int", sensorID_ );
}


void SourceFilter::setParameter() {
	BaseFilter::setParameter();
	sensorID_ = parameters_.getParameter( PARAM_SOURCEFILTER_SENSORID ).convert<int>(); // set source parameter
} // setParameter

SourceFilter::~SourceFilter()
{
}

/*virtual*/ int SourceFilter::getFilterType() const
{
	return BaseFilterInterface::SOURCE;

} // getFilterType
