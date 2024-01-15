/*
 * libPreciEncoderDriver.cpp
 *
 *  Created on: 4.1.2019
 *      Author: alexander
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <unistd.h>

#include "PreciEncoder.h"
#include "Trigger/libPreciEncoderDriver.h"

namespace precitec
{

namespace trigger
{

/* -*-Func-*-
 *
 * PreciEncoder_read_byte - read byte
 *
 * This function allows to read a byte from the Encoder board
 *
 * RETURNS:
 * The return code of ioctl() if ioctl() fails.
 *
 */
int PreciEncoder_read_board_info  (int p_oFileDesc, struct PreciEncoder_info_struct *p_pData)
{
    if (ioctl(p_oFileDesc, PreciEncoder_GET_BOARD_INFO, p_pData) != 0)
    {
        return -1;
    }
    return 0;
}

/* -*-Func-*-
 *
 * PreciEncoder_read_byte - read byte
 *
 * This function allows to read a byte from the Encoder board
 *
 * RETURNS:
 * The return code of ioctl() if ioctl() fails.
 *
 */
int PreciEncoder_read_byte  (int p_oFileDesc, int p_oRegister_Offset, uint8_t *p_pData)
{
    uint32_t  oDataToKernel[4];

    oDataToKernel[0] = p_oRegister_Offset;
    oDataToKernel[1] = 0x00;

    if (ioctl(p_oFileDesc, PreciEncoder_CMD_READ_BYTE, &oDataToKernel[0]) != 0)
    {
        return -1;
    }
    *p_pData = oDataToKernel[1];
    return 0;
}

/* -*-Func-*-
 *
 * PreciEncoder_write_byte - write byte
 *
 * This function allows to write a byte to the Encoder board
 *
 * RETURNS:
 * The return code of ioctl() if ioctl() fails.
 *
 */
int PreciEncoder_write_byte (int p_oFileDesc, int p_oRegister_Offset, uint8_t p_oData)
{
    uint32_t  oDataToKernel[4];

    oDataToKernel[0] = p_oRegister_Offset;
    oDataToKernel[1] = p_oData;

    if (ioctl(p_oFileDesc, PreciEncoder_CMD_WRITE_BYTE, &oDataToKernel[0]) != 0)
    {
        return -1;
    }
    return 0;
}

/* -*-Func-*-
 *
 * PreciEncoder_read_dword - read dword
 *
 * This function allows to read a dword from the Encoder board
 *
 * RETURNS:
 * The return code of ioctl() if ioctl() fails.
 *
 */
int PreciEncoder_read_dword (int p_oFileDesc, int p_oRegister_Offset, uint32_t *p_pData)
{
    uint32_t  oDataToKernel[4];

    oDataToKernel[0] = p_oRegister_Offset;
    oDataToKernel[1] = 0x00;

    if (ioctl(p_oFileDesc, PreciEncoder_CMD_READ_DWORD, &oDataToKernel[0]) != 0)
    {
        return -1;
    }
    *p_pData = oDataToKernel[1];
    return 0;
}

/* -*-Func-*-
 *
 * PreciEncoder_write_dword - write dword
 *
 * This function allows to write a dword to the Encoder board
 *
 * RETURNS:
 * The return code of ioctl() if ioctl() fails.
 *
 */
int PreciEncoder_write_dword(int p_oFileDesc, int p_oRegister_Offset, uint32_t p_oData)
{
    uint32_t  oDataToKernel[4];

    oDataToKernel[0] = p_oRegister_Offset;
    oDataToKernel[1] = p_oData;

    if (ioctl(p_oFileDesc, PreciEncoder_CMD_WRITE_DWORD, &oDataToKernel[0]) != 0)
    {
        return -1;
    }
    return 0;
}

/* -*-Func-*-
 *
 * PreciEncoder_write_encoder_divider
 *
 * This function allows to write a encoder divider (compare value) to the Encoder board
 *
 * RETURNS:
 * The return code of ioctl() if ioctl() fails.
 *
 */
int PreciEncoder_write_encoder_divider(int p_oFileDesc, uint32_t p_oValue)
{
    uint32_t  oDataToKernel[4];

    oDataToKernel[0] = p_oValue;

    if (ioctl(p_oFileDesc, PreciEncoder_CMD_SET_PRESET_VALUE, &oDataToKernel[0]) != 0)
    {
        return -1;
    }
    return 0;
}

/* -*-Func-*-
 *
 * PreciEncoder_start_encoder_interrupt
 *
 * This function starts the encoder interrupt on the Encoder board
 *
 * RETURNS:
 * The return code of ioctl() if ioctl() fails.
 *
 */
int PreciEncoder_start_encoder_interrupt(int p_oFileDesc)
{
    if (ioctl(p_oFileDesc, PreciEncoder_CMD_START_ENCODER_INTERRUPT, NULL) != 0)
    {
        return -1;
    }
    return 0;
}

/* -*-Func-*-
 *
 * PreciEncoder_stop_encoder_interrupt
 *
 * This function stops the encoder interrupt on the Encoder board
 *
 * RETURNS:
 * The return code of ioctl() if ioctl() fails.
 *
 */
int PreciEncoder_stop_encoder_interrupt(int p_oFileDesc)
{
    if (ioctl(p_oFileDesc, PreciEncoder_CMD_STOP_ENCODER_INTERRUPT, NULL) != 0)
    {
        return -1;
    }
    return 0;
}

} // namespace trigger

} // namespace precitec

