/*
 * GlobalDefs.h
 *
 *  Created on: 24.03.2010
 *      Author: f.agrawal
 */

#ifndef GLOBALDEFS_H_
#define GLOBALDEFS_H_

#include <errno.h>
#include <signal.h>
#include <stdint.h>

/*
 * EtherCAT slave Type defs
 */
#define _DIG_8BIT_ 8
#define _DIG_16BIT_ 16
#define _ANALOG_CHAN1_ -1
#define _ANALOG_CHAN2_ -2
#define _ANALOG_CHAN3_ -3
#define _ANALOG_CHAN4_ -4
#define _GATEWAY_	-10

#define _OVERSAMPLING_SAMPLES_  100
#define _GATEWAY_DATA_SIZE_  	20

#define HEADERSIZE 4
#define _BLOCKSIZE_ 10000 //TODO: size???

#define SEND_ITEM_SIZE	6000

/**
 * Message-Struktur
 **/
struct MESSAGE{
	short ID;			  			///<ID der Message
	short Length;					///<Anzahl Datenbyte
	char  Data[_BLOCKSIZE_]; 		///<Daten
};

/**
 * struct for MessagePassing informations (e.g. register)
 **/

#ifdef __linux__
struct _pulse {
	uint16_t		type;
	uint16_t		subtype;
	int8_t			code;
	uint8_t			zero[3];
	union sigval	value;
	int32_t			scoid;
};
#endif

struct _ipcData {
    _pulse hdr; ///< Header
    MESSAGE data; ///< Data
};

/**
 * ThreadStruktur
 **/
struct PROXY_ID_STRUCT{
	unsigned long nProductCode; ///< ProductCode
	unsigned short nCountInstance; ///< Instanz
	unsigned long nVendorID; ///< VendorID

};

//TCP-MESSAGES:

#define ETHERCAT_START	1
#define ETHERCAT_STOP	2
#define ETHERCAT_GETSTATE 9
#define ETHERCAT_MASTER_PROXIES_ONLINE 3

#define ETHERCAT_REQUEST_SLAVE_INFO	4
#define ETHERCAT_REPLY_OK 5

#define ETHERCAT_WIN_WCF_SERVICE_ONLINE 6
#define ETHERCAT_WIN_WCF_REQUEST_SLAVEINFO 7
#define ETHERCAT_WIN_WCF_SEND_SLAVEINFO 8

#define SDO_REQUEST  1
#define SDO_WRITE  2

/* EtherCAT state */
typedef enum MY_EC_T_STATE
{
    UNKNOWN  = 0,                        /*< unknown */
    INIT     = 1,                        /*< init */
    PREOP    = 2,                        /*< pre-operational */
    SAFEOP   = 4,                        /*< safe operational */
    OP       = 8                        /*< operational */
} EtherCATState;

/* EtherCAT slave Type */
typedef enum ECATType
{
    Unknwown = 0,
	DigIn8Bit  = 1,
    DigOut8Bit,
    DigIn16Bit,
    AnalogDigIn2Channels,
    DigOut16Bit,
    MotionServo,
    AnalogDigOversamplingIn2Channels,
    AnalogDigOut2Channels,
    EncoderInput,
    Gateway

} ECATType;

#endif /* GLOBALDEFS_H_ */
