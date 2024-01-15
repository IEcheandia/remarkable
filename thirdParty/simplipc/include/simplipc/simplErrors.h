/*======================================================================

FILE:			simplErrors.h

DESCRIPTION:	This file contains the global variable error definitions
				used by the simpl source code library functions.

AUTHOR:			FC Software Inc.

-----------------------------------------------------------------------
    Copyright (C) 2000,2005,2007 FCSoftware Inc. 

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    If you discover a bug or add an enhancement contact us on the 
    SIMPL project mailing list.

-----------------------------------------------------------------------

Revision history:
====================================================================
$Log: simplErrors.h,v $
Revision 1.8  2009/01/16 21:36:43  bobfcsoft
FIFO_PATH TOO LONG added

Revision 1.7  2009/01/13 20:44:29  johnfcsoft
added fifo type errors

Revision 1.6  2009/01/12 19:45:27  johnfcsoft
 added orphaned fifo

Revision 1.5  2008/12/02 16:21:57  johnfcsoft
add lowball SIMPL ID's error

Revision 1.4  2007/07/24 20:18:45  bobfcsoft
new contact info

Revision 1.3  2006/01/10 15:28:45  bobfcsoft
v3.0 changes

Revision 1.2  2005/09/26 15:46:12  bobfcsoft
proxy/trigger changes

Revision 1.1.1.1  2005/03/27 11:50:37  paul_c
Initial import

Revision 1.1  2003/06/23 16:11:08  root
Initial revision


====================================================================
======================================================================*/

#ifndef _SIMPL_ERRORS_H
#define _SIMPL_ERRORS_H

typedef enum
	{
	NO_ERROR=0,
	NO_NAME,
	NO_FIFO_PATH,
	NAME_NOT_AVAILABLE,
	CANNOT_CREATE_SHMEM,
	CANNOT_DELETE_SHMEM,
	CANNOT_ATTACH_SHMEM,
	CANNOT_DETACH_SHMEM,
	CANNOT_UNLINK_SHMEM,
	NO_NAME_ATTACHED,
	FIFO_CREATION_FAILURE,
	FIFO_GET_FAILURE,
	FIFO_OPEN_FAILURE,
	FIFO_READ_FAILURE,
	FIFO_WRITE_FAILURE,
	BAD_FIFO_DESCRIPTOR,
	NAME_LOCATE_SURROGATE_FAILURE,
	HOST_NAME_FAILURE,
	NO_NAMED_PROCESS_RUNNING,
	RECEIVE_MESSAGE_TOO_LARGE,
	REPLY_MESSAGE_TOO_LARGE,
	COMMUNICATION_ERROR,
	NAME_TOO_SHORT,
	TOO_MANY_COLONS,
	NO_SYSTEM_HOSTNAME,
	SYSTEM_HOSTNAME_TOO_LONG,
	PROTOCOL_NOT_AVAILABLE,
	TABLE_FULL,
	FIFO_DIRECTORY_OPEN_ERROR,
	COMMAND_LINE_ERROR,
	IMPROPER_PROXY,
	LOCAL_HOST_PROBLEM,
	SIMPL_ID_OUT_OF_RANGE,
	ORPHANED_FIFO,
	FIFO_NAMING_FAILURE,
	FIFO_CHMOD_FAILURE,
	FIFO_PATH_TOO_LONG,
	MAX_ERROR_TYPES
	} SIMPL_ERROR;

#ifdef _SIMPL_PRIMARY
_ALLOC SIMPL_ERROR _simpl_errno = NO_ERROR;
_ALLOC char *_simpl_errstr[MAX_ERROR_TYPES] = 
	{
	(char *)"no error",
	(char *)"no simpl name supplied",
	(char *)"no simpl fifo path available - check export variable FIFO_PATH",
	(char *)"simpl name is probably in use",
	(char *)"cannot create shmem for message communication",
	(char *)"cannot delete shmem from message communication",
	(char *)"cannot attach shmem created for message communication",
	(char *)"cannot detach shmem created for message communication",
	(char *)"cannot remove shmem created for message communication",
	(char *)"no simpl name has been attached to this process",
	(char *)"error in creating trigger fifo",
	(char *)"error in getting trigger fifo",
	(char *)"error in opening trigger fifo",
	(char *)"error in reading from trigger fifo",
	(char *)"error in writing to trigger fifo",
	(char *)"bad fifo file descriptor",
	(char *)"cannot find the surrogate process",
	(char *)"cannot ascertain current host name",
	(char *)"cannot locate remote process",
	(char *)"received message too large for receiver buffer",
	(char *)"reply message too large for sender buffer",
	(char *)"receive/reply problem ... could be a failed receiver",
	(char *)"requested name too short",
	(char *)"too many colons in requested name locate",
	(char *)"no system host name set",
	(char *)"system host name is too long",
	(char *)"protocol not in router table",
	(char *)"no more room in remote receiver table",
	(char *)"cannot open fifo directory",
	(char *)"command line args parsing error",
	(char *)"proxy value must be >= 1",
	(char *)"local host has no available IP information",
	(char *)"SIMPL ID is out of range for allowable file descriptors",
	(char *)"orphaned fifo detected on name_locate()",
	(char *)"incorrect fifo type; RECEIVE or REPLY only",
	(char *)"error setting permissions on fifo",
	(char *)"FIFO_PATH too long"
	};
#else
_ALLOC SIMPL_ERROR _simpl_errno;
_ALLOC char *_simpl_errstr[MAX_ERROR_TYPES]; 
#endif

#endif
