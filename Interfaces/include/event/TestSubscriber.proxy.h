/*
 * TestSubscriber.proxy.h
 *
 *  Created on: 15.07.2010
 *      Author: Adrian Garcea
 */

#ifndef TESTSUBSCRIBER_PROXY_H_
#define TESTSUBSCRIBER_PROXY_H_

#include "server/proxy.h"
#include "event/TestSubscriber.interface.h"

namespace precitec {

namespace interface {

template<> class TTestSubscriber<EventProxy> : public Server<EventProxy> ,
		public TTestSubscriber<AbstractInterface> {
public:
	TTestSubscriber() :
		Server<EventProxy> (TTestSubscriber<Messages> ().info),
				TTestSubscriber<AbstractInterface> () {
	}

	virtual void printFieldSum(Field const& f) {
		INIT_EVENT1(TTestSubscriber, printFieldSum, Field);
		//signaler().initMessage(Msg::index);
		signaler().marshal(f);
		signaler().send();
	}
};

} // namespace interface


} // namespace precitec

#endif /* TESTSUBSCRIBER_PROXY_H_ */
