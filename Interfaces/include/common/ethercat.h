/*
 * ethercat.h
 *
 *  Created on: 17.08.2010
 *      Author: f.agrawal
 */
#if defined __linux__
#ifndef ETHERCAT_H_
#define ETHERCAT_H_


#include "EcType.h"

#define ECAT_DEVICE_NAMESIZE                80

struct EC_T_GET_SLAVE_INFO
{
    EC_T_DWORD                  dwScanBusStatus;    /* 0x00 */  /*< Status during last Bus Scan */

    EC_T_DWORD	            dwVendorId;         /* 0x01 */  /*< Vendor Identification */
    EC_T_DWORD	            dwProductCode;      /* 0x02 */  /*< Product Code */
    EC_T_DWORD	            dwRevisionNumber;   /* 0x03 */  /*< Revision Number */
    EC_T_DWORD	            dwSerialNumber;     /* 0x04 */  /*< Serial Number */

    EC_T_WORD                   wPortState;         /* 0x05 */  /*< out port link state (SB Instance)*/
    EC_T_WORD                   wReserved;                      /*< Res */

    EC_T_BOOL                   bDcSupport;         /* 0x06 */  /*< out slave does support DC*/
    EC_T_BOOL                   bDc64Support;       /* 0x07 */  /*< out slave does support 64Bit DC*/

    EC_T_WORD                   wAliasAddress;      /* 0x08 */  /*< out slave alias address*/
    EC_T_WORD                   wPhysAddress;                   /*< out slave physical address*/

    EC_T_DWORD                  dwPdOffsIn;         /* 0x09 */  /*< out process data offset of Input Data (in Bits)*/
    EC_T_DWORD                  dwPdSizeIn;         /* 0x0A */  /*< out process data size of Input Data (in Bits)*/
    EC_T_DWORD                  dwPdOffsOut;        /* 0x0B */  /*< out process data offset of Output Data (in Bits)*/
    EC_T_DWORD                  dwPdSizeOut;        /* 0x0C */  /*< out process data size of Output Data*/
    EC_T_DWORD                  dwPdOffsIn2;        /* 0x0D */  /*< out process data offset of Input data (in Bits)*/
    EC_T_DWORD                  dwPdSizeIn2;        /* 0x0E */  /*< out process data size of Input Data (in Bits)*/
    EC_T_DWORD                  dwPdOffsOut2;       /* 0x0F */  /*< out process data offset of Output Data (in Bits)*/
    EC_T_DWORD                  dwPdSizeOut2;       /* 0x10 */  /*< out process data size of Output Data*/
    EC_T_DWORD                  dwPdOffsIn3;        /* 0x11 */  /*< out process data offset of Input Data (in Bits)*/
    EC_T_DWORD                  dwPdSizeIn3;        /* 0x12 */  /*< out process data size of Input Data (in Bits)*/
    EC_T_DWORD                  dwPdOffsOut3;       /* 0x13 */  /*< out process data offset of Output Data (in Bits)*/
    EC_T_DWORD                  dwPdSizeOut3;       /* 0x14 */  /*< out process data size of Output Data*/
    EC_T_DWORD                  dwPdOffsIn4;        /* 0x15 */  /*< out process data offset of Input Data (in Bits)*/
    EC_T_DWORD                  dwPdSizeIn4;        /* 0x16 */  /*< out process data size of Input Data (in Bits)*/
    EC_T_DWORD                  dwPdOffsOut4;       /* 0x17 */  /*< out process data offset of Output Data (in Bits)*/
    EC_T_DWORD                  dwPdSizeOut4;       /* 0x18 */  /*< out process data size of Output Data*/

    EC_T_WORD                   wCfgPhyAddress;     /* 0x19 */  /*< out slave configured physical address*/
    EC_T_WORD                   wReserved2;                     /*< reserved */

    EC_T_CHAR                   abyDeviceName[ECAT_DEVICE_NAMESIZE];
                                                    /* 0x1A */  /*< out slave name of configuration*/
    EC_T_BOOL                   bIsMailboxSlave;    /* 0x6A */  /*< out whether slave support mailboxes*/
    EC_T_DWORD                  dwMbxOutSize;       /* 0x6B */  /*< out mailbox 1 output size*/
    EC_T_DWORD                  dwMbxInSize;        /* 0x6C */  /*< out mailbox 1 input size*/
    EC_T_DWORD                  dwMbxOutSize2;      /* 0x6D */  /*< out mailbox 2 output size*/
    EC_T_DWORD                  dwMbxInSize2;       /* 0x6E */  /*< out mailbox 2 input size*/

    EC_T_DWORD                  dwErrorCode;        /* 0x6F */  /*< out last return code*/
    EC_T_DWORD                  dwSBErrorCode;      /* 0x70 */  /*< out last return value from SB*/

    EC_T_BYTE                   byPortDescriptor;   /* 0x71 */  /*< out Port Descriptor */
    EC_T_BYTE                   byESCType;                      /*< out ESC Node Type */
    EC_T_WORD                   wReserved3;                     /*< reserved */

    EC_T_WORD                   wAlStatusValue;     /* 0x72 */  /*< out AL Status Register Value */
    EC_T_WORD                   wAlStatusCode;                  /*< out AL Status Code */

    EC_T_BOOL                   bIsOptional;        /* 0x73 */  /*< out slave is in an optional hot connect group */
    EC_T_BOOL                   bIsPresent;         /* 0x74 */  /*< out slave is currently present on bus */


    EC_T_DWORD                  dwReserved[0x1A];   /* 0x75 */  /*< out Reserved*/
};
                                                   /* Size = 0x90 * DWORD */

#endif
#endif /* ETHERCAT_H_ */
