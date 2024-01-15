/*
 * libPreciEncoderDriver.h
 *
 *  Created on: 4.1.2019
 *      Author: alexander
 */

#ifndef _LIBPRECIENCODERDRIVER_H_
#define _LIBPRECIENCODERDRIVER_H_

//#ifdef __cplusplus
//extern "C" {
//#endif

namespace precitec
{

namespace trigger
{

int PreciEncoder_read_board_info  (int p_oFileDesc, struct PreciEncoder_info_struct *p_pData);

int PreciEncoder_read_byte  (int p_oFileDesc, int p_oRegister_Offset, uint8_t *p_pData);

int PreciEncoder_write_byte (int p_oFileDesc, int p_oRegister_Offset, uint8_t p_oData);

int PreciEncoder_read_dword (int p_oFileDesc, int p_oRegister_Offset, uint32_t *p_pData);

int PreciEncoder_write_dword(int p_oFileDesc, int p_oRegister_Offset, uint32_t p_oData);

int PreciEncoder_write_encoder_divider(int p_oFileDesc, uint32_t p_oValue);

int PreciEncoder_start_encoder_interrupt(int p_oFileDesc);

int PreciEncoder_stop_encoder_interrupt(int p_oFileDesc);

//#ifdef __cplusplus
//}
//#endif

} // namespace trigger

} // namespace precitec

#endif /* _LIBPRECIENCODERDRIVER_H_ */

