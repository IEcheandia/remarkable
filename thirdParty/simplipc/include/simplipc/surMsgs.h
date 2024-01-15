/*======================================================================

FILE:			surMsgs.h

DESCRIPTION:	This file contains message structure and constant
				definitions used by the simpl source.

AUTHOR:			FC Software Inc.

-----------------------------------------------------------------------
    Copyright (C) 2000-2009 FCSoftware Inc. 

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
$Log: surMsgs.h,v $
Revision 1.7  2011/03/23 16:56:42  bobfcsoft
added hooks for remote name_attach

Revision 1.6  2007/09/12 16:31:55  bobfcsoft
enabling RS232 surrogates

Revision 1.5  2007/07/24 20:18:45  bobfcsoft
new contact info

Revision 1.4  2006/01/10 15:28:45  bobfcsoft
v3.0 changes

Revision 1.1.1.1  2005/03/27 11:50:37  paul_c
Initial import

Revision 1.3  2003/04/14 13:22:37  root
compatibility with powerPC chips

Revision 1.2  2002/11/22 16:24:14  root
2.0rc3


====================================================================
======================================================================*/

/*
NOTE
----

Read the note in simplDefs.h regarding char type variables and
structures.
*/

#ifndef _SUR_MSGS_H
#define _SUR_MSGS_H

#include "simplipc/simplDefs.h"

typedef enum
	{
	SUR_NAME_ATTACH,
	SUR_NAME_DETACH,
	SUR_NAME_LOCATE,
	SUR_SEND,
	SUR_REPLY,
	SUR_CLOSE,
	SUR_PROXY,
	SUR_ERROR,
	SUR_ALIVE,
	SUR_ALIVE_REPLY,
	SUR_SURROGATE_READY,
	SUR_REQUEST_PROTOCOL,
	SUR_DUMP_TABLE,
	MAX_SUR_TOKENS
	} SUR_TOKEN;

// 12 bytes
typedef struct
	{
	int token;
	unsigned nbytes;
	unsigned ybytes;
	int surPid;
	} SUR_MSG_HDR;	

// 24 bytes
typedef struct
	{
	char token[8];
	char nbytes[8];
	char ybytes[8];
	char surPid[8];
	} SUR_MSG_CHR_HDR;	

// 156 bytes
typedef struct
	{
	SUR_MSG_HDR hdr;
	// host name where the remote receiver lives
	char rHostName[MAX_HOST_NAME_LEN + 1];
	// remote receiver's SIMPL name
	char rProgramName[MAX_PROGRAM_NAME_LEN + 1];
	// host name where the sender lives
	char sHostName[MAX_HOST_NAME_LEN + 1];
	} SUR_NAME_LOCATE_MSG;

// 124 bytes
typedef struct
	{
	SUR_MSG_CHR_HDR hdr;
	// host name where the remote receiver lives
	char rHostName[MAX_HOST_NAME_LEN + 1];
	// remote receiver's SIMPL name
	char rProgramName[MAX_PROGRAM_NAME_LEN + 1];
	// host name where the sender lives
	char sHostName[MAX_HOST_NAME_LEN + 1];
	} SUR_NAME_LOCATE_CHR_MSG;

//  20 bytes
typedef struct
	{
	SUR_MSG_HDR hdr;
	int result;
	} SUR_NAME_LOCATE_REPLY;

// 40 nbytes
typedef struct
	{
	SUR_MSG_CHR_HDR hdr;
	char result[8];
	} SUR_NAME_LOCATE_CHR_REPLY;

typedef struct
	{
	SUR_MSG_HDR hdr;
	// host name where the remote receiver lives
	char rHostName[MAX_HOST_NAME_LEN + 1];
	// remote receiver's SIMPL name
	char rProgramName[MAX_PROGRAM_NAME_LEN + 1];
	// local receiver's SIMPL name
	char lProgramName[MAX_PROGRAM_NAME_LEN + 1];
	} SUR_NAME_ATTACH_MSG;

typedef struct
	{
	SUR_MSG_HDR hdr;
	int result;
	} SUR_NAME_ATTACH_REPLY;

typedef struct
	{
	SUR_MSG_CHR_HDR hdr;
	// host name where the remote receiver lives
	char rHostName[MAX_HOST_NAME_LEN + 1];
	// remote receiver's SIMPL name
	char rProgramName[MAX_PROGRAM_NAME_LEN + 1];
	// local receiver's SIMPL name
	char lProgramName[MAX_PROGRAM_NAME_LEN + 1];
	} SUR_NAME_ATTACH_CHR_MSG;

typedef struct
	{
	SUR_MSG_CHR_HDR hdr;
	char result[8];
	} SUR_NAME_ATTACH_CHR_REPLY;

//  20 bytes
typedef struct
	{
	SUR_MSG_HDR hdr;
	int proxyValue;
	} SUR_PROXY_MSG;

//  32 bytes
typedef struct
	{
	SUR_MSG_CHR_HDR hdr;
	char proxyValue[8];
	} SUR_CHR_PROXY_MSG;

// 20 bytes
typedef struct
	{
	SUR_MSG_HDR hdr;
	} SUR_KA_MSG;

// 40 bytes
typedef struct
	{
	SUR_MSG_CHR_HDR hdr;
	} SUR_KA_CHR_MSG;

// 16 bytes
typedef struct
	{
	SUR_MSG_HDR hdr;
	} SUR_KA_REPLY_MSG;

// 32 bytes
typedef struct
	{
	SUR_MSG_CHR_HDR hdr;
	} SUR_KA_REPLY_CHR_MSG;

// 72 bytes
typedef struct
	{
	SUR_MSG_HDR hdr;
	char protocolName[MAX_PROTOCOL_NAME_LEN + 1];
	char programName[MAX_PROGRAM_NAME_LEN + 1];
	} SUR_PROTOCOL_MSG;

// 36 bytes
typedef struct
	{
	SUR_MSG_HDR hdr;
	char protocolName[MAX_PROTOCOL_NAME_LEN + 1];
	} SUR_REQUEST_PROTOCOL_MSG;

// 32 bytes
typedef struct
	{
	char programName[MAX_PROGRAM_NAME_LEN + 1];
	} SUR_PROTOCOL_REPLY_MSG;

// 12 bytes
typedef struct
	{
	SUR_MSG_HDR hdr;
	} SUR_DUMP_TABLE_MSG;

// 560 bytes
typedef struct
	{
	PROTOCOL_TABLE_ENTRY table[MAX_NUM_PROTOCOLS];
	} SUR_DUMP_TABLE_REPLY_MSG;

#endif
