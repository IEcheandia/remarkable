
///////////////////////////////////////////////////////////
//  FilterHandle.cpp
//  Implementation of the Class FilterHandle
//  Created on:      28-Jan-2008 11:27:50
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#include "fliplib/FilterHandle.h"
#include "fliplib/Activator.h"

using fliplib::FilterHandle;
using fliplib::Activator;

FilterHandle::FilterHandle(fliplib::BaseFilter* filter, const std::string& componentUri) 
	: filter_(filter), componentUri_(componentUri) 
{ 
}

FilterHandle::FilterHandle()
{

}

FilterHandle::~FilterHandle()
{
	
}

