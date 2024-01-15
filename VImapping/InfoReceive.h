
/*
 * Send.h
 *
 *  Created on: 22.02.2010
 *      Author: f.agrawal
 */

#ifndef InfoReceive_H_
#define InfoReceive_H_

#include <Receive.h>
#include <BaseInfo.h>
#include <T8BitDigIPC.h>

#define SENDOUT_VALUE	1
#define SENDOUT_TRIGGER	2

class VI_InspectionControl;
class MappedReceiver;

template<typename T>
class InfoReceive : public BaseInfo {

public:

	InfoReceive(Receive<T>& receiver,MainCallback<VI_InspectionControl> &cb, unsigned short slaveOutBits, unsigned int productCode, unsigned int instance, unsigned int startBit, unsigned int length);
	virtual ~InfoReceive();
	MainCallback<VI_InspectionControl> &m_cb;
	Receive<T>& m_receiver;
	unsigned int m_productCode;
	unsigned int m_instance;
	unsigned int m_startBit;
	unsigned int m_lenth;
};


template<typename T>
InfoReceive<T>::InfoReceive(Receive<T>& receiver,MainCallback<VI_InspectionControl> &cb, unsigned short slaveOutBits, unsigned int productCode, unsigned int instance, unsigned int startBit, unsigned int length)
	:BaseInfo(SENDOUT_TRIGGER,slaveOutBits),m_cb(cb),m_receiver(receiver){

	m_productCode = productCode;
	m_instance = instance;
	m_startBit = startBit;
	m_lenth = length;

}

template<typename T>
InfoReceive<T>::~InfoReceive() {

}


#endif /* InfoSendOut_H_ */
