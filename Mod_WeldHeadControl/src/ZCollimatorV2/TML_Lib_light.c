/*
 * TML_Lib_light.c
 *
 *  Copyright (c) 2019, TECHNOSOFT
 *  All rights reserved.
 *  Created on: Apr 9, 2015
 *  Author: g_blujdea
 *  Version: 2.1
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

#include "viWeldHead/ZCollimatorV2/TML_lib_light.h"
#include "viWeldHead/ZCollimatorV2/TML_instructions.h"

bool TS_ReadStatus(uint16_t AxisID, uint16_t Register, uint16_t * RegisterValue)
{
	#ifndef RS232_COM
		CAN_MSG CAN_RX_message;
		CAN_MSG CAN_TX_message;
	#else
		RS232_MSG RS232_TX_message;
		RS232_MSG RS232_RX_message;
	#endif

	TML_INSTR TML_instruction;

	TML_instruction.opCode = READ_16_BIT_RAM;
	TML_instruction.length = 2; /*Data request instructions have 2 words of data*/
#ifndef RS232_COM
	TML_instruction.TML_data[0] = MASTER_ID << 4;
#else
    TML_instruction.TML_data[0] = (MASTER_ID << 4) | HOST_BIT;
    RS232_RX_message.length = 10;
#endif
	switch(Register){
		case REG_SRL:
			TML_instruction.TML_data[1] = SRL_ADDR;
			break;
		case REG_SRH:
			TML_instruction.TML_data[1] = SRH_ADDR;
			break;
		case REG_MER:
			TML_instruction.TML_data[1] = MER_ADDR;
			break;
		case REG_DER:
			TML_instruction.TML_data[1] = DER_ADDR;
			break;
		case REG_DER2:
			TML_instruction.TML_data[1] = DER2_ADDR;
			break;
		default:
			return false;
			break;
	}
	
		/*Send the message with the TML instruction - HW dependent*/
#ifndef RS232_COM
		TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
		if (!SendMessage(&CAN_TX_message))
			return false;
#else
		TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
		if (!SendMessage(&RS232_TX_message))
			return false;	
#endif

	/*Wait for drive to reply*/

	/*Decode the received message - HW dependent*/
	#ifndef RS232_COM
		if (!ReceiveMessage(&CAN_RX_message))
			return false;
		*RegisterValue = ((uint16_t)CAN_RX_message.CAN_data[3] << 8) | CAN_RX_message.CAN_data[2];

	#else
		if (!ReceiveMessage(&RS232_RX_message))
			return false;
		*RegisterValue = ((uint16_t)RS232_RX_message.RS232_data[7] << 8) | RS232_RX_message.RS232_data[8];
	#endif

	return true;

}

bool TS_DisableTMLProgram(uint16_t AxisID)
{
	#ifndef RS232_COM
		CAN_MSG CAN_RX_message;
		CAN_MSG CAN_TX_message;
	#else
		RS232_MSG RS232_TX_message;
		RS232_MSG RS232_RX_message;
	#endif

	TML_INSTR TML_instruction;

//Set the pointer variable
	TML_instruction.opCode = WRITE_16_BIT_RAM | (0x01FF & DOWNLOAD_POINTER);
	TML_instruction.length = 1;/*Write instructions has 1 data word*/
	TML_instruction.TML_data[0] = 0x4000; //start address of the TML program

#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	/*Send the message with TML instruction - HW dependent*/
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	/*Send the message with TML instruction - HW dependent*/
	if (!SendMessage(&RS232_TX_message))
		return false;
#endif			

//Erase the first location of the TML program to disabled it
	TML_instruction.opCode = WRITE_16_BIT_EEPROM;
	TML_instruction.length = 2;/*Write instructions has 2 data word*/
	TML_instruction.TML_data[0] = DOWNLOAD_POINTER;
	TML_instruction.TML_data[1] = 1;

#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	/*Send the message with TML instruction - HW dependent*/
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	/*Send the message with TML instruction - HW dependent*/
	if (!SendMessage(&RS232_TX_message))
		return false;
#endif			

	return true;
}


bool TS_WriteSoftwareFile(uint16_t AxisID, const char *pFileConfig)
{
	#ifndef RS232_COM
		CAN_MSG CAN_RX_message;
		CAN_MSG CAN_TX_message;
	#else
		RS232_MSG RS232_TX_message;
		RS232_MSG RS232_RX_message;
	#endif

	TML_INSTR TML_instruction;
	
	char str[10]; //test line from software file
	int isAddress = 1;//flag to know when the string read is address or data
	FILE *fptr;
	int i, j;
	uint16_t w; // hexadecimal value read from software file
	int16_t ReadDownloadAddress;
	int16_t DownloadAddress;
	bool isError = false;

	if ((fptr = fopen(pFileConfig,"r")) == NULL)
	{
		printf("Error opening file %s! \n", pFileConfig);
		return false; //exit if unable to open the file
	}

	while ((i = ReadSwFileLine(fptr, str)) == 0)
	// reads text until newline    
	{
		j = sscanf(str, "%hx", &w);

		if (j == 1) 
		{
			if (isAddress == 0)
			{
				//EEPROM data
				//printf("Data to be sent %hx \n",w);
				TML_instruction.opCode = WRITE_16_BIT_EEPROM_A;
				TML_instruction.length = 2;/*Write instructions has 2 data word*/
				TML_instruction.TML_data[0] = DOWNLOAD_POINTER;
				TML_instruction.TML_data[1] = w;

#ifndef RS232_COM
				TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
				/*Send the message with TML instruction - HW dependent*/
				if (!SendMessage(&CAN_TX_message))
				{
					isError = true;
					break;
				}
#else
				TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
				/*Send the message with TML instruction - HW dependent*/
				if (!SendMessage(&RS232_TX_message))
				{	
					isError = true;
					break;
				}
#endif			
				DownloadAddress ++; //address for next write operation

				//Wait for the EEPROM write to finish. 
				//The drive will reply to data request when ready for new EEPROM write
				if (!TS_Read16bitValue(AxisID, DOWNLOAD_POINTER, &ReadDownloadAddress))
				{
					isError = true;
					break;
				}

				if (ReadDownloadAddress != DownloadAddress)
				{
					// address returned by the drive doesn't match the download address
					isError = true;
					break;
				}
			}
			else
			{
				//EEPROM address
				//printf("Address to send data %hx \n",w);
				//Set the pointer variable
				TML_instruction.opCode = WRITE_16_BIT_RAM | (0x01FF & DOWNLOAD_POINTER);
				TML_instruction.length = 1;/*Write instructions has 1 data words*/
				TML_instruction.TML_data[0] = w;

				DownloadAddress = w;

#ifndef RS232_COM
				TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
				/*Send the message with TML instruction - HW dependent*/
				if (!SendMessage(&CAN_TX_message))
				{
					isError = true;
					break;
				}
#else
				TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
				/*Send the message with TML instruction - HW dependent*/
				if (!SendMessage(&RS232_TX_message))
				{
					isError = true;
					break;
				}
#endif			

				isAddress = 0;
			}
		}	
		else
		{
				isAddress = 1; //next read will be the address of the next data block
		}

	}
	
	fclose(fptr);

	if (isError)
	{
		return false;
	}
	else
	{
		return true;
	}


}

bool TS_VerifySoftwareFile(uint16_t AxisID, const char *pFileConfig)
{
	#ifndef RS232_COM
		CAN_MSG CAN_RX_message;
		CAN_MSG CAN_TX_message;
	#else
		RS232_MSG RS232_TX_message;
		RS232_MSG RS232_RX_message;
	#endif

	TML_INSTR TML_instruction;
	
	char str[10]; //test line from software file
	int16_t isAddress = 1;//flag to know when the string read is address or data
	FILE *fptr;
	uint16_t i, j;
	uint16_t start_address, end_address = 0;
	uint16_t w; // hexadecimal value read from software file
	int16_t ChecksumDrive, ChecksumSW = 0;
	bool isError = false;

	if ((fptr = fopen(pFileConfig,"r")) == NULL)
	{
		printf("Error opening file %s! \n", pFileConfig);
		return false; //exit if unable to open the file
	}

	while ((i = ReadSwFileLine(fptr, str)) == 0)
	// reads text until newline    
	{
		j = sscanf(str, "%hx", &w);

		if (j == 1) 
		{
			if (isAddress == 0)
			{
				//EEPROM data to compute checksum
				end_address ++;
				ChecksumSW = ChecksumSW + w;
			}
			else
			{
				//EEPROM address
				ChecksumSW = 0;
                end_address = w - 1;
				start_address = w;
				isAddress = 0;
			}
		}	
		else
		{
			//Send the TML instruction CHECKSUM, SPI 
			TML_instruction.opCode = CHECKSUM_SPI;
			TML_instruction.length = 3;/*Write instructions has 3 data words*/
			TML_instruction.TML_data[0] = DOWNLOAD_POINTER;/*use DOWNLOAD_POINTER to store the checksum*/;
			TML_instruction.TML_data[1] = start_address;
			TML_instruction.TML_data[2] = end_address;

		#ifndef RS232_COM
			TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
			/*Send the message with TML instruction - HW dependent*/
			if (!SendMessage(&CAN_TX_message))
			{
				isError = true;
				break;
			}		
		#else
			TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
			/*Send the message with TML instruction - HW dependent*/
            if (!SendMessage(&RS232_TX_message))
            {
                isError = true;
                break;
            }
		#endif			
            sleep_ms(CHECKSUM_DELAY);
			//Request download pointer variable with the checksum
            if (!TS_Read16bitValue(AxisID, DOWNLOAD_POINTER, &ChecksumDrive))
            {
                isError = true;
                break;
            }

			isAddress = 1; //next read from SW file will be the address of the next data block

            if (ChecksumDrive != ChecksumSW)
            {
                printf("Checksum software file doesn't match checksum drive");
                isError = true;
                break;
            }
		}

	}
	
	fclose(fptr);
	
	if (isError)
	{
		return false;
	}
	else
	{
		return true;		
	}

}


bool TS_InitializeDrive(uint16_t AxisID)
{
#ifndef RS232_COM
		CAN_MSG CAN_TX_message;
	#else
		RS232_MSG RS232_TX_message;
	#endif

	TML_INSTR TML_instruction;

	TML_instruction.opCode = ENDINIT_OP_CODE;
	TML_instruction.length = 0; /*ENDINIT has no data words*/
		
		/*Send the message with the TML instruction - HW dependent*/
	#ifndef RS232_COM
		TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
		if (!SendMessage(&CAN_TX_message))
			return false;
	#else
		TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);		
		if (!SendMessage(&RS232_TX_message))	
			return false;
	#endif	

	return true;
}

bool TS_CheckSetupTable(uint16_t AxisID, bool *SetupTableStatus)
{
	#ifndef RS232_COM
		CAN_MSG CAN_RX_message;
		CAN_MSG CAN_TX_message;
	#else
		RS232_MSG RS232_TX_message;
		RS232_MSG RS232_RX_message;
	#endif

	TML_INSTR TML_instruction;

	TML_instruction.opCode = READ_16_BIT_RAM;
	TML_instruction.length = 2; /*Data request instructions has 2 data words*/
#ifndef RS232_COM
	TML_instruction.TML_data[0] = MASTER_ID << 4;
#else
    TML_instruction.TML_data[0] = (MASTER_ID << 4) | HOST_BIT;
#endif
	TML_instruction.TML_data[1] = MER_ADDR;
	
	/*Send the message with the TML instruction - HW dependent*/
	#ifndef RS232_COM
		TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
		if (!SendMessage(&CAN_TX_message))
			return false;
	#else
		TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
		if (!SendMessage(&RS232_TX_message))	
			return false;
	#endif

	/*Wait for drive to reply*/
	
	/*Decode the received message - HW dependent*/
	#ifndef RS232_COM
		if (!ReceiveMessage(&CAN_RX_message))
			return false;
		*SetupTableStatus = !((((uint16_t)CAN_RX_message.CAN_data[3] << 8) | CAN_RX_message.CAN_data[2]) & 0x0004);
	#else
		if (!ReceiveMessage(&RS232_RX_message))
			return false;		
		*SetupTableStatus = (bool)!((((uint16_t)RS232_RX_message.RS232_data[7] << 8) | RS232_RX_message.RS232_data[8]) & 0x0004);
	#endif
	return true;
}

bool TS_Power(uint16_t AxisID, bool PowerSwitch)
{
#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif

	TML_INSTR TML_instruction;

	TML_instruction.opCode = (PowerSwitch == false) ? AXISOFF_OP_CODE : AXISON_OP_CODE;
	TML_instruction.length = 0; /*AXISON/AXISOFF has no data word*/
	
	/*Send the message with the TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	return true;
}

bool TS_Reset(uint16_t AxisID)
{

#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif

	TML_INSTR TML_instruction;

	TML_instruction.opCode = RESET_AXIS;
	TML_instruction.length = 0; /*RESET has no data word*/

	/*Send the message with the TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	return true;
}

bool TS_ResetFault(uint16_t AxisID)
{

#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif

	TML_INSTR TML_instruction;

	TML_instruction.opCode = RESET_FAULT;
	TML_instruction.length = 0; /*FAULTR has no data word*/

	/*Send the message with the TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	return true;
}

bool TS_Save(uint16_t AxisID)
{

#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif

	TML_INSTR TML_instruction;

	TML_instruction.opCode = SAVE_OP_CODE;
	TML_instruction.length = 0; /*SAVE has no data word*/

	/*Send the message with the TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	return true;
}

bool TS_UpdateImmediate(uint16_t AxisID)
{

#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif

	TML_INSTR TML_instruction;

	TML_instruction.opCode = UPD_OP_CODE;
	TML_instruction.length = 0; /*UPD has no data word*/

	/*Send the message with the TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	return true;
}

bool TS_UpdateOnEvent(uint16_t AxisID)
{
	/*Send !UPD */
#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif

	TML_INSTR TML_instruction;

	TML_instruction.opCode = UPD_EVENT_OP_CODE;
	TML_instruction.length = 0; /*!UPD has no data word*/

	/*Send the message with the TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	return true;
}

bool TS_Stop(uint16_t AxisID)
{
	/*Send STOP */
#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif

	TML_INSTR TML_instruction;

	TML_instruction.opCode = STOP_OP_CODE;
	TML_instruction.length = 0; /*STOP has no data word*/

	/*Send the message with the TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	return true;
}

bool TS_SetPosition(uint16_t AxisID, int32_t PositionValue)
{
#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif

	TML_INSTR TML_instruction;

	TML_instruction.opCode = SAP_OP_CODE;
	TML_instruction.length = 2; /*SAP has 2 data word*/
	TML_instruction.TML_data[0] = LOWORD(PositionValue);
	TML_instruction.TML_data[1] = HIWORD(PositionValue);

	/*Send the message with TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	return true;
}

bool TS_QuickStopDecelerationRate(uint16_t AxisID, double Deceleration)
{
	uint16_t MemoryAddress = 0x0858; /*address of CDEC variable */

    return TS_Write32bitValue(AxisID, MemoryAddress, FIXED(Deceleration));
}

bool TS_SetCurrent(uint16_t AxisID, int16_t CurrentValue)
{
	uint16_t MemoryAddress = 0x027b; /*address of IDRSTEP variable */

	return TS_Write16bitValue(AxisID, MemoryAddress, CurrentValue);
}

bool TS_SetTargetPositionToActual(uint16_t AxisID)
{
#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif

	TML_INSTR TML_instruction;

	TML_instruction.opCode = STA_OP_CODE;
	TML_instruction.length = 1; /*STA has 1 data word*/
	TML_instruction.TML_data[0] = STA_DATA;

	/*Send the message with TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	return true;
}

bool TS_MoveAbsolute(uint16_t AxisID, int32_t AbsPosition, double Speed, double Acceleration, int16_t MoveMoment, int16_t ReferenceBase)
{
	if (!TS_Write32bitValue(AxisID, CPOS_ADDR, AbsPosition))
		return false;	

	if (!TS_Write32bitValue(AxisID, CSPD_ADDR, FIXED(Speed)))
		return false;

	if (!TS_Write32bitValue(AxisID, CACC_ADDR, FIXED(Acceleration)))
		return false;

	/*Set motion mode: position profile */
	if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(MODE_PP_DATA),LOWORD(MODE_PP_DATA), NO_TML_DATA, NO_TML_DATA, 2))
		return false;

	/*Set profile type: absolute */
	if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(CPA_DATA),LOWORD(CPA_DATA), NO_TML_DATA, NO_TML_DATA, 2))
		return false;

	if(ReferenceBase == FROM_REFERENCE)
		if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(TUM1_DATA),LOWORD(TUM1_DATA), NO_TML_DATA, NO_TML_DATA, 2))
			return false;
	
	switch(MoveMoment)
	{
	case UPDATE_ON_EVENT:
		if (!TS_UpdateOnEvent(AxisID))
			return false;
		break;
	case UPDATE_IMMEDIATE:
		if (!TS_UpdateImmediate(AxisID))
			return false;
		break;
	}

	return true;
}

bool TS_MoveRelative(uint16_t AxisID, int32_t RelPosition, double Speed, double Acceleration, bool IsAdditive, int16_t MoveMoment, int16_t ReferenceBase)
{
	if (!TS_Write32bitValue(AxisID, CPOS_ADDR, RelPosition))
		return false;	

	if (!TS_Write32bitValue(AxisID, CSPD_ADDR, FIXED(Speed)))
		return false;

	if (!TS_Write32bitValue(AxisID, CACC_ADDR, FIXED(Acceleration)))
		return false;

	/*Set motion mode: position profile */
	if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(MODE_PP_DATA),LOWORD(MODE_PP_DATA), NO_TML_DATA, NO_TML_DATA, 2))
		return false;

	/*Set profile type: relative */
	if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(CPR_DATA),LOWORD(CPR_DATA), NO_TML_DATA, NO_TML_DATA, 2))
		return false;

	if(IsAdditive)
	{
		/*SRB ACR, 0xFFFF, 0x0800;*/
		if (!TS_ExecuteTML(AxisID, SRBL_OP_CODE, ACR_ADDR, 0xFFFF, 0x0800, NO_TML_DATA, 3))
			return false;
	}

	if(ReferenceBase == FROM_REFERENCE)
		if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(TUM1_DATA),LOWORD(TUM1_DATA), NO_TML_DATA, NO_TML_DATA, 2))
			return false;
	
	switch(MoveMoment)
	{
	case UPDATE_ON_EVENT:
		if (!TS_UpdateOnEvent(AxisID))
			return false;
		break;
	case UPDATE_IMMEDIATE:
		if (!TS_UpdateImmediate(AxisID))
			return false;
		break;
	default:
		return false;
	}

	return true;
}

bool TS_MoveSCurveAbsolute(uint16_t AxisID, int32_t AbsPosition, double Speed, double Acceleration, int32_t JerkTime, int16_t MoveMoment, int16_t DecelerationType)
{

	if (!TS_Write32bitValue(AxisID, CPOS_ADDR, AbsPosition))
		return false;	

	if (!TS_Write32bitValue(AxisID, CSPD_ADDR, FIXED(Speed)))
		return false;

	if (!TS_Write32bitValue(AxisID, CACC_ADDR, FIXED(Acceleration)))
		return false;

	if (!TS_Write32bitValue(AxisID, TJERK_ADDR, JerkTime))
		return false;

	/*Set motion mode: position profile */
	if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(MODE_PSC_DATA),LOWORD(MODE_PSC_DATA), NO_TML_DATA, NO_TML_DATA, 2))
		return false;

	/*Set profile type: absolute */
	if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(CPA_DATA),LOWORD(CPA_DATA), NO_TML_DATA, NO_TML_DATA, 2))
		return false;

	switch(DecelerationType)
	{
	case S_CURVE_SPEED_PROFILE:
		/*SRB ACR, 0xFFFE, 0x0000;*/
		if (!TS_ExecuteTML(AxisID, SRBL_OP_CODE, ACR_ADDR, 0xFFFE, 0x0000, NO_TML_DATA, 3))
			return false;
		break;
	case TRAPEZOIDAL_SPEED_PROFILE:
		/*"SRB ACR, 0xFFFF, 0x0001;";*/
		if (!TS_ExecuteTML(AxisID, SRBL_OP_CODE, ACR_ADDR, 0xFFFF, 0x0001, NO_TML_DATA, 3))
			return false;
		break;
	default:
		return false;
	}

	switch(MoveMoment)
	{
	case UPDATE_ON_EVENT:
		if (!TS_UpdateOnEvent(AxisID))
			return false;
		break;
	case UPDATE_IMMEDIATE:
		if (!TS_UpdateImmediate(AxisID))
			return false;
		break;
	default:
		return false;
	}

	return true;
}

bool TS_MoveSCurveRelative(uint16_t AxisID, int32_t RelPosition, double Speed, double Acceleration, int32_t JerkTime, int16_t MoveMoment, int16_t DecelerationType)
{

	if (!TS_Write32bitValue(AxisID, CPOS_ADDR, RelPosition))
		return false;	

	if (!TS_Write32bitValue(AxisID, CSPD_ADDR, FIXED(Speed)))
		return false;

	if (!TS_Write32bitValue(AxisID, CACC_ADDR, FIXED(Acceleration)))
		return false;

	if (!TS_Write32bitValue(AxisID, TJERK_ADDR, JerkTime))
		return false;

	/*Set motion mode: position profile */
	if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(MODE_PSC_DATA),LOWORD(MODE_PSC_DATA), NO_TML_DATA, NO_TML_DATA, 2))
		return false;

	/*Set profile type: relative */
	if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(CPR_DATA),LOWORD(CPR_DATA), NO_TML_DATA, NO_TML_DATA, 2))
		return false;

	switch(DecelerationType)
	{
	case S_CURVE_SPEED_PROFILE:
		/*SRB ACR, 0xFFFE, 0x0000;*/
		if (!TS_ExecuteTML(AxisID, SRBL_OP_CODE, ACR_ADDR, 0xFFFE, 0x0000, NO_TML_DATA, 3))
			return false;
		break;
	case TRAPEZOIDAL_SPEED_PROFILE:
		/*"SRB ACR, 0xFFFF, 0x0001;";*/
		if (!TS_ExecuteTML(AxisID, SRBL_OP_CODE, ACR_ADDR, 0xFFFF, 0x0001, NO_TML_DATA, 3))
			return false;
		break;
	default:
		return false;
	}

	switch(MoveMoment)
	{
	case UPDATE_ON_EVENT:
		if (!TS_UpdateOnEvent(AxisID))
			return false;
		break;
	case UPDATE_IMMEDIATE:
		if (!TS_UpdateImmediate(AxisID))
			return false;
		break;
	default:
		return false;
	}

	return true;
}

bool TS_MoveVelocity(uint16_t AxisID, double Speed, double Acceleration, int16_t MoveMoment, int16_t ReferenceBase)
{

	if (!TS_Write32bitValue(AxisID, CSPD_ADDR, FIXED(Speed)))
		return false;

	if (!TS_Write32bitValue(AxisID, CACC_ADDR, FIXED(Acceleration)))
		return false;

	/*Set motion mode: speed profile */
	if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(MODE_SP_DATA),LOWORD(MODE_SP_DATA), NO_TML_DATA, NO_TML_DATA, 2))
		return false;

	if(ReferenceBase == FROM_REFERENCE)
		if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(TUM1_DATA),LOWORD(TUM1_DATA), NO_TML_DATA, NO_TML_DATA, 2))
			return false;
	
	switch(MoveMoment)
	{
	case UPDATE_ON_EVENT:
		if (!TS_UpdateOnEvent(AxisID))		
			return false;
		break;
	case UPDATE_IMMEDIATE:
		if (!TS_UpdateImmediate(AxisID))
			return false;
		break;
	default:
		return false;
	}

	return true;
}

bool TS_SetAnalogueMoveExternal (uint16_t AxisID, uint16_t ReferenceType, bool UpdateFast, double LimitVariation, int16_t MoveMoment)
{

	if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(EXTREF_1_DATA),LOWORD(EXTREF_1_DATA), NO_TML_DATA, NO_TML_DATA, 2))
		return false;

	switch(ReferenceType)
	{
	case REFERENCE_POSITION:
		if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(MODE_PE_DATA),LOWORD(MODE_PE_DATA), NO_TML_DATA, NO_TML_DATA, 2))
			return false;
		if(LimitVariation > 0)
		{
			if (!TS_Write32bitValue(AxisID, CSPD_ADDR, FIXED(LimitVariation)))
				return false;
			/*SRB UPGRADE, 0xFFFF, 0x0004;*/
			if (!TS_ExecuteTML(AxisID, SRBL_OP_CODE, UPGRADE_ADDR, 0xFFFF, 0x0004, NO_TML_DATA, 3))
				return false;
		}
		else 
		{
			/*SRB UPGRADE, 0xFFFB, 0x0000;*/
			if (!TS_ExecuteTML(AxisID, SRBL_OP_CODE, UPGRADE_ADDR, 0xFFFB, 0x0000, NO_TML_DATA, 3))
				return false;
		}
		break;

	case REFERENCE_SPEED:
		if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(MODE_SE_DATA),LOWORD(MODE_SE_DATA), NO_TML_DATA, NO_TML_DATA, 2))
			return false;
		if(LimitVariation > 0)
		{
			if (!TS_Write32bitValue(AxisID, CACC_ADDR, FIXED(LimitVariation)))
				return false;
			/*SRB UPGRADE, 0xFFFF, 0x0004;*/
			if (!TS_ExecuteTML(AxisID, SRBL_OP_CODE, UPGRADE_ADDR, 0xFFFF, 0x0004, NO_TML_DATA, 3))
				return false;
		} 
		else 
			/*SRB UPGRADE, 0xFFFB, 0x0000;*/
			if (!TS_ExecuteTML(AxisID, SRBL_OP_CODE, UPGRADE_ADDR, 0xFFFB, 0x0000, NO_TML_DATA, 3))
				return false;
		break;

	case REFERENCE_TORQUE:
		if (UpdateFast)
		{
			if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(MODE_TEF_DATA),LOWORD(MODE_TEF_DATA), NO_TML_DATA, NO_TML_DATA, 2))
				return false;
		}
		else
		{
			if (!TS_ExecuteTML(AxisID, MOTION_OP_CODE, HIWORD(MODE_TES_DATA),LOWORD(MODE_TES_DATA), NO_TML_DATA, NO_TML_DATA, 2))
				return false;
		}
		break;

	default:
		return false;
	}

	switch(MoveMoment)
	{
	case UPDATE_ON_EVENT:
		if (!TS_UpdateOnEvent(AxisID))
			return false;
		break;
	case UPDATE_IMMEDIATE:
		if (!TS_UpdateImmediate(AxisID))
			return false;
		break;
	default:
		return false;
	}

	return true;
}

bool TS_SendSynchronization (uint16_t AxisID, uint32_t Period)
{
	/*Send SETSYNC */
	if (!TS_ExecuteTML(AxisID, SETSYNC_OP_CODE, LOWORD(Period),HIWORD(Period), NO_TML_DATA, NO_TML_DATA, 2))
		return false;

	return true;
}

bool TS_ReadFunctionsTable(uint16_t AxisID, uint16_t* FunctionsAddresses, uint8_t* FunctionNo)
{
#ifndef RS232_COM
	CAN_MSG CAN_RX_message;
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
	RS232_MSG RS232_RX_message;
#endif
	TML_INSTR TML_instruction;
	uint16_t FunctionTableEEPROM = 0;
	uint16_t tmpFunctionAddress;

	/*Read function table pointer*/
	TML_instruction.opCode = READ_16_BIT_RAM;
	TML_instruction.length = 2; /*Data request instructions has 2 data words*/
#ifndef RS232_COM
	TML_instruction.TML_data[0] = MASTER_ID << 4;
#else
    TML_instruction.TML_data[0] = (MASTER_ID << 4) | HOST_BIT;
#endif
	TML_instruction.TML_data[1] = FUNCTION_TABLE_POINTER;

	/*Build CAN message to read function table pointer from RAM*/
	/*Send the message with the TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	/*Wait for drive to reply*/

	/*Decode the received message - HW dependent*/
#ifndef RS232_COM
	if ((CAN_RX_message.identifier & 0x000000FF) == AxisID)
	{
		if (!ReceiveMessage(&CAN_RX_message))
			return false;

		FunctionTableEEPROM = ((uint16_t)CAN_RX_message.CAN_data[3] << 8)|CAN_RX_message.CAN_data[2];
	}
#else
	if (RS232_RX_message.RS232_data[4] == AxisID)
	{
		if (!ReceiveMessage(&RS232_RX_message))
			return false;		

		FunctionTableEEPROM = ((uint16_t)RS232_RX_message.RS232_data[7] << 8) | RS232_RX_message.RS232_data[8];
	}
#endif

	if (!FunctionTableEEPROM)
	{
		/*No functions defined on the drive*/
		return false;
	}

	for (uint8_t j=0; j<10;j++)
	{
		TML_instruction.opCode = READ_16_BIT_EEPROM;
		TML_instruction.length = 2; /*Data request instructions has 2 words*/
#ifndef RS232_COM
		TML_instruction.TML_data[0] = MASTER_ID << 4;
#else
        TML_instruction.TML_data[0] = (MASTER_ID << 4) | HOST_BIT;
#endif
		TML_instruction.TML_data[1] = FunctionTableEEPROM;
		/*Send the message with TML instruction - HW dependent*/		
#ifndef RS232_COM
		TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
		if (!SendMessage(&CAN_TX_message))
			return false;
#else
		TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
		if (!SendMessage(&RS232_TX_message))
			return false;	
#endif

	/*Wait for drive to reply*/

	/*Decode the received message - HW dependent*/
#ifndef RS232_COM
		if ((CAN_RX_message.identifier & 0x000000FF) == AxisID)
		{		
			if (!ReceiveMessage(&CAN_RX_message))
				return false;

			tmpFunctionAddress = ((uint16_t)CAN_RX_message.CAN_data[5] << 8) | CAN_RX_message.CAN_data[4];

			if(tmpFunctionAddress == 0)
			{
				/*No more function address*/
				break;
			}

			FunctionsAddresses[j]= tmpFunctionAddress;

			*FunctionNo = *FunctionNo + 1;
			FunctionTableEEPROM++; /*increment EEPROM pointer for next location*/
		}
	}
#else
		if (RS232_RX_message.RS232_data[4] == AxisID)
		{
			if (!ReceiveMessage(&RS232_RX_message))
				return false;		

			tmpFunctionAddress = ((uint16_t)RS232_RX_message.RS232_data[7] << 8) | RS232_RX_message.RS232_data[8];
			if (tmpFunctionAddress == 0)
			{
				/*No more function address*/
				break;
			}

			FunctionsAddresses[j] = tmpFunctionAddress;

			*FunctionNo = *FunctionNo + 1;
			FunctionTableEEPROM++; /*increment EEPROM pointer for next location*/
		}
	}
#endif //RS232_COM
	return true;

}


bool TS_StartFunction(uint16_t AxisID, uint16_t FunctionAddress)
{
#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif

	TML_INSTR TML_instruction;

	if(!FunctionAddress || FunctionAddress < EEPROM_LOWER_ADDR || FunctionAddress> EEPROM_UPPER_ADDR)
	{
		/*if FunctionAddress is zero or outside of EEPROM boundries terminate function*/
		return false;
	}

	TML_instruction.opCode = CALL_FUNCTION;
	TML_instruction.length = 1; /*CALL has 1 data word*/
	TML_instruction.TML_data[0] = FunctionAddress;

	/*Send the message with TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	return true;
}

bool TS_StartFunctionByNumber(uint16_t AxisID, uint16_t FunctionNumber)
{
#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif

	TML_INSTR TML_instruction;
	uint16_t tempOpCode;

	if(FunctionNumber < 1 || FunctionNumber > 10)
	{
		/*The number function can be between 1 and 10 */
		return false;
	}

	TML_instruction.opCode = (CALL_FUNCTION_NO & 0xFFF0) | (FunctionNumber & 0x000F);
	TML_instruction.length = 0; /*FUNCTION x instruction has 0 data word*/

	/*Send the message with TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	return true;
}



bool TS_ExecuteTML(uint16_t AxisID, uint16_t OpCode, uint16_t TMLData1, uint16_t TMLData2, uint16_t TMLData3, uint16_t TMLData4, uint8_t NOWords)
{
#ifndef RS232_COM
	CAN_MSG CAN_TX_message; 
#else
	RS232_MSG RS232_TX_message;
#endif
	TML_INSTR TML_instruction;

	TML_instruction.opCode = OpCode;
	TML_instruction.length = NOWords;
	TML_instruction.TML_data[0] = TMLData1;
	TML_instruction.TML_data[1] = TMLData2;
	TML_instruction.TML_data[2] = TMLData3;
	TML_instruction.TML_data[3] = TMLData4;

	/*Send the message with TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	return true;
}

bool TS_SetOutput(uint16_t AxisID, uint8_t nIO, uint8_t OutValue)
{
#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif
	TML_INSTR TML_instruction;

	if (nIO > 3)
	{
		return false;
	}

	TML_instruction.opCode = SET_RESET_OUT;
	TML_instruction.length = 2;/*OUT instruction has 2 data word*/
	TML_instruction.TML_data[0] = 1 << nIO;
	TML_instruction.TML_data[1] = (OutValue == 0) ? 0: 1 << nIO;
	
	/*Send the message with TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	return true;
}

bool TS_ReadInput(uint16_t AxisID, uint8_t nIO, bool* InValue)
{
#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
	CAN_MSG CAN_RX_message;
#else
	RS232_MSG RS232_TX_message;
	RS232_MSG RS232_RX_message;
#endif
	TML_INSTR TML_instruction;

	if (nIO > 5)
	{
		return false;
	}

	TML_instruction.opCode = READ_16_BIT_RAM;
	TML_instruction.length = 2;/*Data request instructions has 2 data word*/
#ifndef RS232_COM
	TML_instruction.TML_data[0] = MASTER_ID << 4;
#else
    TML_instruction.TML_data[0] = (MASTER_ID << 4) | HOST_BIT;
    RS232_RX_message.length = 10;
#endif
	TML_instruction.TML_data[1] = INSTATUS_ADDR;

	/*Send the message with TML instruction - HW dependent*/
#ifndef RS232_COM
	TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
	if (!SendMessage(&CAN_TX_message))
		return false;
#else
	TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
	if (!SendMessage(&RS232_TX_message))
		return false;	
#endif

	/*Wait for drive to reply*/
	
	/*Decode the received message - HW dependent*/
#ifndef RS232_COM
	if ((CAN_RX_message.identifier & 0x000000FF) == AxisID)
	{
		if (!ReceiveMessage(&CAN_RX_message))
			return false;
		*InValue = (bool)((((uint16_t)CAN_RX_message.CAN_data[3] << 8) | CAN_RX_message.CAN_data[2]) & (uint16_t)(1<<nIO));
	}
#else
	if (RS232_RX_message.RS232_data[4] == AxisID)
	{
		if (!ReceiveMessage(&RS232_RX_message))
			return false;		
		*InValue = (bool)((((uint16_t)RS232_RX_message.RS232_data[7] << 8) | RS232_RX_message.RS232_data[8]) & (uint16_t)(1 << nIO));
	}
#endif
	return true;
}

bool TS_Write16bitValue(uint16_t AxisID, uint16_t MemoryAddress, int16_t WriteValue)
{
#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif
	TML_INSTR TML_instruction;

	if (MemoryAddress)
	{
        if (IS_EEPROM(MemoryAddress))
		{ 	//The write address is from EEPROM
			//EEPROM writes can be done only with indirect addressing via pointer variable

			//Set the pointer variable
			TML_instruction.opCode = WRITE_16_BIT_RAM | (0x01FF & DOWNLOAD_POINTER);
			TML_instruction.length = 1;/*Write instructions has 2 data word*/
			TML_instruction.TML_data[0] = MemoryAddress;
#ifndef RS232_COM
			TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
			/*Send the message with TML instruction - HW dependent*/
			if (!SendMessage(&CAN_TX_message))
				return false;
#else
			TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
			/*Send the message with TML instruction - HW dependent*/
			if (!SendMessage(&RS232_TX_message))
				return false;
#endif			

			//Write to the EEPROM
			TML_instruction.opCode = WRITE_16_BIT_EEPROM;
			TML_instruction.length = 2;/*Write instructions has 2 data word*/
			TML_instruction.TML_data[0] = DOWNLOAD_POINTER;
			TML_instruction.TML_data[1] = WriteValue;
		}
		else
		{
			TML_instruction.opCode = WRITE_16_BIT_RAM | (0x01FF & MemoryAddress);
			if (MemoryAddress > RAM_PAGE_800)
			{
				TML_instruction.opCode |= WRITE_RAM_PAGE_800_BIT;
			}
            TML_instruction.length = 1;/*Write instructions has 1 data words*/
			TML_instruction.TML_data[0] = WriteValue;
		}

		/*Send the message with TML instruction - HW dependent*/
#ifndef RS232_COM
		TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
		if (!SendMessage(&CAN_TX_message))
			return false;
#else
		TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
		if (!SendMessage(&RS232_TX_message))
			return false;
#endif
		return true;
	}
	else
		return false;

}

bool TS_Write32bitValue(uint16_t AxisID, uint16_t MemoryAddress, int32_t WriteValue)
{
#ifndef RS232_COM
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
#endif
	TML_INSTR TML_instruction;

	if (MemoryAddress)
	{
        if (IS_EEPROM(MemoryAddress))
		{ 	//The read address is from EEPROM
			//EEPROM writes can be done only with indirect addressing via pointer variable

			//Set the pointer variable
			TML_instruction.opCode = WRITE_16_BIT_RAM | (0x01FF & DOWNLOAD_POINTER);
			TML_instruction.length = 1;/*Write instructions has 2 data word*/
			TML_instruction.TML_data[0] = MemoryAddress;
#ifndef RS232_COM
			TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
			/*Send the message with TML instruction - HW dependent*/
			if (!SendMessage(&CAN_TX_message))
				return false;
#else
			TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
			/*Send the message with TML instruction - HW dependent*/
			if (!SendMessage(&RS232_TX_message))
				return false;
#endif

			//Write to the EEPROM
			TML_instruction.opCode = WRITE_32_BIT_EEPROM;
			TML_instruction.length = 3;/*Write instructions has 2 data word*/
			TML_instruction.TML_data[0] = DOWNLOAD_POINTER;
			TML_instruction.TML_data[1] = LOWORD(WriteValue);
			TML_instruction.TML_data[2] = HIWORD(WriteValue);
		}
		else
		{
			TML_instruction.opCode = WRITE_32_BIT_RAM | (0x01FF & MemoryAddress);
			if (MemoryAddress > RAM_PAGE_800)
			{
				TML_instruction.opCode |= WRITE_RAM_PAGE_800_BIT;
			}
			TML_instruction.length = 2;/*Write instructions has 2 data word*/
			TML_instruction.TML_data[0] = LOWORD(WriteValue);
			TML_instruction.TML_data[1] = HIWORD(WriteValue);
		}
		
		/*Send the message with TML instruction - HW dependent*/
#ifndef RS232_COM
		TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
		if (!SendMessage(&CAN_TX_message))
			return false;
#else
		TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
		if (!SendMessage(&RS232_TX_message))
			return false;
#endif

		return true;
	}
	else
		return false;

}

bool TS_Read16bitValue(uint16_t AxisID, uint16_t MemoryAddress, int16_t *ReadValue)
{
#ifndef RS232_COM
	CAN_MSG CAN_RX_message;
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
	RS232_MSG RS232_RX_message;
#endif
	TML_INSTR TML_instruction;

	if (MemoryAddress)
	{
        if (IS_EEPROM(MemoryAddress))
		{ 	//The read address is from EEPROM
			TML_instruction.opCode = READ_16_BIT_EEPROM;
#ifdef RS232_COM
            RS232_RX_message.length = 12; //The reply to data request has 12 bytes
#endif
		}
		else
		{
			TML_instruction.opCode = READ_16_BIT_RAM;
#ifdef RS232_COM
            RS232_RX_message.length = 10; //The reply to data request has 10 bytes
#endif
		}

		TML_instruction.length = 2; /*Data request instructions have 2 data word*/
#ifndef RS232_COM
		TML_instruction.TML_data[0] = MASTER_ID << 4;
#else
        TML_instruction.TML_data[0] = (MASTER_ID << 4) | HOST_BIT;
#endif
		TML_instruction.TML_data[1] = MemoryAddress;

#ifndef RS232_COM
		TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
		/*Send the message with TML instruction - HW dependent*/
		if (!SendMessage(&CAN_TX_message))
			return false;
#else
		TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
		/*Send the message with TML instruction - HW dependent*/
		if (!SendMessage(&RS232_TX_message))
			return false;
#endif

	/*Wait for drive to reply*/

	/*Decode the received message - HW dependent*/
    if (IS_EEPROM(MemoryAddress))
	{
#ifndef RS232_COM
		if (!ReceiveMessage(&CAN_RX_message))
			return false;
		*ReadValue = ((uint16_t)CAN_RX_message.CAN_data[5] << 8) | CAN_RX_message.CAN_data[4];
#else
		if (!ReceiveMessage(&RS232_RX_message))
			return false;		
        *ReadValue = ((uint16_t)RS232_RX_message.RS232_data[9] << 8) | RS232_RX_message.RS232_data[10];
#endif
	}
	else
	{	
#ifndef RS232_COM
		if (!ReceiveMessage(&CAN_RX_message))
			return false;		
		*ReadValue = ((uint16_t)CAN_RX_message.CAN_data[3] << 8) | CAN_RX_message.CAN_data[2];
#else
		if (!ReceiveMessage(&RS232_RX_message))
			return false;		
		*ReadValue = ((uint16_t)RS232_RX_message.RS232_data[7] << 8) | RS232_RX_message.RS232_data[8];
#endif
	}
		return true;
	}
	else
		return false;

}

bool TS_Read32bitValue(uint16_t AxisID, uint16_t MemoryAddress, int32_t *ReadValue)
{
#ifndef RS232_COM
	CAN_MSG CAN_RX_message;
	CAN_MSG CAN_TX_message;
#else
	RS232_MSG RS232_TX_message;
	RS232_MSG RS232_RX_message;
#endif
	TML_INSTR TML_instruction;

	if (MemoryAddress)
	{
        if (IS_EEPROM(MemoryAddress))
		{ 	//The read address is from EEPROM
			TML_instruction.opCode = READ_32_BIT_EEPROM;
#ifdef RS232_COM
            RS232_RX_message.length = 14;
#endif
		}
		else
		{
			TML_instruction.opCode = READ_32_BIT_RAM;
#ifdef RS232_COM
            RS232_RX_message.length = 12;
#endif
		}

		TML_instruction.length = 2; /*Data request instructions have 2 data word*/
#ifndef RS232_COM
		TML_instruction.TML_data[0] = MASTER_ID << 4;
#else
        TML_instruction.TML_data[0] = (MASTER_ID << 4) | HOST_BIT;
#endif
		TML_instruction.TML_data[1] = MemoryAddress;
		
		/*Send the message with the TML instruction - HW dependent*/
#ifndef RS232_COM
		TS_TML_to_TMLCAN(AxisID, &TML_instruction, &CAN_TX_message);
		if (!SendMessage(&CAN_TX_message))
			return false;
#else
		TS_TML_to_RS232(AxisID, &TML_instruction, &RS232_TX_message);
		if (!SendMessage(&RS232_TX_message))
			return false;	
#endif

		/*Wait for drive to reply*/

		/*Decode the received message - HW dependent*/
        if (IS_EEPROM(MemoryAddress))
		{
#ifndef RS232_COM
			if (!ReceiveMessage(&CAN_RX_message))
				return false;
			*ReadValue = ((uint32_t)(((uint16_t)CAN_RX_message.CAN_data[7]) << 8 | CAN_RX_message.CAN_data[6]) << 16) | ((uint16_t)CAN_RX_message.CAN_data[5]) << 8 | CAN_RX_message.CAN_data[4];
#else
			if (!ReceiveMessage(&RS232_RX_message))
				return false;		
			*ReadValue = ((uint32_t)(((uint16_t)RS232_RX_message.RS232_data[11]) << 8 | RS232_RX_message.RS232_data[12]) << 16) | ((uint16_t)RS232_RX_message.RS232_data[9]) << 8 | RS232_RX_message.RS232_data[10];
#endif
		} 
		else
		{
#ifndef RS232_COM
			if (!ReceiveMessage(&CAN_RX_message))
				return false;
			*ReadValue = ((uint32_t)((CAN_RX_message.CAN_data[5]) << 8 | CAN_RX_message.CAN_data[4]) << 16) | (CAN_RX_message.CAN_data[3]) << 8 | CAN_RX_message.CAN_data[2];
#else
			if (!ReceiveMessage(&RS232_RX_message))
                return false;
			*ReadValue = ((uint32_t)(((uint16_t)RS232_RX_message.RS232_data[9]) << 8 | RS232_RX_message.RS232_data[10]) << 16) | ((uint16_t)RS232_RX_message.RS232_data[7]) << 8 | RS232_RX_message.RS232_data[8];
#endif

		}
			return true;
	}
	else
		return false;

}


bool TS_TML_to_TMLCAN(uint16_t AxisID, TML_INSTR *TML_instruction, CAN_MSG *CAN_TX_message)
{

	 CAN_TX_message->identifier = ((((uint32_t)(TML_instruction->opCode & 0xFE00)) << 13) | ((uint32_t)AxisID << 13) | (TML_instruction->opCode & 0x01FF));
	 CAN_TX_message->length = TML_instruction->length * 2;

	 for (int j=0; j < TML_instruction->length;j++)
	 {
		 CAN_TX_message->CAN_data[2*j] = LOBYTE(TML_instruction->TML_data[j]);
		 CAN_TX_message->CAN_data[2*j+1] = HIBYTE(TML_instruction->TML_data[j]);
	 }

	return true;
}

bool TS_TML_to_RS232(uint16_t AxisID, TML_INSTR *TML_instruction, RS232_MSG *RS232_TX_message)
{
	unsigned int checksum = 0;
	int i;

	RS232_TX_message->length = TML_instruction->length * 2 + RS232_HEADER;

	/*The message length sent to the drive doesn't contain the length byte and checksum byte */
	RS232_TX_message->RS232_data[0] = RS232_TX_message->length - 2;

	RS232_TX_message->RS232_data[1] = HIBYTE(AxisID << 4);
	RS232_TX_message->RS232_data[2] = LOBYTE(AxisID << 4);
	RS232_TX_message->RS232_data[3] = HIBYTE(TML_instruction->opCode);
	RS232_TX_message->RS232_data[4] = LOBYTE(TML_instruction->opCode);

	for (int j=0; j < TML_instruction->length;j++)
	{
		RS232_TX_message->RS232_data[2 * j + 5] = HIBYTE(TML_instruction->TML_data[j]);
		RS232_TX_message->RS232_data[2 * j + 6] = LOBYTE(TML_instruction->TML_data[j]);
	}

	for (i = RS232_TX_message->RS232_data[0]; i >= 0; i--)
	{
		checksum += RS232_TX_message->RS232_data[i];
	}
	RS232_TX_message->RS232_data[RS232_TX_message->length-1] = (uint8_t)checksum;

	return true;
}

int ReadSwFileLine(FILE* pFileConfig, char* strline)
{
	int iChar;
	int iPos = 0; //position in the char array

	if (pFileConfig == NULL)
		exit(EXIT_FAILURE); //program exits if no file is provided 
		
	while ((iChar = fgetc(pFileConfig)) != EOF)
	{
		if (iChar!='\r' && iChar!='\n')
			strline[iPos++] = iChar;
		else if (iChar == '\n')
			break; //return the string
	}
	strline[iPos] = '\0';

	if (iChar == EOF)
		return 1;
	else
		return 0;
}

void sleep_ms(uint16_t milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}
