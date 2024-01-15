/*
 * libPreciRS422Driver.cpp
 *
 *  Created on: 12.10.2016
 *      Author: alexander
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <unistd.h>

#include "PreciRS422.h"
#include "viWeldHead/libPreciRS422Driver.h"

/* -*-Func-*-
 *
 * PreciRS422_read_byte - read byte
 *
 * This function allows to read a byte from the RS422 board
 *
 * RETURNS:
 * The return code of ioctl() if ioctl() fails.
 *
 */
int PreciRS422_read_board_info  (int p_oFileDesc, struct PreciRS422_info_struct *p_pData)
{
	if (ioctl(p_oFileDesc, PreciRS422_GET_BOARD_INFO, p_pData) != 0)
	{
		return -1;
	}
	return 0;
}

/* -*-Func-*-
 *
 * PreciRS422_read_byte - read byte
 *
 * This function allows to read a byte from the RS422 board
 *
 * RETURNS:
 * The return code of ioctl() if ioctl() fails.
 *
 */
int PreciRS422_read_byte  (int p_oFileDesc, int p_oRegister_Offset, uint8_t *p_pData)
{
    uint32_t  oDataToKernel[4];

    oDataToKernel[0] = p_oRegister_Offset;
    oDataToKernel[1] = 0x00;

    if (ioctl(p_oFileDesc, PreciRS422_CMD_READ_BYTE, &oDataToKernel[0]) != 0)
    {
    	return -1;
    }
    *p_pData = oDataToKernel[1];
    return 0;
}

/* -*-Func-*-
 *
 * PreciRS422_write_byte - write byte
 *
 * This function allows to write a byte to the RS422 board
 *
 * RETURNS:
 * The return code of ioctl() if ioctl() fails.
 *
 */
int PreciRS422_write_byte (int p_oFileDesc, int p_oRegister_Offset, uint8_t p_oData)
{
    uint32_t  oDataToKernel[4];

    oDataToKernel[0] = p_oRegister_Offset;
    oDataToKernel[1] = p_oData;

    if (ioctl(p_oFileDesc, PreciRS422_CMD_WRITE_BYTE, &oDataToKernel[0]) != 0)
    {
    	return -1;
    }
    return 0;
}

/* -*-Func-*-
 *
 * PreciRS422_read_byte - read byte
 *
 * This function allows to read a byte from the RS422 board
 *
 * RETURNS:
 * The return code of ioctl() if ioctl() fails.
 *
 */
int PreciRS422_read_dword (int p_oFileDesc, int p_oRegister_Offset, uint32_t *p_pData)
{
    uint32_t  oDataToKernel[4];

    oDataToKernel[0] = p_oRegister_Offset;
    oDataToKernel[1] = 0x00;

    if (ioctl(p_oFileDesc, PreciRS422_CMD_READ_DWORD, &oDataToKernel[0]) != 0)
    {
    	return -1;
    }
    *p_pData = oDataToKernel[1];
    return 0;
}

/* -*-Func-*-
 *
 * PreciRS422_write_byte - write byte
 *
 * This function allows to write a byte to the RS422 board
 *
 * RETURNS:
 * The return code of ioctl() if ioctl() fails.
 *
 */
int PreciRS422_write_dword(int p_oFileDesc, int p_oRegister_Offset, uint32_t p_oData)
{
    uint32_t  oDataToKernel[4];

    oDataToKernel[0] = p_oRegister_Offset;
    oDataToKernel[1] = p_oData;

    if (ioctl(p_oFileDesc, PreciRS422_CMD_WRITE_DWORD, &oDataToKernel[0]) != 0)
    {
    	return -1;
    }
    return 0;
}

