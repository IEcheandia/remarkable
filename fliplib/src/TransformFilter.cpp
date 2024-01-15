///////////////////////////////////////////////////////////
//  TransformFilter.cpp
//  Implementation of the Class TransformFilter
//  Created on:      17-Okt-2007 17:45:05
//  Original author: Sevitec
///////////////////////////////////////////////////////////
#include <string>
#include "fliplib/TransformFilter.h"

using fliplib::TransformFilter;


void TransformFilter::setParameter() {
	BaseFilter::setParameter();
} // setParameter

TransformFilter::TransformFilter(const std::string& name) :
	BaseFilter(name)
{
}

TransformFilter::TransformFilter(const std::string& name, const Poco::UUID & filterID) :
	BaseFilter(name, filterID)
{
}

TransformFilter::~TransformFilter()
{
}

/*virtual*/ int TransformFilter::getFilterType() const
{
	return BaseFilterInterface::TRANSFORM;

} // getFilterType
