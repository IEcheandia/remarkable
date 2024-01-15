/*
 * IWatchdogNotify.h
 *
 *  Created on: 18.04.2017
 *      Author: root
 */

#ifndef IWATCHDOGTIMEOUTNOTIFY_H_
#define IWATCHDOGTIMEOUTNOTIFY_H_

class IWatchdogTimeoutNotify
{

public:
	virtual void onWatchdogTimeout() = 0;
};

#endif /* IWATCHDOGTIMEOUTNOTIFY_H_ */
