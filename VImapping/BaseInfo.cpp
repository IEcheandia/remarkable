/*
 * BaseInfo.cpp
 *
 *  Created on: 28.04.2010
 *      Author: f.agrawal
 */

#include "BaseInfo.h"

BaseInfo::BaseInfo(short type, short slaveOutBits) {

	m_type = type;
	m_slaveOutBits = slaveOutBits;
	m_writeBits = 1;

}

BaseInfo::~BaseInfo() {

}
