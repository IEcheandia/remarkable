/*
 * TestSubscriber.interface.h
 *
 *  Created on: 15.07.2010
 *      Author: Adrian Garcea
 */

#ifndef TESTSUBSCRIBER_INTERFACE_H_
#define TESTSUBSCRIBER_INTERFACE_H_

#include "system/types.h"				// wg PvString
#include "server/interface.h"
#include "module/interfaces.h" 	// wg module::TestServer
#include "message/serializer.h"
#include "common/testClass.h"

/**
 * abstract Base Class and Message definition;
 *
 * The abstract Base Class is inherited in the
 * local implementations(<EventServer> an <EventHandler>)
 * and remote implementations (<EventHandler>)
 *
 */
namespace precitec {

using system::module::TestServer;
using namespace system;
using namespace message;

namespace interface {

template<int CallType> class TTestSubscriber;

//ABstractInterface
template<> class TTestSubscriber<AbstractInterface> {
public:
	TTestSubscriber() {
	}
	virtual ~TTestSubscriber() {
	}

	virtual void printFieldSum(Field const& f) = 0;
};

//Message Definition
template<> class TTestSubscriber<Messages> : public Server<Messages> {
public:
	TTestSubscriber() :
		info(TestSubscriber, sendBufLen, replyBufLen, NumMessages) {
	}
	MessageInfo info;
private:
	//constants
	enum {Bytes = 1, KBytes = 1024, MBytes = 1024* 1024 };
	enum {sendBufLen = 200*KBytes, replyBufLen = 200*KBytes};
public:
	MESSAGE_LIST1(
			TestSubscriber,
			MESSAGE_NAME1(printFieldSum, Field)
			);
public:
	DEFINE_MSG1(void,printFieldSum, Field);
};

} // namespace interface
}// namespace precitec

#endif /* TESTSUBSCRIBER_INTERFACE_H_ */
