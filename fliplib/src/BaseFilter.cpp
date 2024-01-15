///////////////////////////////////////////////////////////
//  BaseFilter.cpp
//  Implementation of the Class BaseFilter
//  Created on:      30-Okt-2007 15:11:55
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#include <sstream>

#include "Poco/Bugcheck.h"
#include "Poco/Delegate.h"
#include "Poco/UUIDGenerator.h"

#include "fliplib/BaseFilter.h"
#include "fliplib/BaseDelegate.h"
#include "fliplib/Exception.h"

#include "common/defines.h"

#include "system/tools.h"

#include "module/moduleLogger.h"

// use lock-free atomics, also supporting gcc 4.6.1

#if defined __QNX__ || defined __linux__
#include "wmAtomics.h"
using namespace precitec::utils;
#else
#include "..\..\..\win\wmNative\TheLogger\winAtomics.h"
using namespace Precitec::Utils;
#endif // #ifdef __QNX___

using namespace precitec::system;

// allow using the logger for performance log

#include "module/logType.h"

#ifndef THELOGGER_API
	#define THELOGGER_API // later undefed
#endif

namespace precitec {
	void THELOGGER_API wmLog( LogType, std::string p_oString, ... ); // fwd dec
}


using namespace Poco;
using namespace fliplib;
using Poco::UUID;
using precitec::system::Timer;

std::atomic<int> BaseFilter::sProcessingCounter {0};

BaseFilter::BaseFilter(const std::string& name) :
	filterID_		( UUIDGenerator::defaultGenerator().createRandom() ),
	name_			( name ),
    m_overallProcessingTime( 0 ),
    m_minProcessingTime(std::make_pair(0, std::chrono::nanoseconds::max())),
    m_maxProcessingTime(std::make_pair(0, std::chrono::nanoseconds::zero())),
    m_paintTime( 0 ),
    m_minPaintTime(std::make_pair(0, std::chrono::nanoseconds::max())),
    m_maxPaintTime(std::make_pair(0, std::chrono::nanoseconds::zero())),
    m_paintTimeCounter( 0 ),
    m_oAlwaysEnableTiming(false),
    m_oGraphIndex(-1),
	m_skippedCounter(0),
    m_oTimerCnt     ( 0 ),
	m_oCounter		( 0 ),
	m_oVerbosity	( eLow )
{
	// Gruppenevent abonnieren. Standardmaessig auf Proceed setzen
	groupevent_+= BaseDelegate<BaseFilter, PipeGroupEventArgs>(this, &BaseFilter::timedProceedGroup);
	// add base parameter
	parameters_.add("Verbosity",		Parameter::TYPE_int,	static_cast<int>(m_oVerbosity));
}

BaseFilter::BaseFilter(const std::string& name,  UUID const& filterID) :
	filterID_		( filterID ),
	name_			( name ),
    m_overallProcessingTime( 0 ),
    m_minProcessingTime(std::make_pair(0, std::chrono::nanoseconds::max())),
    m_maxProcessingTime(std::make_pair(0, std::chrono::nanoseconds::zero())),
    m_paintTime( 0 ),
    m_minPaintTime(std::make_pair(0, std::chrono::nanoseconds::max())),
    m_maxPaintTime(std::make_pair(0, std::chrono::nanoseconds::zero())),
    m_paintTimeCounter( 0 ),
    m_oAlwaysEnableTiming(false),
    m_oGraphIndex(-1),
	m_skippedCounter(0),
    m_oTimerCnt     ( 0 ),
	m_oCounter		( 0 ),
	m_oVerbosity	( eLow )
{
	// Gruppenevent abonnieren. Standardmaessig auf Proceed setzen
	groupevent_+= BaseDelegate<BaseFilter, PipeGroupEventArgs>(this, &BaseFilter::timedProceedGroup);
	// add base parameter
	parameters_.add("Verbosity",		Parameter::TYPE_int,	static_cast<int>(m_oVerbosity));
}

BaseFilter::~BaseFilter()
{
	// Gruppenevent loeschen
	groupevent_-= BaseDelegate<BaseFilter, PipeGroupEventArgs>(this, &BaseFilter::timedProceedGroup);

	// PipeMap aufraeumen. Im first steht der Name (string)
	for (PipeMap::const_iterator it = map_.begin(); it != map_.end(); ++it)
	{
		delete it->second;
	}

	attributes_.clear();
}

void BaseFilter::registerPipe(BasePipe *pipe, const std::string& contentType, int channel)
// Registriert eine Pipe im Filter (threadsafe)
{
	poco_check_ptr(pipe);

	registerPipe( pipe, pipe->name(), contentType, channel );

}

void BaseFilter::registerPipe(const std::string& pipeName, const std::string& contentType, int channel)
// Registriert eine Pipe im Filter (threadsafe)
{
	registerPipe( NULL, pipeName, contentType, channel );

}

void BaseFilter::registerPipe(BasePipe *pipe, const std::string& pipeName, const std::string& contentType, int channel)
// Registriert eine Pipe im Filter (threadsafe)
{
	PipeMap::iterator it = map_.find( pipeName );
	if (it == map_.end())
	{
		PipeInfo* info = new PipeInfo(pipeName, contentType, pipe);

		std::stringstream out;
		out << channel;

		info->setAttribute( "channel", out.str() );
		map_[pipeName] = info;
	}
	else
		std::cout << "fliplib BaseFilter::registerOutput; Pipe '" << pipeName << " ' already registered in filter '" << this->name() << "'" << std::endl;
}

void BaseFilter::unregisterPipe(const BasePipe *pipe)
// Loescht die Registrierung der Pipe (threadsafe)
{
	poco_check_ptr(pipe);

	unregisterPipe ( pipe->name() );
}

void BaseFilter::unregisterPipe(const std::string& pipeName)
{
	PipeMap::iterator it = map_.find(pipeName);
	if (it != map_.end())
	{
		delete ( it->second );
		map_.erase(it);
	}
	else
	{
		throw NotFoundException(instanceID_.toString() + " unregisterOutput: Pipe [" + pipeName + "] konnte nicht geloescht werden in Filter:" + name_);
	}
}

bool BaseFilter::isValidConnected() const
{
    return true;
}

BasePipe* BaseFilter::findPipe(const std::string &name) const
// Liefert einen Zieger auf die Pipe in Abhaengigkeit des Namen (threadsafe)
// Wenn keine passende Pipe gefunden wird, liefert diese Funktion NULL zurueck
{
	PipeMap::const_iterator it = map_.find( name );
	if (it != map_.end())
	{
		return it->second->pipe();
	}

	return 0;
}

PipeInfo* BaseFilter::getPipeInfo(const std::string &pipeName) const
{
	PipeMap::const_iterator it = map_.find( pipeName );
	if (it != map_.end())
	{
		return it->second;
	}

	return 0;
}

BaseFilter::Iterator BaseFilter::begin() const
// Liefert den Begin Iterator fuer die foreach Abfrage (threadsafe)
{
	return Iterator(map_.begin());
}

BaseFilter::Iterator BaseFilter::end() const
// Liefert den End Iterator fuer die foreach Abfrage (threadsafe)
{
	return Iterator(map_.end());
}

void BaseFilter::proceed(const void* sender, PipeEventArgs&  e)
// Darf nicht aufgerufen werden!
{
	std::stringstream ss;
	ss << "Methode proceed nicht implementiert in Filter:" << name_ << "::" << instanceID_.toString() << std::endl;
	std::cout << ss.str();
	throw NotImplementedException(ss.str());
}

void BaseFilter::proceedGroup(const void* sender, PipeGroupEventArgs&  e)
// Darf nicht aufgerufen werden!
{
	throw NotImplementedException("Methode proceedGroup nicht implementiert in Filter:" + name_ + "::" + instanceID_.toString());
}

inline void BaseFilter::timedProceed(const void* p_pSender, PipeEventArgs& p_rEventArgs) {
    // only, when the filter counter is equal to the image number of the incoming data, proceed gets invoked
    // otherwise, the filter is still blocked by working on older data
    // NB: the CAS deliberately does not increment the counter, this is done in preSignalAction()
    synchronizeOnImgNb(p_rEventArgs.m_oImgNb);

	if (m_oVerbosity == eMax || m_oAlwaysEnableTiming)
    {
		m_oTimer.restart();
	} // if

	try
	{
		//precitec::wmLog(precitec::eInfo, "%i %s START.\n", m_oCounter, name_.c_str()); // debug
        m_preSignalActionCalled.get() = false;
		proceed(p_pSender, p_rEventArgs);
		// logTiming() called in preSignalAction (due to synchronous case where signal() blocks)
	}
	catch(...) {
		logExcpetion(name_ + "::proceed()", std::current_exception());
	} // catch
    if (!*m_preSignalActionCalled)
    {
        wmLog(precitec::eDebug, "Filter %s did not call preSignalAction\n", name_.c_str());
        preSignalAction();
    }
    m_preSignalActionCalled.get() = false;
} // timedProceed

inline void BaseFilter::timedProceedGroup(const void* p_pSender, PipeGroupEventArgs& p_rGroupEventArgs) {
    // only, when the filter counter is equal to the image number of the data, proceed gets invoked
    // otherwise, the filter is still blocked by working on older data
    // NB: the CAS deliberately does not increment the counter, this is done in preSignalAction()
    synchronizeOnImgNb(p_rGroupEventArgs.m_oImgNb);

	if (m_oVerbosity == eMax || m_oAlwaysEnableTiming)
    {
		m_oTimer.restart();
	} // if

	try
	{
		//precitec::wmLog(precitec::eInfo, "%i %s START.\n", m_oCounter, name_.c_str()); // debug
        m_preSignalActionCalled.get() = false;
		proceedGroup(p_pSender, p_rGroupEventArgs);
		// logTiming() called in preSignalAction (due to synchronous case where signal() blocks)
	}
	catch(...) {
		logExcpetion(name_ + "::proceedGroup()", std::current_exception());
	} // catch
    if (!*m_preSignalActionCalled)
    {
        wmLog(precitec::eDebug, "Filter %s did not call preSignalAction\n", name_.c_str());
        preSignalAction();
    }
    m_preSignalActionCalled.get() = false;
} // timedProceedGroup

bool BaseFilter::subscribe(BasePipe& pipe, int group)
// Abonniert saemtliche Events von der Pipe
{
	if (group)
		groupevent_.add(pipe);
	else
		pipe.install(BaseDelegate<BaseFilter, PipeEventArgs>(this, &BaseFilter::timedProceed));

	return true;
}

bool BaseFilter::unsubscribe(BasePipe& pipe)
// Loescht das Abo fuer die Events
{
	groupevent_.remove(pipe);
	pipe.uninstall(BaseDelegate<BaseFilter, PipeEventArgs>(this, &BaseFilter::timedProceed));

    return true;
}

bool BaseFilter::connectPipe(BasePipe* pipe)
{
	poco_check_ptr(pipe);

	return connectPipe(pipe, 1);
}

bool BaseFilter::connectPipe(BasePipe* pipe, int group)
// Der Graphbuilder versucht einen Pin zu verbinden
{
	poco_check_ptr(pipe);

	return subscribe(*pipe, group);
}

bool BaseFilter::disconnectPipe(BasePipe* pipe)
// Der Graphbuilder versucht einen Pin zu entfernen
{
	poco_check_ptr(pipe);

	return unsubscribe(*pipe);
}

UUID BaseFilter::id() const
// Liefert den Namen des Filters
{
	return instanceID_;
}

void BaseFilter::setId(const UUID& id)
// Setzt die ID des Filters
{
	instanceID_ = id;
}


std::string BaseFilter::name() const
// Liefert den Namen des Filters
{
	return name_;
}

std::string BaseFilter::nameInGraph() const
{
    if (m_oGraphIndex != -1)
    {
        std::string oName =  name_ + std::to_string(m_oGraphIndex);
        if (m_oProcessingIndex != -1)
        {
            oName +=  "_" + std::to_string(m_oProcessingIndex);
        }
        return oName;
    }
    else
    {
        return name_;
    }
}

UUID BaseFilter::filterID() const
// Liefert den Namen des Filters
{
	return filterID_;
}

void BaseFilter::setAttribute(const std::string&  key, const std::string& value)
{
	attributes_.insert( std::pair<std::string, std::string>(key, value) );
}

void BaseFilter::setAttribute(const std::string& pipeName, const std::string&  key, const std::string& value)
{
	PipeInfo* info=getPipeInfo( pipeName );
	if ( info )
		info->setAttribute( key, value );
}

BaseFilter::AttributeMap & BaseFilter::getAttribute()
{
	return attributes_;
}

BaseFilter::AttributeMap & BaseFilter::getAttribute(const std::string& pipeName)
{
	PipeInfo* info=getPipeInfo( pipeName );
	if ( !info )
		throw NullPointerException("PipeInfo " + pipeName + " does not exits!");

	return info->getAttribute();

}

ParameterContainer& BaseFilter::getParameters()
{
	return parameters_;
}

int BaseFilter::pipeCount(bool input) const
{
	int count=0;

	// Pipe auslesen
	for (BaseFilter::Iterator it = begin(); it != end(); ++it)
	{
		if ( ( input && it->second->isInputPipe() ) || ( !input && it->second->isOutputPipe() ) )
			count++;
	}

	return count;
}


std::string BaseFilter::toXml(const std::string& fqName) const
{
	std::stringstream ss;
	ss << "<filter id=\"" << filterID_.toString();

	if (fqName.length() > 0)
		ss << "\" name=\"" << fqName  << "\" ";
	else
		ss << "\" name=\"" << name_  << "\" ";

	// Standard Attribute ausgeben
	ss << "countPipeIn=\"" 	<< pipeCount(true)  << "\" ";
	ss << "countPipeOut=\"" << pipeCount(false) << "\" ";

	// Attribute auslesen
	for (BaseFilter::AttributeMap::const_iterator it = attributes_.begin(); it != attributes_.end(); ++it)
	{
		ss << it->first << "=\"" << it->second << "\" ";
	}

	ss << ">";

	// Parameter auslesen
	ss << parameters_.toXml();

	ss << "<pipes>";

		// Pipe auslesen
		for (BaseFilter::Iterator it = begin(); it != end(); ++it)
		{
			ss << it->second->toXml();
		}

	ss << "</pipes>";
	ss << "</filter>";

	return ss.str();
}


void PipeInfo::setAttribute(const std::string&  key, const std::string& value)
{
	attributes_.insert( std::pair<std::string, std::string>(key, value) );
}

PipeInfo::AttributeMap & PipeInfo::getAttribute()
{
	return attributes_;
}

std::string PipeInfo::toXml() const
{
	std::stringstream ss;

	ss << "<pipe ";
	ss << "name=\"" << name_ << "\" ";
	ss << "type=\"" << (isInputPipe()? "in":"out") << "\" " ;

	int channel = 0;

	for (PipeInfo::AttributeMap::const_iterator it = attributes_.begin(); it != attributes_.end(); ++it)
	{
		// Channel Nr wird automatisch vergeben, wenn nichts abgegeben wird
		if (it->first == "channel" )
		{
			if (it->second == "-1")
				ss << "channel=\"" << channel << "\" ";
			else
			{
				ss << "channel=\"" << it->second << "\" ";
				channel = atoi(it->second.c_str());
			}
			channel++;
		}
		else
			ss << it->first << "=\"" << it->second << "\" ";
	}
	ss << " />";

	return ss.str();
}



/*virtual*/ void BaseFilter::setParameter() {
	m_oVerbosity		= static_cast<VerbosityType>(parameters_.getParameter("Verbosity").convert<int>()); // set base parameter
} // setParameter



void BaseFilter::preSignalAction()
{
    logTiming();

	try
	{
        precitec::system::ElapsedTimer timer;
		paint();    // actually not always needed - hasCanvas() does not help
        logPaintTime(timer.elapsed());

        if (g_oDebugTimings)
        {
        	if (m_oProcessingIndex == -1 && sProcessingCounter != -1 )
        	{
        		//assign a processing index (only the first time, note that the order can be altered if some filters do not send a result)
        		m_oProcessingIndex = sProcessingCounter;
                sProcessingCounter ++;
        	}
        }

	}
	catch(...) {
		logExcpetion(name_ + "::paint()", std::current_exception());
		precitec::wmFatal(precitec::eInternalError, "QnxMsg.Fatal.InternalError", "Filter exception, inspection of parts is not possible anymore.\n");	// worker will starve
	} // catch

    //precitec::wmLog(precitec::eInfo, "%i %s END %i.\n", m_oCounter, name_.c_str(), (int)this); // debug
    Poco::ScopedLock<Poco::FastMutex> lock(m_synchronizationMutex);
    m_oCounter++;
    m_preSignalActionCalled.get() = true;
    m_synchronization.broadcast();
}



void BaseFilter::setCounter(int p_oCount)
{
    Poco::ScopedLock<Poco::FastMutex> lock(m_synchronizationMutex);
    m_oCounter =   p_oCount;
} // setCounter



void BaseFilter::resetTimerCnt()
{
    m_oTimerCnt =   0;
    m_overallProcessingTime = std::chrono::nanoseconds::zero();
    m_minProcessingTime = std::make_pair(0, std::chrono::nanoseconds::max());
    m_maxProcessingTime = std::make_pair(0, std::chrono::nanoseconds::zero());

    m_paintTimeCounter = 0;
    m_paintTime = std::chrono::nanoseconds::zero();
    m_minPaintTime = std::make_pair(0, std::chrono::nanoseconds::max());
    m_maxPaintTime = std::make_pair(0, std::chrono::nanoseconds::zero());

    m_skippedCounter = 0;
} // resetTimerCnt



void BaseFilter::resetSignalCntGroupEvent()
{
	groupevent_.resetSignalCounters();
} // resetSignalCntGroupEvent


void BaseFilter::resetSignalCountGroupEvent(int imageNumber)
{
	groupevent_.resetSignalCounter(imageNumber);
}


inline void BaseFilter::logTiming()
{
    if (m_oVerbosity != eMax && !m_oAlwaysEnableTiming)
    {
        return;
    }
    const auto elapsed = m_oTimer.elapsed();
    m_overallProcessingTime += elapsed;
    if (elapsed < m_minProcessingTime.second)
    {
        m_minProcessingTime = std::make_pair(m_oTimerCnt, elapsed);
    }
    if (elapsed > m_maxProcessingTime.second)
    {
        m_maxProcessingTime = std::make_pair(m_oTimerCnt, elapsed);
    }
    ++m_oTimerCnt;
} // logTiming

void BaseFilter::logPaintTime(const std::chrono::nanoseconds &elapsed)
{
    if (m_oVerbosity != eMax && !m_oAlwaysEnableTiming)
    {
        return;
    }
    m_paintTime += elapsed;
    if (elapsed < m_minPaintTime.second)
    {
        m_minPaintTime = std::make_pair(m_paintTimeCounter, elapsed);
    }
    if (elapsed > m_maxPaintTime.second)
    {
        m_maxPaintTime = std::make_pair(m_paintTimeCounter, elapsed);
    }
    ++m_paintTimeCounter;
}

void BaseFilter::logProcessingTime()
{
    if (m_oVerbosity != eMax && !m_oAlwaysEnableTiming)
    {
        return;
    }

    std::string oDescription = nameInGraph();

    const float minTime = std::chrono::duration<float, std::chrono::milliseconds::period>(m_minProcessingTime.second).count();
    const float maxTime = std::chrono::duration<float, std::chrono::milliseconds::period>(m_maxProcessingTime.second).count();
    const auto oMeanTiming = std::chrono::duration<float, std::chrono::milliseconds::period>(m_overallProcessingTime).count() / m_oTimerCnt;

    const float minPaintTime = std::chrono::duration<float, std::chrono::milliseconds::period>(m_minPaintTime.second).count();
    const float maxPaintTime = std::chrono::duration<float, std::chrono::milliseconds::period>(m_maxPaintTime.second).count();
    const auto meanPaintTiming = std::chrono::duration<float, std::chrono::milliseconds::period>(m_paintTime).count() / m_paintTimeCounter;

    precitec::wmLog(precitec::eInfo, "Runtime [ms] of %s      \t: mean %f; min %f; max %f \n",
                    oDescription.c_str(), oMeanTiming,minTime, maxTime);
    precitec::wmLog(precitec::eInfo, "Runtime info of %s      \t: runs = %i; min in frame %i; max in frame %i, skipped frames= %i \n",
                    oDescription.c_str(), m_oTimerCnt, m_minProcessingTime.first, m_maxProcessingTime.first, m_skippedCounter);

    precitec::wmLog(precitec::eInfo, "Paint runtime [ms] of %s\t: mean %f; min %f; max %f \n",
                    oDescription.c_str(), meanPaintTiming, minPaintTime, maxPaintTime);
    precitec::wmLog(precitec::eInfo, "Paint runtime info of %s\t: runs = %i; min in frame %i; max in frame %i \n",
                    oDescription.c_str(), m_paintTimeCounter, m_minPaintTime.first,m_maxPaintTime.first);

}

void BaseFilter::synchronizeOnImgNb(int p_oImgNb)
{
	if (g_oNbPar == 1)	// no parallelism
	{
		return;			// safe way: allow special filters to work, which manipulate image number, discard frames, ...
						// for now, also necessary for simulation, which works single threaded
	} // if

    if (p_oImgNb < m_oCounter)
    {
        wmLog(precitec::eError, "Synchronization error: Filter %s should wait for image number %d, but current counter is already %d \n", nameInGraph().c_str(), p_oImgNb, m_oCounter);
        return;
    }

    m_synchronizationMutex.lock();

    if (g_oDebugTimings)
    {
        const long int wait_ms = 2;
        precitec::LogType oLogType =  precitec::eDebug;

        int waitAttempts = 0;

        while (m_oCounter != p_oImgNb)
        {
            if (!m_synchronization.tryWait(m_synchronizationMutex, wait_ms))
            {
               ++waitAttempts;
            }
            if(waitAttempts )
            {
                wmLog(oLogType, "Filter %s has been waiting %d ms in synchronizeOnImgNb(%d - current counter %d) \n",
                    nameInGraph().c_str(), waitAttempts*wait_ms, p_oImgNb, m_oCounter);
            }
        }
    }
    else
    {
        while (m_oCounter != p_oImgNb)
        {
            m_synchronization.wait(m_synchronizationMutex);
        }
    }



    m_synchronizationMutex.unlock();
} // synchronizeOnImgNb

void BaseFilter::ensureImageNumber(int imageNumber)
{
    Poco::ScopedLock<Poco::FastMutex> lock(m_synchronizationMutex);
    m_oCounter = std::max(m_oCounter, imageNumber);
    m_synchronization.broadcast();
}

void BaseFilter::skipImageProcessing(int imageNumber)
{
    synchronizeOnImgNb(imageNumber);


	if (m_oVerbosity == eMax  || m_oAlwaysEnableTiming)
    {
		m_oTimer.restart();
        ++m_skippedCounter;
	} // if

    preSignalAction();
}


void BaseFilter::alwaysEnableTiming(bool p_oValue)
{
    m_oAlwaysEnableTiming = p_oValue;
}

void BaseFilter::setGraphIndex(int index)
{
    m_oGraphIndex = index;
}


