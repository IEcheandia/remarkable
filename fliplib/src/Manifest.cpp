///////////////////////////////////////////////////////////
//  Manifest.cpp
//  Implementation of the Export Table
//  Created on:      30-Okt-2007 13:18:44
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#include <iostream>
#include "Poco/ClassLibrary.h"

#include "fliplib/Activator.h"
#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilterInterface.h"

#include "fliplib/NullFilter.h"
#include "fliplib/NullSourceFilter.h"
#include "fliplib/NullSinkFilter.h"

using fliplib::BaseFilterInterface;
using fliplib::NullFilter;
using fliplib::NullSinkFilter;
using fliplib::NullSourceFilter;
using fliplib::Activator;

// Publish Export Filters
FLIPLIB_BEGIN_MANIFEST(BaseFilterInterface)
	FLIPLIB_EXPORT_CLASS(NullFilter)
	FLIPLIB_EXPORT_CLASS(NullSinkFilter)
	FLIPLIB_EXPORT_CLASS(NullSourceFilter)
FLIPLIB_END_MANIFEST


//no namespace active!
extern "C" void pocoInitializeLibrary()
{
	std::cout << "Library initializing" << std::endl;
}


extern "C" void pocoUninitializeLibrary()
{
	std::cout << "Library uninitializing" << std::endl;
	
	//Activator::destroy();
	
}
