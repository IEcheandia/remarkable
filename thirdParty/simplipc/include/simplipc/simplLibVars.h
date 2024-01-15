/*======================================================================

FILE:			simplLibVars.h

DESCRIPTION:	This file contains the global variable definitions used
				by the simpl source code library functions.

AUTHOR:			FC Software Inc.

-----------------------------------------------------------------------
    Copyright (C) 2000, 2007 FCSoftware Inc. 

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
$Log: simplLibVars.h,v $
Revision 1.4  2009/01/16 21:37:18  bobfcsoft
changed _simpl_fifoPath from pointer to string

Revision 1.3  2007/07/24 20:18:45  bobfcsoft
new contact info

Revision 1.2  2006/07/11 14:48:02  bobfcsoft
added Relay

Revision 1.1.1.1  2005/03/27 11:50:37  paul_c
Initial import

Revision 1.4  2003/06/23 16:12:45  root
errors have been relocated to simplErrors.h

Revision 1.3  2002/11/22 16:23:10  root
2.0rc3


====================================================================
======================================================================*/

#ifndef _SIMPL_LIB_VARS_H
#define _SIMPL_LIB_VARS_H

#include "simplipc/simplDefs.h"

// globals
//_ALLOC int _simpl_remoteReceiverId[MAX_NUM_REMOTE_RECEIVERS];
_ALLOC char *_simpl_blockedSenderId[MAX_NUM_BLOCKED_SENDERS];
//_ALLOC char _simpl_fifoPath[MAX_FIFO_PATH_LEN + 1];
//_ALLOC int _simpl_sender_shmid;

//#ifdef _SIMPL_PRIMARY
//_ALLOC WHO_AM_I _simpl_myStuff = {"", -1, -1, -1, -1, 0, 0, 0};
//#else
//_ALLOC WHO_AM_I _simpl_myStuff;
//#endif

#endif
