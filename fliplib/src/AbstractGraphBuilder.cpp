///////////////////////////////////////////////////////////
//  AbstractGraphBuilder.cpp
//  Implementation of the Class AbstractGraphBuilder
//  Created on:      08-Nov-2007 14:13:52
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#include "fliplib/Activator.h"
#include "fliplib/AbstractGraphBuilder.h"

using fliplib::Activator;
using fliplib::BaseFilter;
using fliplib::AbstractGraphBuilder;

AbstractGraphBuilder::AbstractGraphBuilder()
{
}

AbstractGraphBuilder::~AbstractGraphBuilder()
{
}

fliplib::FilterHandle* AbstractGraphBuilder::activate(const std::string& componentUri, const std::string& filter)
// Laedt die Komponente in den Speicher und erstellt eine Instance des Filters
{
	return Activator::createInstance(componentUri, filter);
}

fliplib::GraphContainer fliplib::AbstractGraphBuilder::buildGraphDescription(const std::string& /*p_rUri*/)
{
    return fliplib::GraphContainer{};
}
