/*
 * BaseInfo.h
 *
 *  Created on: 28.04.2010
 *      Author: f.agrawal
 */

#ifndef BASEINFOSENDOUT_H_
#define BASEINFOSENDOUT_H_

#define RECEIVE_VALUE	1
#define RECEIVE_TRIGGER	2

class BaseInfo {
public:

	short m_type;
	short m_slaveOutBits;
	unsigned int m_writeBits;

	BaseInfo(short type, short slaveBits);
	virtual ~BaseInfo();
};

#endif //BASEINFOSENDOUT_H_
