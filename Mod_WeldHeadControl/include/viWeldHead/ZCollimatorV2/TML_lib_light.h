/*
 * TML_lib_light.h
 *
 *  Copyright (c) 2018, TECHNOSOFT
 *  All rights reserved.
 *  Created on: Apr 9, 2015
 *  Author: g_blujdea
 *	Version: 1.7
 *
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *        * Redistributions of source code must retain the above copyright
 *          notice, this list of conditions and the following disclaimer.
 *        * Redistributions in binary form must reproduce the above copyright
 *          notice, this list of conditions and the following disclaimer in the
 *          documentation and/or other materials provided with the distribution.
 *        * Neither the name of the TECHNOSOFT nor the
 *          names of its contributors may be used to endorse or promote products
 *          derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *    DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef INCLUDE_TML_LIB_LIGHT_H_
#define INCLUDE_TML_LIB_LIGHT_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define RS232_COM	// comment the definition for TMLCAN communication

#undef LOBYTE
#define LOBYTE(x)    ((uint8_t)(x))
#undef HIBYTE
#define HIBYTE(x)    ((uint8_t)((uint16_t)(x) >> 8))

#undef LOWORD
#define LOWORD(x)    ((uint16_t)(x))
#undef HIWORD
#define HIWORD(x)    ((uint16_t)((uint32_t)(x) >> 16))

#define FIXED(x)   (uint32_t)(x * 65536.0 + 0.5)

#define MAX_ERROR 512

#define CHECKSUM_DELAY 500

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
	uint16_t opCode;
	uint8_t length;
	uint16_t TML_data[4];
}TML_INSTR;

typedef struct {
	uint32_t identifier;
	uint8_t length;
	uint8_t CAN_data[8];
}CAN_MSG;

typedef struct {
	uint8_t length;
	uint8_t RS232_data[12];
}RS232_MSG;

#define RS232_HEADER	6 // RS232 message header length: axis ID 2 bytes + operation code 2 bytes + checksum 1 byte + message length 1 byte
#define HOST_BIT		0x0001

/*CAN node ID of the PC*/
#define MASTER_ID	120

/*Constants used for the register for function TS_ReadStatus*/
#define REG_SRL		0
#define REG_SRH		1
#define REG_MER		2
#define REG_DER		3
#define REG_DER2	4

/*Constants for registers addresses*/
#define SRL_ADDR 	0x090E
#define SRH_ADDR	0x090F
#define SR_ADDR		0x090E
#define MER_ADDR	0x08FC
#define DER_ADDR	0x03AD
#define DER2_ADDR	0x0305
#define MCR_ADDR	0x0309
#define MCR_1_ADDR	0x02B0
#define UPGRADE_ADDR	0x0857
#define ACR_ADDR	0x0912

#define CPOS_ADDR	0x029E
#define CSPD_ADDR	0x02A0
#define CACC_ADDR	0x02A2
#define TJERK_ADDR	0x08D2

#define APOS_ADDR	0x0228

#define INSTATUS_ADDR	0x0908

#define FUNCTION_TABLE_POINTER 0x09C9

/*Constants used to select or set the group*/
#define GROUP_0		0
#define GROUP_1		1
#define GROUP_2		2
#define GROUP_3		3
#define GROUP_4		4
#define GROUP_5		5
#define GROUP_6		6
#define GROUP_7		7
#define GROUP_8		8

/*Constants used as values for 'MoveMoment' parameters*/
#define UPDATE_NONE			-1
#define UPDATE_ON_EVENT		0
#define UPDATE_IMMEDIATE	1

/*Constants used for 'ReferenceType' parameters*/
#define REFERENCE_POSITION		0
#define REFERENCE_SPEED			1
#define REFERENCE_TORQUE		2
#define REFERENCE_VOLTAGE		3

/*Constants used for ReferenceBase*/
#define FROM_MEASURE	0
#define FROM_REFERENCE	1

/*Constants used for DecelerationType*/
#define S_CURVE_SPEED_PROFILE		0
#define TRAPEZOIDAL_SPEED_PROFILE	1


/***** PROTOTYPES *******************/
/*Translate TML instructions to TMLCAN messages*/
bool TS_TML_to_TMLCAN(uint16_t AxisID, TML_INSTR *TML_instruction, CAN_MSG *CAN_message);

/*Translate TML instructions to RS232 messages*/
bool TS_TML_to_RS232(uint16_t AxisID, TML_INSTR *TML_instruction, RS232_MSG *RS232_message);

/*The function reads a status register from the drive*/
bool TS_ReadStatus(uint16_t AxisID, uint16_t Register, uint16_t *RegisterValue);

/*The function sends the ENDINIT instruction*/
bool TS_InitializeDrive(uint16_t AxisID);

/*The function reads MER.2 to determine if the setup table is valid or not*/
bool TS_CheckSetupTable(uint16_t AxisID, bool *SetupTableStatus);

/*The function enables/disables the power stage of the drive */
bool TS_Power(uint16_t AxisID, bool PowerSwitch);

/*The function reads the functions table generated from EasyMotion Studio and stored in the non-volatile memory of the drive */
bool TS_ReadFunctionsTable(uint16_t AxisID, uint16_t* FunctionsAddresses, uint8_t* FunctionNo);

/*The function triggers the execution of the TML function stored on the drive */
bool TS_StartFunction(uint16_t AxisID, uint16_t FunctionAddress);

bool TS_StartFunctionByNumber(uint16_t AxisID, uint16_t FunctionNumber);

/*The function cancels the execution of the current TML function */
bool TS_AbortFunction(uint16_t AxisID);

/*The function sends the TML command to the drive */
bool TS_ExecuteTML(uint16_t AxisID, uint16_t OpCode, uint16_t TMLData1, uint16_t TMLData2, uint16_t TMLData3, uint16_t TMLData4, uint8_t NOWords);

/*The function set/reset the output of the drive */
bool TS_SetOutput(uint16_t AxisID, uint8_t nIO, uint8_t OutValue);

/*The function reads the input state from the drive */
bool TS_ReadInput(uint16_t AxisID, uint8_t nIO, bool* InValue);

/*The function downloads the sofware file to the drive */
bool TS_WriteSoftwareFile(uint16_t AxisID, const char *pFileConfig);

/*The function checks if the content of the software file matches the content of the non-volatile memory of the drive */
bool TS_VerifySoftwareFile(uint16_t AxisID, const char *pFileConfig);

int ReadSwFileLine(FILE* pFileConfig, char* strline);

/*The function disables the TML program stored in the non-volatile memory of the drive */
bool TS_DisableTMLProgram(uint16_t AxisID);

/*The function resets the select drive */
bool TS_Reset(uint16_t AxisID);

/*The function resets FAULT state on the selected drive */
bool TS_ResetFault(uint16_t AxisID);

/*The function sends the SAVE command to the selected drive - the drive will save the configuration to the EEPROM - timeout!! */
bool TS_Save(uint16_t AxisID);

/*The function starts the motion on the selected drive */
bool TS_UpdateImmediate(uint16_t AxisID);

/*The function configures the drive to start the motion when an event is detected */
bool TS_UpdateOnEvent(uint16_t AxisID);

bool TS_Stop(uint16_t AxisID);

/*The function sets the actual position of the motor on the selected drive */
bool TS_SetPosition(uint16_t AxisID, int32_t PositionValue);

bool TS_QuickStopDecelerationRate(uint16_t AxisID, double Deceleration);

/*The function sets the actual position of the motor on the selected drive */
bool TS_SetCurrent(uint16_t AxisID, int16_t CurrentValue);

/*The function sets the target position value equal to the actual position value */
bool TS_SetTargetPositionToActual(uint16_t AxisID);

/*The function  sets a trapezoidal position profile - the position is absolute*/
bool TS_MoveAbsolute(uint16_t AxisID, int32_t AbsPosition, double Speed, double Acceleration, int16_t MoveMoment, int16_t ReferenceBase);

/*The function  sets a trapezoidal position profile - the position command is an increment from the current position*/
bool TS_MoveRelative(uint16_t AxisID, int32_t RelPosition, double Speed, double Acceleration, bool IsAdditive, int16_t MoveMoment, int16_t ReferenceBase);

/*The function  sets a position profile with S shape for speed - the position command is an absolute*/
bool TS_MoveSCurveAbsolute(uint16_t AxisID, int32_t RelPosition, double Speed, double Acceleration, int32_t JerkTime, int16_t MoveMoment, int16_t DecelerationType);

/*The function  sets a position profile with S shape for speed - the position command is an increment from the current position*/
bool TS_MoveSCurveRelative(uint16_t AxisID, int32_t RelPosition, double Speed, double Acceleration, int32_t JerkTime, int16_t MoveMoment, int16_t DecelerationType);

/*The function sets a speed profile */
bool TS_MoveVelocity(uint16_t AxisID, double Speed, double Acceleration, int16_t MoveMoment, int16_t ReferenceBase);

/*The function configures the drive to use the external analog signal as reference signal */
bool TS_SetAnalogueMoveExternal (uint16_t AxisID, uint16_t ReferenceType, bool UpdateFast, double LimitVariation, int16_t MoveMoment);

bool TS_SendSynchronization (uint16_t AxisID, uint32_t Period);

/*The function enables the drive to send data automatically to the host/master device */
bool TS_SendDataToHost(uint16_t AxisID, uint16_t HostAddress, uint32_t StatusRegMask, uint16_t ErrorRegMask);

/*The function requests the checksum value between the startAddress and endAddress */
bool TS_OnlineChecksum(uint16_t AxisID, uint16_t startAddress, uint16_t endAddress, uint16_t* checksum);

bool TS_Write16bitValue(uint16_t AxisID, uint16_t MemoryAddress, int16_t writeValue);

bool TS_Write32bitValue(uint16_t AxisID, uint16_t MemoryAddress, int32_t writeValue);

bool TS_Read16bitValue(uint16_t AxisID, uint16_t MemoryAddress, int16_t* readValue);

bool TS_Read32bitValue(uint16_t AxisID, uint16_t MemoryAddress, int32_t* readValue);

void sleep_ms(uint16_t miliseconds);

#ifndef RS232_COM
	extern bool SendMessage(CAN_MSG * CAN_TX_message);
	extern bool ReceiveMessage(CAN_MSG * CAN_RX_message);
#else
	extern bool SendMessage(RS232_MSG * RS232_TX_message);
	extern bool ReceiveMessage(RS232_MSG * RS232_RX_message);
#endif

#ifdef __cplusplus
}
#endif
    
#endif /* INCLUDE_TML_LIB_H_ */
