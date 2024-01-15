///////////////////////////////////////////////////////////
//  ResultFilter.cpp
//  Implementation of the Class ResultFilter
//  Created on:      17-Okt-2007 17:45:05
//  Original author: Sevitec
///////////////////////////////////////////////////////////
#include <string>
#include "fliplib/ResultFilter.h"

using fliplib::ResultFilter;


void ResultFilter::setParameter() {
	BaseFilter::setParameter();
} // setParameter

ResultFilter::ResultFilter(const std::string& name) :
	BaseFilter(name)
{
}

ResultFilter::ResultFilter(const std::string& name, const Poco::UUID & filterID) :
	BaseFilter(name, filterID)
{
}

ResultFilter::~ResultFilter()
{
}

/*virtual*/ int ResultFilter::getFilterType() const
{
	return BaseFilterInterface::RESULT;

} // getFilterType
