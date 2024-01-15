///////////////////////////////////////////////////////////
//  NullRenderer.cpp
//  Implementation of the Class NullRenderer
//  Created on:      17-Okt-2007 17:45:05
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#include <string>
#include <iostream>
#include "Poco/Delegate.h"
#include "fliplib/BasePipe.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/NullSinkFilter.h"


using Poco::Delegate;
using fliplib::BasePipe;
using fliplib::NullSinkFilter;
using fliplib::PipeEventArgs;

const std::string fliplib::NullSinkFilter::FILTERNAME 	= std::string("NullSinkFilter");

NullSinkFilter::NullSinkFilter() :
	SinkFilter(NullSinkFilter::FILTERNAME),
	count_(0)
{
}

NullSinkFilter::~NullSinkFilter(){

}

void NullSinkFilter::proceed(const void* sender, PipeEventArgs& e)
// (Override) Verarbeite saemtliche Messages
{	
	count_++;
}
