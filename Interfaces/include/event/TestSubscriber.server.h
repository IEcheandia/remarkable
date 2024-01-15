/*
 * TestSubscriber.server.h
 *
 *  Created on: 15.07.2010
 *      Author: Adrian Garcea
 */

#ifndef TESTSUBSCRIBER_SERVER_H_
#define TESTSUBSCRIBER_SERVER_H_

#include <iostream>

#include "system/types.h"					// wg String

#include "event/TestSubscriber.interface.h"

namespace precitec {

namespace interface {

/**
 * Test
 */
template<> class TTestSubscriber<EventServer> : public TTestSubscriber<
		AbstractInterface> {
public:
	TTestSubscriber() {
	}
	virtual ~TTestSubscriber() {
	}
public:
	virtual void printFieldSum(Field const& f) {
		std::cout << "FieldSum: " << f.sumField() << std::endl;
	}
};

} // namespace interface

} // namespace precitec


#endif /* TESTSUBSCRIBER_SERVER_H_ */
