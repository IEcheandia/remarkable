/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Andreas Beschorner (BA)
 * 	@date		03/2013
 *  @brief      16bit Alignment defines for windows and QNX, necessary for certain intrinsics and assembly commands.
 */


#ifndef ASMDEFS_H_
#define ASMDEFS_H_

#ifndef align16
#define align16(oExpr) oExpr __attribute__((aligned(16)))
#endif

#endif
