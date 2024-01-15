/*
 * VICallbackBase.h
 *
 *  Created on: 08.02.2010
 *      Author: f.agrawal
 */

#ifndef VICALLBACKBASE_H_
#define VICALLBACKBASE_H_
#include <GlobalDefs.h>
#include "event/viWeldHeadPublish.h"
#include "event/viWeldHeadSubscribe.h"
#include "event/viService.h"

using namespace precitec::interface;

//*****************
//** Base class (all callback-callers use this class)
//*****************
class VICallbackBase {
public:
	VICallbackBase(){};
	virtual ~VICallbackBase(){};

	virtual void operator()(HeadAxisID axis, MotionMode mode, int value) = 0;
	virtual void operator()(HeadAxisID axis, MotionMode mode) = 0;
	virtual void operator()(HeadAxisID axis, ErrorCode errorCode, int value) = 0;
};

//*****************
//**All concrete callback objects instantiate from this class
//*****************
template<class C>
class VIMainCallback: public VICallbackBase {
public:
	VIMainCallback(C &obj, void(C::*method)(HeadAxisID axis, MotionMode mode, int value)) :	m_obj(obj), m_headValueReached(method) {
	}
	VIMainCallback(C &obj, void(C::*method)(HeadAxisID axis, MotionMode mode)) :	m_obj(obj), m_HeadIsReady(method) {
	}
	VIMainCallback(C &obj, void(C::*method)(HeadAxisID axis, ErrorCode errorCode, int value)) :	m_obj(obj), m_setHeadError(method) {
	}

	virtual void operator()(HeadAxisID axis, MotionMode mode, int value) {
		(m_obj.*m_headValueReached)(axis,mode,value);
	}
	virtual void operator()(HeadAxisID axis, MotionMode mode) {
		(m_obj.*m_HeadIsReady)(axis, mode);
	}
	virtual void operator()(HeadAxisID axis, ErrorCode errorCode, int value) {
		(m_obj.*m_setHeadError)(axis,errorCode,value);
	}

private:
	C &m_obj; // Objekt, dessen Memberfunktion aufgerufen werden soll

	void (C::*m_headValueReached)(HeadAxisID axis, MotionMode mode, int value);
	void (C::*m_HeadIsReady)(HeadAxisID axis, MotionMode mode);
	void (C::*m_setHeadError)(HeadAxisID axis, ErrorCode errorCode, int value);

};

#endif /* VICALLBACKBASE_H_ */

