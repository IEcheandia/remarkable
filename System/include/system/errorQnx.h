#ifndef ERRORQNX_H_
#define ERRORQNX_H_
#pragma once

/**
 *  System::errorQnx.h
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         Wolfgang Reichl (WoR)
 *	@date           22.03.2012
 *	@brief					provide a simple interface to convert qnx-return values into error-messages
 */

#if defined __QNX__ || defined __linux__
#include <errno.h> // wg sys_errlist


namespace precitec
{
namespace system
{
	/**
	 * look qnx error messages up in system table
	 * this fnction is defined for Win and thus avoids #ifdef orgies in application files
	 * @param error qnx return negative values, this function doesn't care
	 * @return
	 */
	PvString errorToMessage(int error) {
		if (error==0) return "no error";
		// qnx return s neg values; the table needs the (negated) positive values
		if (error<0) error = -error;
		return PvString(sys_errlist[error]);
	}

#else
	/// empty as it will never be called or even referenced
	PvString errorToMessage(int ) { return ""; }
#endif

} // namespace system
} // namespace precitec

#endif // ERRORQNX_H_
