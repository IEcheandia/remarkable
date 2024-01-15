/*
 * IWatchDog.h
 *
 *  Created on: 18.04.2017
 *      Author: root
 */

#ifndef IWATCHDOGTIMER_H_
#define IWATCHDOGTIMER_H_

class IWatchdogTimer
{

public:
	virtual void start() = 0;
	virtual void resetWatchdog() = 0;
};

#endif /* IWATCHDOGTIMER_H_ */
