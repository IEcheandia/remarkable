/*
 * TML_instructions.h
 *
 *  Copyright (c) 2018, TECHNOSOFT
 *  All rights reserved.
 *  Created on: Apr 9, 2015
 *      Author: g_blujdea
 *  Version: 1.4
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

#ifndef INCLUDE_TML_INSTRUCTIONS_H_
#define INCLUDE_TML_INSTRUCTIONS_H_

#define EEPROM_LOWER_ADDR	0x4000
#define EEPROM_UPPER_ADDR	0x7FFF
#define IS_EEPROM(x) (x >= EEPROM_LOWER_ADDR) && (x <= EEPROM_UPPER_ADDR)	

#define DOWNLOAD_POINTER		0x032b

#define READ_16_BIT_RAM		0xB204
#define READ_32_BIT_RAM		0xB205

#define READ_16_BIT_EEPROM	0xB008
#define READ_32_BIT_EEPROM	0xB009

#define WRITE_16_BIT_EEPROM		0x90A8
#define WRITE_16_BIT_EEPROM_A	0x9028	//write operation with autoincrement of the pointer

#define WRITE_32_BIT_EEPROM		0x90A9
#define WRITE_32_BIT_EEPROM_A	0x9029	//write operation with autoincrement of the pointer

#define WRITE_16_BIT_RAM		0x2000
#define WRITE_32_BIT_RAM		0x2400

#define RAM_PAGE_800			0x0800
#define WRITE_RAM_PAGE_800_BIT	0x0200


#define AXISON_OP_CODE	0x0102
#define AXISOFF_OP_CODE	0x0002

#define ENDINIT_OP_CODE	0x0020

#define CALL_FUNCTION 		0x7401
#define CALL_FUNCTION_P 	0x7601

#define CALLS_FUNCTION		0x1C01
#define CALLS_FUNCTION_P	0x1E01

#define CALL_FUNCTION_NO	0x1C80

#define SET_RESET_OUT		0xEC00

#define SET_CAN_BR			0x0804

#define RESET_AXIS			0x0402

#define RESET_FAULT			0x1C04

#define SAVE_OP_CODE		0x1C08

#define CHECKSUM_SPI		0xDB50

#define UPD_OP_CODE			0x0108
#define UPD_EVENT_OP_CODE	0x0108

#define SAP_OP_CODE			0x0108
#define STA_OP_CODE			0x2CB2
#define STA_DATA			0x0228

#define SRBL_OP_CODE	0x5C00
#define SETSYNC_OP_CODE	0x1404
#define STOP_OP_CODE	0x01C4


#define MOTION_OP_CODE		0x5909 // Op code for TUM1, MODE PP, MODE SP

#define TUM1_DATA		0xFFFF4000
#define MODE_PP_DATA	0xBFC18701
#define MODE_PSC_DATA	0xFFC18707
#define MODE_SP_DATA	0xBBC18301 
#define MODE_PE_DATA	0xBFC08700 
#define MODE_SE_DATA	0xB3C08300
#define MODE_GS_DATA	0xB7C58705 
#define MODE_TES_DATA	0xB1C08100 
#define MODE_TEF_DATA	0xB1E08120 

#define EXTREF_0_DATA	0xFF3F0000 
#define EXTREF_1_DATA	0xFF7F0040  
#define EXTREF_2_DATA	0xFFBF0080  

#define CPA_DATA		0xFFFF2000 
#define CPR_DATA		0xDFFF0000

#define NO_TML_DATA		0 


#endif /* INCLUDE_TML_INSTRUCTIONS_H_ */
