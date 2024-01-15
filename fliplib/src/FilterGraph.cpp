///////////////////////////////////////////////////////////
//  FilterGraph.cpp
//  Implementation of the Class FilterGraph
//  Created on:      30-Okt-2007 13:33:01
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#include <string>
#include <sstream>
#include <iostream>

#include "fliplib/Activator.h"
#include "fliplib/FilterHandle.h"
#include "fliplib/FilterGraph.h"
#include "fliplib/FilterControlInterface.h"
#include "fliplib/SourceFilter.h"

using Poco::FastMutex;
using fliplib::FilterGraph;
using fliplib::BaseFilter;
using fliplib::SourceFilter;
using fliplib::AbstractFilterVisitor;
using fliplib::FilterControlInterface;
using fliplib::Activator;
using fliplib::FilterHandle;

FilterGraph::FilterGraph(const Poco::UUID& id) :
	id_(id)
{
}

FilterGraph::~FilterGraph()
{
	clear();
}

const Poco::UUID& FilterGraph::id() const
// Liefert den Namen des Filters
{
	return id_;
}


void FilterGraph::unbind(BaseFilter* filter)
// Saemtliche Verbindungen des Filters werden geloescht, der Filter wird aber nicht aus der Map geloescht
{
	// den Filter von den anderen abhaengen
	for (FilterMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
	{
		BaseFilter* compareFilter = it->second->getFilter();

		if (filter != compareFilter)
		{
			// Durchsuche alle Pipes ob diese mit dem delFilter verbunden sind
			for (BaseFilter::Iterator pipeIt = compareFilter->begin(); pipeIt != compareFilter->end(); ++pipeIt)
			{
				if (pipeIt->second->isOutputPipe())
					filter->disconnectPipe(pipeIt->second->pipe());
			}
		}
	}
}

void FilterGraph::bind(BaseFilter* filter)
// Bindet den Filter an die anderen
{
	// den Filter von den anderen abhaengen
	for (FilterMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
	{
		BaseFilter* compareFilter = it->second->getFilter();

		if (filter != compareFilter)
		{
			// Durchsuche alle Pipes ob diese mit dem delFilter verbunden sind
			for (BaseFilter::Iterator pipeIt = compareFilter->begin(); pipeIt != compareFilter->end(); ++pipeIt)
			{
				if (pipeIt->second->isOutputPipe())
					filter->connectPipe(pipeIt->second->pipe());
			}
		}
	}
}


void FilterGraph::insert(const Poco::UUID& id, FilterHandle* filter)
// Neuer Filter in Graph aufnehmen; Wenn die Filterinstance bereits existiert keine Aktion
{
	FastMutex::ScopedLock lock(mutex_);

	FilterMap::iterator it = map_.find(id);
	if (it == map_.end())
	{
		map_[id] = filter;
		filter->getFilter()->setId(id);
	}
}


void FilterGraph::remove(const Poco::UUID& id)
// Loescht einen Filter aus den Graph
{
	FastMutex::ScopedLock lock(mutex_);

	FilterMap::iterator it = map_.find(id);
	if (it != map_.end())
	{
		FilterHandle* delFilter = it->second;

		// Alle anderen Filter von dem zu loeschenden Filter disconnecten
		unbind(delFilter->getFilter());

		// Filter zerstoeren
		Activator::destroyInstance(delFilter);

		// Aus Map entfernen
		map_.erase(it);
	}
}


void FilterGraph::control(AbstractFilterVisitor& visitor)
{
	FastMutex::ScopedLock lock(mutex_);

	// an alle Filter setupdaten senden
	for (FilterMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
	{
		visitor.control(*it->second->getFilter());
	}
}

void FilterGraph::control(const std::list<std::unique_ptr<fliplib::AbstractFilterVisitor>> &visitors)
{
    FastMutex::ScopedLock lock{mutex_};

    for (auto it = map_.begin(); it != map_.end(); ++it)
    {
        auto filter = it->second;
        for (auto &visitor : visitors)
        {
            if (!visitor)
            {
                continue;
            }
            visitor->control(*filter->getFilter());
        }
    }
}

void FilterGraph::controlAccordingToProcessingOrder(AbstractFilterVisitor& visitor)
{
	FastMutex::ScopedLock lock(mutex_);

	//Create an ordered map where the key is the processing index and the value is a vector of filterHandles (supports also the case processingIndex = -1)
    std::map< int, std::vector<FilterHandle *> > oOrderedMap;
    for (FilterMap::const_iterator itSrc = map_.begin(); itSrc != map_.end(); ++itSrc )
    {
        auto processingIndex = itSrc->second->getFilter()->readProcessingIndex();
        oOrderedMap[processingIndex].push_back(itSrc->second);
    }

    
    for (auto it = oOrderedMap.begin(); it != oOrderedMap.end(); ++it)
    {
        for (auto && rpFilterHandle : it->second)
        {
            visitor.control(*rpFilterHandle->getFilter());
        }
    }        

}


BaseFilter* FilterGraph::find(const Poco::UUID& id)
// Sucht einen bestimmten Filter
{
	FastMutex::ScopedLock lock(mutex_);
	FilterMap::iterator it = map_.find(id);
	if (it != map_.end())
	{
		return it->second->getFilter();
	}

	return NULL;
}

void FilterGraph::clear()
// Loescht alle Filter aus den Graph; Die Filterinstancen werden zerstoert
{
	FastMutex::ScopedLock lock(mutex_);

	// Erst alle Verbindungen loesen
	for (FilterMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
		unbind(it->second->getFilter());

	//  Alle Filter loeschen
	for (FilterMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
		it->second->free();

	// Alle Libraries loeschen
	for (FilterMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
		Activator::destroyInstance(it->second);

	// Map loeschen
	map_.clear();
}

void FilterGraph::fire()
// Triggert den Graph. nur Sourcen
{
	FastMutex::ScopedLock lock(mutex_);

	for (FilterMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
	{
		BaseFilter* filter = it->second->getFilter();
		if (filter && filter->getFilterType() == BaseFilterInterface::SOURCE)
			static_cast<SourceFilter *>(filter)->fire();
	}
}

const std::map<Poco::UUID, FilterHandle*>&	FilterGraph::getFilterMap() const {
	return map_;
} // getFilterMap

std::string FilterGraph::toString() const
{
	std::stringstream ss;

	ss << std::endl << "Graph ID:" << id().toString() << std::endl;
	for (FilterMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
	{
		BaseFilter* filter = it->second->getFilter();
		ss << "\tFilter ID:" << filter->id().toString() << " Name:" << filter->name() << std::endl;

		for (BaseFilter::Iterator it = filter->begin(); it != filter->end(); ++it)
		{
			ss << "\t\tPipe Name:" << it->first << std::endl;
		}
	}

	return ss.str();
}

std::string FilterGraph::toStringVerbose() const
{
	std::stringstream ss;

	ss << std::endl << "Graph ID='" << id().toString() << "'" << std::endl;

	for (FilterMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
	{
		BaseFilter* filter = it->second->getFilter();
		std::string xml = filter->toXml();
		ss << xml << std::endl;

		for (BaseFilter::Iterator it = filter->begin(); it != filter->end(); ++it)
		{
			ss << "\t\tPipe Name:" << it->first << std::endl;
		}
	}

	return ss.str();
}


