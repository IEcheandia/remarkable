///////////////////////////////////////////////////////////
//  NullRenderer.cpp
//  Implementation of the Class NullSourceFilter
//  Created on:      17-Okt-2007 17:45:05
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#include <string>
#include "Poco/Activity.h"
#include "Poco/Thread.h"

#include "fliplib/BasePipe.h"
#include "fliplib/NullSourceFilter.h"

using Poco::Activity;
using Poco::Thread;
using Poco::SharedPtr;
using fliplib::NullSourceFilter;

const std::string fliplib::NullSourceFilter::FILTERNAME 	= std::string("NullSourceFilter");
const std::string fliplib::NullSourceFilter::PIPENAME	= std::string("NullSourcePipe");

NullSourceFilter::NullSourceFilter() :			
	SourceFilter(NullSourceFilter::FILTERNAME), 
	output_(new NullSourcePipe(this, NullSourceFilter::PIPENAME))
{
}

NullSourceFilter::~NullSourceFilter()
{
	delete output_;
}

void NullSourceFilter::fire()
{
}

