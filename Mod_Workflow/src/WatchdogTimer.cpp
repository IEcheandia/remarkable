/*
 * WatchdogTimer.cpp
 *
 *  Created on: 18.04.2017
 *      Author: AL
 */

#include "../include/workflow/WatchdogTimer.h"

namespace precitec {
namespace workflow {

WatchdogTimer::WatchdogTimer(int timeoutInMiliseconds, IWatchdogTimeoutNotify& watchdogTimeoutNotify):
	m_timer (timeoutInMiliseconds, timeoutInMiliseconds),
	m_callback(*this, &WatchdogTimer::onTimer),
	m_watchdogTimeoutNotify(watchdogTimeoutNotify)
{
}

WatchdogTimer::~WatchdogTimer()
{
}

void WatchdogTimer::start()
{
	m_timer.start(m_callback);
}

void WatchdogTimer::resetWatchdog()
{
	m_timer.restart();
}

void WatchdogTimer::onTimer(Poco::Timer& timer)
{
	timer.restart(0);
	m_watchdogTimeoutNotify.onWatchdogTimeout();

}

}
}
