///////////////////////////////////////////////////////////
//  ParameterList.cpp
//  Implementation of the Class ParameterList
//  Created on:      11-Dez-2007 17:22:41
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <typeinfo>
#include "Poco/Types.h"
#include "Poco/Bugcheck.h"
#include "Poco/DynamicAny.h"
#include "Poco/Exception.h"
#include "fliplib/Fliplib.h"
#include "fliplib/Exception.h"
#include "fliplib/Parameter.h"
#include "fliplib/ParameterContainer.h"

using namespace Poco;

namespace fliplib
{

ParameterContainer::ParameterContainer()
{

}

ParameterContainer::~ParameterContainer()
{
	clear();
}
void ParameterContainer::addStr(const std::string & name, const std::string& type, const std::string& value)
{
	Parameter* pParameter ( new Parameter(name, type, value) );
	add( name, pParameter );
}


void ParameterContainer::update(const std::string& name, const std::string& type, const Poco::DynamicAny& value)
{
	add( name, type, Parameter::convertToType(type, value) );
}

void ParameterContainer::add(const std::string& name, const std::string& type, const Poco::DynamicAny& value)
{
    auto it = map_.find(name);
    if (it == map_.end())
    {
        map_[name] =  new Parameter(name, type, value);
    }
    else
    {
        if ((*it->second).getValue().type() != value.type())
        {
            (*it->second).setValue(Parameter::convertToType(type, value));
        }
        else
        {
            (*it->second).setValue(value);
        }
    }
}

void ParameterContainer::add(const std::string& name, Parameter* parameter)
{
	poco_check_ptr( parameter );

	ParameterMap::iterator it = map_.find(name);
	if (it == map_.end())
	{
		map_[name] = parameter;
	}
	else
	{
		// Wenn der Parameter bereits existiert, wird geprueft ob der Type gleich ist
		if ((*it->second).getValue().type() != parameter->getValue().type())
		{
			std::stringstream ss;
			ss << "Parameter '" << name << "' already exists but type different. T1:" << (*it->second).getValue().type().name() << " T2:" << parameter->getValue().type().name() << std::endl;
			std::cout << ss.str() << std::endl;
			delete (parameter);
			throw ParameterException(ss.str());
		}

		(*it->second).setValue(parameter->getValue());

		// kopierter Parameter wird nicht mehr benoetigt
		delete (parameter);

	}
}

const Parameter& ParameterContainer::findParameter(const std::string& name) const
{
	auto it = map_.find(name);
	if (it != map_.end())
	{
		return *it->second;
	}

	throw ParameterException("Parameter '" + name + "' not found");
}

const Poco::DynamicAny& ParameterContainer::getParamValue(const std::string& name)
{
	return findParameter(name).getValue();
}

bool ParameterContainer::exists(const std::string& name)
{
	ParameterMap::iterator it = map_.find(name);
	return (it != map_.end());
}

bool ParameterContainer::isUpdated()
{
	// Parameter zerstoeren
	for (ParameterMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
	{
		if ((it->second)->isUpdated())
			return true;
	}

	return false;
}

void ParameterContainer::confirm()
{
	// Parameter zerstoeren
	for (ParameterMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
	{
		(it->second)->updated_ = false;
	}

}

int ParameterContainer::count()
{
	return map_.size();
}

void ParameterContainer::clear()
// Loescht alle Parameter aus dem Container; Die Parameterinstanzen werden geloescht
{
	// Parameter zerstoeren
	for (ParameterMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
		delete it->second;

	// Map loeschen
	map_.clear();
}

std::string ParameterContainer::toXml() const
{
	std::stringstream ss;
	ss << "<parameters>";

	for (ParameterMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
	{
		Parameter* p = it->second;
		ss << (*p).toXml();
	}

	ss << "</parameters>";
	return ss.str();
}

ConversionProxy ParameterContainer::getParameter(const std::string& name) const {
	 return ConversionProxy(name, *this);
}

} // namespace fliplib
