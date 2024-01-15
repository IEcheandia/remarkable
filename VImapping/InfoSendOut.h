/*
 * InfoSend.h
 *
 *  Created on: 22.02.2010
 *      Author: f.agrawal
 */

#ifndef InfoSendOut_H_
#define InfoSendOut_H_

#include <SendOut.h>
#include <BaseInfo.h>



template<typename T>
class InfoSendOutValue : public BaseInfo {
public:
	InfoSendOutValue(SendOut<T>* sender, unsigned short slaveOutBits, unsigned int productCode, unsigned int instance, unsigned int startByte, unsigned int writeBits);

	virtual ~InfoSendOutValue();

	SendOut<T>* m_sender;


	unsigned int m_productCode;
	unsigned int m_instance;

	unsigned int m_startBit;

};




template<typename T>
InfoSendOutValue<T>::InfoSendOutValue(SendOut<T>* sender, unsigned short slaveOutBits, unsigned int productCode, unsigned int instance, unsigned int startByte, unsigned int writeBits)
	:BaseInfo(/*SENDOUT_VALUE*/1,slaveOutBits){

	m_sender = sender;
	m_productCode = productCode;
	m_instance = instance;
	m_startBit = startByte;
	m_writeBits = writeBits;

}

template<typename T>
InfoSendOutValue<T>::~InfoSendOutValue() {

}


//#############################################################

template<typename T>
class InfoSendOutTrigger : public BaseInfo {
public:
	InfoSendOutTrigger(SendOut<T>* sender, unsigned short slaveOutBits, unsigned int productCode, unsigned int instance, unsigned int bitNr);
//	bool operator == (const InfoSendOutTrigger<T> &d2) const;
	virtual ~InfoSendOutTrigger();

	SendOut<T>* m_sender;

	unsigned int m_productCode;
	unsigned int m_instance;
	unsigned int m_bitNr;

};

//template<typename T>
//bool InfoSendOutTrigger<T>::operator == (const InfoSendOutTrigger<T> &d2) const
// {
//   bool ret = false;
//   return (m_productCode == d2.m_productCode && m_instance == d2.m_instance);
//
// }


template<typename T>
InfoSendOutTrigger<T>::InfoSendOutTrigger(SendOut<T>* sender, unsigned short slaveOutBits, unsigned int productCode, unsigned int instance, unsigned int bitNr)
	:BaseInfo(/*SENDOUT_TRIGGER*/2,slaveOutBits){

	m_sender = sender;

	m_productCode = productCode;
	m_instance = instance;
	m_bitNr = bitNr;

}

template<typename T>
InfoSendOutTrigger<T>::~InfoSendOutTrigger() {
	delete m_sender;
	m_sender = NULL;
}


#endif /* InfoSendOut_H_ */
