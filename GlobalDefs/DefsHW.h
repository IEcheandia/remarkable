/*
 * DefsHW.h
 *
 *  Created on: 17.03.2010
 *      Author: f.agrawal
 */

#ifndef DEFSHW_H_
#define DEFSHW_H_


//***********************
//** BEGIN Proxy Channels
//***********************
#define	IN_8_BIT_			"IN_8_BIT_"
#define	OUT_8_BIT_			"OUT_8_BIT_"

#define	IN_16_BIT_			"IN_16_BIT_"
#define	OUT_16_BIT_			"OUT_16_BIT_"

#define	OUT_MASKED_8_BIT_	"OUT_MASKED_8_BIT_"
#define	OUT_MASKED_16_BIT_	"IN_MASKED_16_BIT_"

#define	IN_CHAN1_AD_		"IN_CHAN1_AD_"
#define	OUT_CHAN1_DA_		"OUT_CHAN1_DA_"

#define	IN_CHAN2_AD_		"IN_CHAN2_AD_"
#define	OUT_CHAN2_DA_		"OUT_CHAN2_DA_"

#define	IN_CHAN1_AD_IN_OVERSAMPLING		"IN_CHAN1_AD_OVERSAMPLING"
#define	IN_CHAN2_AD_IN_OVERSAMPLING		"IN_CHAN2_AD_OVERSAMPLING"

#define	IN_CHAN1_ENCODER_	"IN_CHAN1_ENCODER_"
#define	IN_CHAN2_ENCODER_	"IN_CHAN2_ENCODER_"
#define	OUT_CHAN1_ENCODER_	"OUT_CHAN1_ENCODER_"
#define	OUT_CHAN2_ENCODER_	"OUT_CHAN2_ENCODER_"

#define	IN_MOTION_			"IN_MOTION_"
#define	OUT_MOTION_			"OUT_MOTION_"

#define OUT_MASTER_			"OUT_MASTER_"
#define IN_MASTER_			"IN_MASTER_"

#define IN_SERVICE_			"IN_SERVICE_"
#define OUT_SERVICE_		"OUT_SERVICE_"

#define	IN_GATEWAY_			"IN_GATEWAY_"
#define	OUT_GATEWAY_		"OUT_GATEWAY_"

#define SERVICE_VENDORID	1
#define SERVICE_PRODUCTCODE	1

#define NAME_REG_IN_		"NAME_REG_IN_"
#define NAME_WRITE_			"NAME_WRITE_"
#define NAME_RECEIVE_		"NAME_RECEIVE_"



////*** BEGIN MOTION
//
//// SEND OUT CHANNEL
//#define NAME_WRITE_OUT_PROXY_MOTION	"Proxy_WriteOut_Motion"
//// SEND REGISTER CHANNEL
//#define NAME_REG_IN_PROXY_MOTION	"REG_IN_Proxy_Motion"
////*** END MOTION
//
//
////*** BEGIN 8BitDigIn_EL1018
//// SEND REGISTER CHANNEL
//#define NAME_REG_IN_PROXY_EL1018	"REG_IN_Proxy_EL1018"
////*** END BEGIN 8BitDigIn_EL1018
//
//
////*** BEGIN 8BitDigOUT_EL2008
//// SEND REGISTER CHANNEL
//#define NAME_WRITE_OUT_PROXY_EL2008	"Proxy_WriteOut_EL2008"
//#define NAME_WRITE_OUT_MASKED_BYTE_PROXY_EL2008	"Proxy_WriteOut_MaskedByte_EL2008"
////*** END BEGIN 8BitDigIn_EL2008
//
//
////*** BEGIN AD2Channels_EL3162
//// SEND REGISTER CHANNEL
//#define NAME_REG_IN_CHAN1_PROXY_EL3162	"REG_IN_Proxy_Chan1_EL3168"
//// SEND REGISTER CHANNEL
//#define NAME_REG_IN_CHAN2_PROXY_EL3162	"REG_IN_Proxy_Chan2_EL3162"
////*** END BEGIN AD2Channels_EL3162
//
//
////*** BEGIN MasterProxy
//// Receive Out
//#define NAME_WRITE_OUT_MasterProxy	"Proxy_WriteOut_MasterProxy"
//// SEND REGISTER CHANNEL (MasterProxy)
//#define NAME_REG_IN_MASTER_PROXY	"REG_IN_Proxy_MasterProxy"
////*** END MasterProxy
//
//
//
//
////***********************
////** END Proxy Channels
////***********************








//***********************
//** BEGIN VI-Service Channels
//***********************


//*** BEGIN MOTION
// SEND REGISTER CHANNEL
#define NAME_REG_IN_VI_SERVICE_MOTION	"REG_IN_VI-Service_Motion"
// SEND OUT CHANNEL
#define NAME_SEND_OUT_VI_SERVICE_MOTION	"VI_SERVICE_RecvOut_Motion"
//RECEIVE OUT
#define NAME_RECEIVE_OUT_VI_SERVICE_MOTION	"VI_SERVICE_RecvOut_Motion"
//RECEIVE IN
#define NAME_RECEIVE_IN_VI_SERVICE_MOTION	"VI_SERVICE_Recv_Motion"
//*** END MOTION


//*** BEGIN 8BitDigIn_EL1018
// RECEIVE IN
#define NAME_RECEIVE_IN_VI_SERVICE_EL1018	"VI_SERVICE_Recv_EL1018"
// SEND REGISTER CHANNEL
#define NAME_REG_IN_VI_SERVICE_EL1018	"REG_IN_VI-Service_EL1018"
//*** END 8BitDigIn_EL1018


//*** BEGIN 8BitDigOut_EL2008
//RECEIVE OUT
#define NAME_RECEIVE_OUT_VI_SERVICE_EL2008	"VI_SERVICE_RecvOut_EL2008"
//*** END 8BitDigOut_EL2008


//*** BEGIN AD2Channels_EL3162
// RECEIVE IN1
#define NAME_RECEIVE_IN_VI_SERVICE_CHAN1_EL3162	"VI_SERVICE_Recv_Chan1_EL3162"
// RECEIVE IN2
#define NAME_RECEIVE_IN_VI_SERVICE_CHAN2_EL3162	"VI_SERVICE_Recv_Chan2_EL3162"

// SEND REGISTER CHANNEL1
#define NAME_REG_IN_VI_SERVICE_CHAN1_EL3162	"REG_IN_VI-Service_Chan1_EL3162"
// SEND REGISTER CHANNEL2
#define NAME_REG_IN_VI_SERVICE_CHAN2_EL3162	"REG_IN_VI-Service_Chan2_EL3162"
//*** END AD2Channels_EL3162



//*** BEGIN MasterHandler
// RECEIVE IN
#define NAME_RECEIVE_IN_VI_SERVICE_MASTERHANDLER	"VI_SERVICE_Recv_MasterHandler"


//*** END MasterHandler

//***********************
//** END VI-Service Channels
//***********************





//***********************
//** BEGIN VI-WeldHeadControl
//***********************


//*** BEGIN MOTION
// SEND REGISTER CHANNEL
#define NAME_REG_IN_VI_WELDHEADCONTROL_MOTION	"REG_IN_VI-WeldHeadControl_Motion"
// SEND OUT CHANNEL
#define NAME_SEND_OUT_VI_WELDHEADCONTROL_MOTION	"VI_WeldHeadControl_RecvOut_Motion"

#define NAME_RECEIVE_IN_VI_WELDHEADCONTROL_MOTION	"VI_WeldHeadControl_Recv_Motion"
//*** END MOTION



//***********************
//** END VI-WeldHeadControl
//***********************

#endif /* DEFSHW_H_ */
