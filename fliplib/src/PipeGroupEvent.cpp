///////////////////////////////////////////////////////////
//  PipeGroupEvent.cpp
//  Implementation of the Class PipeGroupEvent
//  Created on:      30-Okt-2007 14:29:47
//  Original author: Sevitec
///////////////////////////////////////////////////////////
#include <cassert>

#include "Poco/Delegate.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/BaseDelegate.h"
#include "fliplib/BasePipe.h"
#include "fliplib/PipeGroupEvent.h"


using Poco::Delegate;
using Poco::BasicEvent;
using fliplib::PipeGroupEvent;
using fliplib::PipeGroupEventArgs;
using fliplib::BasePipe;

static const auto g_oInitSigCnt    =    std::array<bool, g_oNbParMax>{{ false }};  

PipeGroupEvent::PipeGroupEvent() :
	members_(0)
{
}

PipeGroupEvent::~PipeGroupEvent()
{
	clear();
	list_.clear();
	members_ = 0;
}

void PipeGroupEvent::add(BasePipe &pipe)
{
	if (std::find(list_.begin(), list_.end(), &pipe) != list_.end())
    {
        return;
    }

	list_.push_back(&pipe);
	pipe.install(BaseDelegate<PipeGroupEvent, fliplib::PipeEventArgs>(this, &PipeGroupEvent::signalhandler));
	members_++;
	signalerCounters_[&pipe] = g_oInitSigCnt;
}

void PipeGroupEvent::remove(BasePipe &pipe)
{
    const auto oFound = std::find(list_.begin(), list_.end(), &pipe);
    if (oFound == list_.cend())
    {
        return;
    }

	members_--;
    list_.erase(oFound);
	signalerCounters_.erase(&pipe);
	pipe.uninstall(BaseDelegate<PipeGroupEvent, fliplib::PipeEventArgs>(this, &PipeGroupEvent::signalhandler));
}



void PipeGroupEvent::signalhandler(const void* sender, fliplib::PipeEventArgs& e)
{
	// Wenn gueltige Daten vorhanden sind, wird der Signaler inkrementiert.

    const auto oIdx                     = e.m_oImgNb % g_oNbPar;

    assert(signalerCounters_[e.pipe()][oIdx] == false); 
    assert(e.m_oImgNb == e.pipe()->getImageNumber(e.m_oImgNb));

	signalerCounters_[e.pipe()][oIdx] = true/*signaled*/;

    //precitec::wmLog(precitec::eInfo, "%i %s %s OK.\n", e.pipe()->getImageNumber(), e.pipe()->parent_->name().c_str(), e.pipe()->name().c_str()); // debug

    // beim Verwerf-Filter kann es vorkommen, dass eine Pipe mehrmals signalisiert wird, bevor es dann endlich zur Abarbeitung kommt. 
	// wir haben jedenfalls neue Daten in dieser
	// Pipe erhalten. Sind diese neuen Daten evtl. neuer als die Daten in anderen Pipes? In dem Fall
	// sind die Daten der betroffenen anderen Pipes zu loeschen, d.h. deren Signal ist zurueck zu nehmen.
	// Diese Strategie erfordert, dass das Schreiben der neuen Daten vor dem entsprechenden Signal erfolgt, aber das ist gegeben.
	
    const fliplib::BasePipe* pPipeThis = e.pipe();
	int oImageNumThis = e.m_oImgNb;
	for (auto oIt = signalerCounters_.begin(); oIt != signalerCounters_.end(); ++oIt)
	{
		const fliplib::BasePipe* pPipeOther = oIt->first;

		if (pPipeOther != pPipeThis && oIt->second[oIdx] != 0 && pPipeOther->getImageNumber(e.m_oImgNb) < oImageNumThis) // Eigene Pipe nicht betrachten.
		{
			// Falls diese Pipe signalisiert wurde... Daten der anderen Pipe sind tatsaechlich veraltet, Signal zuruecksetzen:
  			oIt->second[oIdx] = false/*not signaled*/;
		}
	}

	// only notify when all different in pipes have signaled and reset signal counters
    
    const auto oAllPipesSignaled    =   std::all_of(std::begin(signalerCounters_), std::end(signalerCounters_), 
        [oIdx](signaled_map_t::value_type& p_rVal) { return p_rVal.second[oIdx] == true/*signaled*/; } );
    if (oAllPipesSignaled == false)
    {
        // otherwise, return
        return;
    }

    // reset counter

    std::for_each(std::begin(signalerCounters_), std::end(signalerCounters_), 
        [oIdx](signaled_map_t::value_type& p_rVal) { p_rVal.second[oIdx] = false/*not signaled*/; } );

    // and finally notify

    PipeGroupEventArgs groupEventArgs(members_, this, e.m_oImgNb);
	this->notify(this, groupEventArgs);
}

fliplib::BasePipe* PipeGroupEvent::pipe(const std::string& name) const
{
	for (SenderPipeList::const_iterator it = list_.begin(); it != list_.end(); ++it)
		if ((*it)->name() == name)
			return (*it);
	return NULL;
}

fliplib::BasePipe* PipeGroupEvent::pipeByTag(const std::string& tag)
{
	for (SenderPipeList::const_iterator it = list_.begin(); it != list_.end(); ++it)
		if ((*it)->tag() == tag)
			return (*it);
	return NULL;
}


fliplib::BasePipe* PipeGroupEvent::pipe(const std::type_info& type)
{
	for (SenderPipeList::const_iterator it = list_.begin(); it != list_.end(); ++it)
		if ((*it)->type() == type)
			return (*it);
	return NULL;
}

bool PipeGroupEvent::exists(const std::string& name) const
{
	return (pipe( name ) != NULL);
}

int PipeGroupEvent::count() const
{
	return list_.size();
}

void PipeGroupEvent::resetSignalCounters() {
	for(auto oIt	= std::begin(signalerCounters_); oIt != std::end(signalerCounters_); ++oIt) {
		oIt->second = g_oInitSigCnt;
	} // for
} // resetSignalCounters


void PipeGroupEvent::resetSignalCounter(int imageNumber)
{
	const auto index = imageNumber % g_oNbPar;
	for (auto it = std::begin(signalerCounters_); it != std::end(signalerCounters_); ++it)
	{
		it->second.at(index) = false;
	}
}

