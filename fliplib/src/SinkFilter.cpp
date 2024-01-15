///////////////////////////////////////////////////////////
//  SinkFilter.cpp
//  Implementation of the Class SinkFilter
//  Created on:      17-Okt-2007 17:45:05
//  Original author: Sevitec
///////////////////////////////////////////////////////////
#include <string>
#include "fliplib/SinkFilter.h"
#include "fliplib/BasePipe.h"

using fliplib::SinkFilter;
using fliplib::BasePipe;

SinkFilter::SinkFilter(const std::string& name) :
	BaseFilter(name)
{
}

SinkFilter::SinkFilter(const std::string& name, const Poco::UUID & filterID) :
	BaseFilter(name, filterID)
{
}

void SinkFilter::clearInPipes() 
{
    m_oInPipes.clear();
} // clearInPipes

/*virtual*/ int SinkFilter::getFilterType() const
{
	return BaseFilterInterface::SINK;

} // getFilterType


bool SinkFilter::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	if ( p_rPipe.type() == typeid(precitec::interface::ResultDoubleArray) ) 
    {
		m_oInPipes.push_back(static_cast<const pipe_result_t*>(&p_rPipe));
	} // if
	else
	{
		throw NotImplementedException{ std::string{ "pipe type '" } + p_rPipe.type().name() + "' not supported." };
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe
