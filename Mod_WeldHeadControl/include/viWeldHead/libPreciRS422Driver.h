/*
 * libPreciRS422Driver.h
 *
 *  Created on: 12.10.2016
 *      Author: alexander
 */

#ifndef _LIBPRECIRS422DRIVER_H_
#define _LIBPRECIRS422DRIVER_H_


//#ifdef __cplusplus
//extern "C" {
//#endif


int PreciRS422_read_board_info  (int p_oFileDesc, struct PreciRS422_info_struct *p_pData);

int PreciRS422_read_byte  (int p_oFileDesc, int p_oRegister_Offset, uint8_t *p_pData);

int PreciRS422_write_byte (int p_oFileDesc, int p_oRegister_Offset, uint8_t p_oData);

int PreciRS422_read_dword (int p_oFileDesc, int p_oRegister_Offset, uint32_t *p_pData);

int PreciRS422_write_dword(int p_oFileDesc, int p_oRegister_Offset, uint32_t p_oData);


//#ifdef __cplusplus
//}
//#endif


#endif /* _LIBPRECIRS422DRIVER_H_ */

