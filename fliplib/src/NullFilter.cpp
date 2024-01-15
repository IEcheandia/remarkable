///////////////////////////////////////////////////////////
//  NullFilter.cpp
//  Implementation of the Class NullFilter
//  Created on:      17-Okt-2007 17:45:05
//  Original author: Sevitec
///////////////////////////////////////////////////////////
#include <string>
#include <iostream>
#include "Poco/Delegate.h"
#include "fliplib/BasePipe.h"
#include "fliplib/NullFilter.h"

using Poco::Delegate;
using Poco::SharedPtr;
using fliplib::BasePipe;
using fliplib::NullFilter;
using fliplib::PipeEventArgs;

const std::string fliplib::NullFilter::FILTERNAME 	= std::string("NullFilter");
const std::string fliplib::NullFilter::PIPENAME		= std::string("NullPipe");

NullFilter::NullFilter() :
	TransformFilter(NullFilter::FILTERNAME ),
	output_(new NullPipe(this, NullFilter::PIPENAME)),
	count_(0)
{
}

NullFilter::~NullFilter()
{
	delete output_;
}

					
void NullFilter::proceed(const void* sender, PipeEventArgs& e)
// (Override) Verarbeite saemtliche Messages
{	
}
