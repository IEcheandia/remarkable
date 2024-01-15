/*
 * WatchdogTimer.h
 *
 *  Created on: 18.04.2017
 *      Author: AL
 */

#ifndef WATCHDOGTIMER_H_
#define WATCHDOGTIMER_H_

#include "Poco/Timer.h"
#include "common/IWatchdogTimeoutNotify.h"
#include "common/IWatchdogTimer.h"
#include <iostream>

namespace precitec {
namespace workflow {

class WatchdogTimer : public IWatchdogTimer{
public:
	WatchdogTimer(int timeoutInMiliseconds, IWatchdogTimeoutNotify& watchdogTimeoutNotify);
	virtual ~WatchdogTimer();
	void resetWatchdog();
	void start();

private:
	Poco::Timer m_timer;
	Poco::TimerCallback<WatchdogTimer> m_callback;
	IWatchdogTimeoutNotify& m_watchdogTimeoutNotify;

	void onTimer(Poco::Timer& timer);
};
}
}

#endif /* WATCHDOGTIMER_H_ */
