/*
 * TestSubscriber.handler.h
 *
 *  Created on: 15.07.2010
 *      Author: Adrian Garcea
 */

#ifndef TESTSUBSCRIBER_HANDLER_H_
#define TESTSUBSCRIBER_HANDLER_H_

#include <iostream>
#include <string> // std::string

#include "server/eventHandler.h"
#include "event/TestSubscriber.interface.h"

namespace precitec {

namespace interface {

template<> class TTestSubscriber<EventHandler> : public Server<EventHandler> {
public:
	EVENT_HANDLER( TTestSubscriber);

public:
	void registerCallbacks() {
		REGISTER_EVENT1(TTestSubscriber, printFieldSum, Field);
	}

	void CALLBACK1(printFieldSum, Field)
(Receiver &receiver) {
	Field f;
	receiver.deMarshal(f);
	server_->printFieldSum(f);
}
};

} // namespace interface

} // namespace precitec

#endif /* TESTSUBSCRIBER_HANDLER_H_ */
