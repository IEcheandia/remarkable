/*======================================================================

FILE:			simpl.c

DESCRIPTION:	This file contains the function calls for the 
				simpl library used by application programs.

AUTHOR:			FC Software Inc.

-----------------------------------------------------------------------
    Copyright (C) 2000 FCSoftware Inc. 

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
$Log: simpl.c,v $
Revision 1.26  2012/10/30 19:52:15  bobfcsoft
fixed name_attach rc, added CYGWIN hooks

Revision 1.25  2011/03/23 17:20:48  bobfcsoft
added remote name_attach hooks

Revision 1.24  2010/10/25 18:02:14  bobfcsoft
fixed 4096 byte boundary issue

Revision 1.23  2010/04/16 12:00:26  bobfcsoft
allow NULL buffer in Send; changed names to whatsMyShmPtr and whatsThisShmPtr

Revision 1.22  2009/08/19 14:39:28  bobfcsoft
added whatsMyRecvPtr and whatsMyReplyPtr

Revision 1.21  2009/03/16 15:49:50  johnfcsoft
cleanup

Revision 1.20  2009/01/22 19:04:49  johnfcsoft
clean up

Revision 1.19  2009/01/20 15:03:57  bobfcsoft
use DEFAULT_FIFO_PATH

Revision 1.18  2009/01/16 21:40:13  bobfcsoft
improved /tmp default logic

Revision 1.17  2009/01/15 16:43:27  bobfcsoft
added default fifoPath

Revision 1.16  2009/01/13 20:46:22  johnfcsoft
added init/destroy fifo funcs

Revision 1.15  2009/01/12 19:47:40  johnfcsoft
adjusted removal action to statFifoName

Revision 1.14  2008/12/17 15:53:13  bobfcsoft
replaced _simpl_deleteShmem with _simpl_detachShmem

Revision 1.13  2008/12/02 16:21:06  johnfcsoft
check for lowball SIMPL ID's

Revision 1.12  2007/07/24 19:48:01  bobfcsoft
new contact info

Revision 1.11  2006/07/11 14:48:03  bobfcsoft
added Relay

Revision 1.10  2006/04/24 22:10:20  bobfcsoft
cleanup debugging logs

Revision 1.9  2006/01/26 02:13:42  bobfcsoft
v3.0 enhancements

Revision 1.8  2006/01/10 15:29:40  bobfcsoft
v3.0 changes

Revision 1.4  2005/09/26 15:47:22  bobfcsoft
proxy/trigger changes

Revision 1.3  2005/04/21 17:41:30  bobfcsoft
cleanup in name_locate

Revision 1.2  2005/04/21 17:35:06  bobfcsoft
email change

Revision 1.1.1.1  2005/03/27 11:50:53  paul_c
Initial import

Revision 1.23  2005/02/09 13:18:05  root
allow shmid of 0 to denote a trigger

Revision 1.22  2004/12/09 09:12:59  root
expanded error message in write failure

Revision 1.21  2004/10/29 15:07:23  root
enhanced the Trigger and proxy transmission stuff

Revision 1.20  2004/07/13 16:43:11  root
write side of fifo is O_WRONLY now

Revision 1.19  2003/12/08 16:11:42  root
incorporated _simpl_detachShmem to child_detach

Revision 1.18  2003/06/23 16:18:12  root
added simplErrors.h

Revision 1.17  2003/04/14 13:40:04  root
enabled MAC_OS_X stuff

Revision 1.16  2003/01/07 15:12:28  root
HOSTNAME only required in networked name locate case

Revision 1.15  2002/11/22 16:35:44  root
2.0rc3

====================================================================
======================================================================*/

#define _EXPMTL

// system headers
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <sys/stat.h>

// simpl headers 
#include "simplipc/simplDefs.h"
#include "simplipc/simplProto.h"
#include "simplipc/simplLibProto.h"
#include "simplipc/surMsgs.h"

#define _SIMPL_PRIMARY
#define _ALLOC
#include "simplipc/simplLibVars.h"
#include "simplipc/simplErrors.h"
#undef _ALLOC
#undef _SIMPL_PRIMARY

//extern WHO_AM_I _simpl_myStuff;

/**********************************************************************
FUNCTION:	int name_attach(const char *, void(*myExit)())

PURPOSE:	Initializes SIMPL module

RETURNS:	success: >= 0
			failure: -1
***********************************************************************/

int name_attach(const char *myName, void (*myExit)(), WHO_AM_I& _simpl_myStuff)
{
	const static char *fn = "name_attach";
	// WHO_AM_I _simpl_myStuff is global
	// char *_simpl_fifoPath is global
	// int _simpl_remoteReceiverId[] is global
	// char *_simpl_blockedSenderId[] is global
	int rc;
	char protocolName[MAX_PROTOCOL_NAME_LEN + 1];
	char hostName[MAX_HOST_NAME_LEN + 1];
	char processName[MAX_PROGRAM_NAME_LEN + 1];

	// check minimal length of the process name
	if (!strlen(myName))
	{
		_simpl_setErrorCode(NO_NAME);
		return(-1);
	}

	if (access(DEFAULT_FIFO_PATH,F_OK) == -1)
	{
		_simpl_setErrorCode(NO_FIFO_PATH);
		_simpl_log((char *)"%s: no fifo path defined\n", fn);
		return(-1);
	}

	// for checking purposes
	_simpl_myStuff.pid = getpid();

	// extract the protocol (if any), host (if any) and process names
	/*
	 ******************************************************************************
	_simpl-getNames() nulls out the protocolName, hostName and processName strings
	 ******************************************************************************
	 */
	if (_simpl_getNames(myName, protocolName, hostName, processName) == -1)
	{
		_simpl_setErrorCode(NO_NAME);
		return(-1);
	}

	// there must at least be a destination process name
	if ( strlen(processName) == 0 )
	{
		_simpl_setErrorCode(NO_NAME_ATTACHED);
		return(-1);
	}

	// is this process local or remote?
	if ( strlen(hostName) == 0 )
	{
		// no entry in host field implies a local receiver
		rc = _simpl_local_name_attach(processName, myExit, _simpl_myStuff);
	}
	else
	{
		rc = 0;
	}

	return(rc);
} // end name_attach

/**********************************************************************
FUNCTION:	int name_detach(void)

PURPOSE:	Removes SIMPL functionality upon process exit.

RETURNS:	success: 0
			failure: -1
***********************************************************************/

int name_detach(WHO_AM_I& _simpl_myStuff)
{
	//const static char *fn = "name_detach";
	register int i;
	// WHO_AM_I _simpl_myStuff is global
	// int _simpl_remoteReceiverId[] is global
	// char *_simpl_blockedSenderId[] is global

	// is this program name attached?
	if (_simpl_check(_simpl_myStuff) == -1)
	{
		_simpl_setErrorCode(NO_NAME_ATTACHED);
		return(-1);
	}

	// release any reply-blocked senders
	for (i = 0; i < MAX_NUM_BLOCKED_SENDERS; i++)
	{
		if (_simpl_blockedSenderId[i] != (char *)NULL)
		{
			ReplyError(_simpl_blockedSenderId[i], _simpl_myStuff);
			_simpl_blockedSenderId[i] = (char *)NULL;
		}		
	}

//	// remove any surrogates
//	for (i = 0; i < MAX_NUM_REMOTE_RECEIVERS; i++)
//	{
//		if (_simpl_remoteReceiverId[i] != -1)
//		{
//			Trigger(_simpl_remoteReceiverId[i], PROXY_SHUTDOWN);
//			_simpl_remoteReceiverId[i] = -1;
//		}
//	}

	// close the receive file descriptor
	if (_simpl_myStuff.fd != -1)
	{
		close(_simpl_myStuff.fd);
		_simpl_myStuff.fd = -1;
	}

	// close the reply file descriptor
	if (_simpl_myStuff.y_fd != -1)
	{
		close(_simpl_myStuff.y_fd);
		_simpl_myStuff.y_fd = -1;
	}

	// delete message shared memory segment
	if (_simpl_myStuff.shmSize)
	{
		// replaced 2008Dec17
		//	_simpl_deleteShmem();
		_simpl_detachShmem(_simpl_myStuff);
	}

	// remove Receive fifo
	_simpl_destroyFifo(RECEIVE, _simpl_myStuff);

	// remove Reply fifo
	_simpl_destroyFifo(REPLY, _simpl_myStuff);

	// for checking purposes
	_simpl_myStuff.pid = -1;

	return(0);
}

/**********************************************************************
FUNCTION:	int child_detach(void)

PURPOSE:	Removes a child process' access to the parent's fifos in
			the case of a parent process which has name attached and
			then forked a child.

RETURNS:	success: 0
			failure: -1
***********************************************************************/

int child_detach(WHO_AM_I& _simpl_myStuff)
{
	//const static char *fn = "child_detach";
	// WHO_AM_I _simpl_myStuff is global

	// is this program name attached?
	if (_simpl_check(_simpl_myStuff) == -1)
	{
		_simpl_setErrorCode(NO_NAME_ATTACHED);
		return(-1);
	}

	// close the receive file descriptor
	if (_simpl_myStuff.fd != -1)
	{
		close(_simpl_myStuff.fd);
		_simpl_myStuff.fd = -1;
	}

	// close the reply file descriptor
	if (_simpl_myStuff.y_fd != -1)
	{
		close(_simpl_myStuff.y_fd);
		_simpl_myStuff.y_fd = -1;
	}

	// detach message shared memory segment
	if (_simpl_myStuff.shmSize)
	{
		_simpl_detachShmem(_simpl_myStuff);
	}

	// set the pid to -1 in case of later checking
	_simpl_myStuff.pid = -1;

	return(0);
}

/**********************************************************************
FUNCTION:	int sur_detach(int)

PURPOSE:	This function sends a closure message to a surrogate
			receiver. 

RETURNS:	success: 0
			failure: -1
***********************************************************************/

int sur_detach(int fd, WHO_AM_I& _simpl_myStuff)
{
//	register int i;
	// int _simpl_remoteReceiverId[] is global

	// is this program name attached?
	if (_simpl_check(_simpl_myStuff) == -1)
	{
		_simpl_setErrorCode(NO_NAME_ATTACHED);
		return(-1);
	}

//	// remove surrogate from the remote receiver table
//	for (i = 0; i < MAX_NUM_REMOTE_RECEIVERS; i++)
//	{
//		if (_simpl_remoteReceiverId[i] == fd)
//		{
//			Trigger(fd, PROXY_SHUTDOWN);
//			_simpl_remoteReceiverId[i] = -1;
//			break;
//		}
//	}

	return(0);
}

/**********************************************************************
FUNCTION:	int name_locate(const char *)

PURPOSE:	Returns the fd of the receive fifo of a simpl receiver.

RETURNS:	success: >= 0
			failure: -1
***********************************************************************/

int name_locate(const char *names, WHO_AM_I& _simpl_myStuff)
{
	int rc;
	char protocolName[MAX_PROTOCOL_NAME_LEN + 1];
	char hostName[MAX_HOST_NAME_LEN + 1];
	char processName[MAX_PROGRAM_NAME_LEN + 1];

	// is this program name attached?
	if (_simpl_check(_simpl_myStuff) == -1)
	{
		_simpl_setErrorCode(NO_NAME_ATTACHED);
		return(-1);
	}

	// extract the protocol (if any), host (if any) and process names
	/*
	 ******************************************************************************
	_simpl-getNames() nulls out the protocolName, hostName and processName strings
	 ******************************************************************************
	 */
	if (_simpl_getNames(names, protocolName, hostName, processName) == -1)
	{
		_simpl_setErrorCode(NO_NAME);
		return(-1);
	}

	// there must at least be a destination process name
	if ( strlen(processName) == 0 )
	{
		_simpl_setErrorCode(NO_NAME_ATTACHED);
		return(-1);
	}

	// is this process local or remote?
	if ( strlen(hostName) == 0 )
	{
		// no entry in host field implies a local receiver
		rc = _simpl_local_name_locate(processName);
	}
	else
	{
		rc = 0;
	}

	return(rc);
}

/**********************************************************************
FUNCTION:	int Receive(char **, void *, unsigned)

PURPOSE:	This function receives simpl messages from other processes.

RETURNS:	success: message size in bytes
			failure: -1
***********************************************************************/

int Receive(char **sender, void *inBuffer, unsigned maxBytes, int64_t& _simpl_sender_shmid, WHO_AM_I& _simpl_myStuff)
{
	const static char *fn = "Receive";
	char fifoBuf[sizeof(FIFO_MSG)];
	FIFO_MSG *fifoMsg = (FIFO_MSG *)fifoBuf;
	FCMSG_REC *msgRec;
	// WHO_AM_I _simpl_myStuff is global
	// char *_simpl_fifoPath is global
	// int _simpl_sender_shmid is global

	// is this program name attached?
	if (_simpl_check(_simpl_myStuff) == -1)
	{
		_simpl_setErrorCode(NO_NAME_ATTACHED);
		return(-1);
	}

	// reset the _simpl_sender_shmid
	_simpl_sender_shmid = -1;

	// wait on the fifo for a triggering message from a sending process
	if (_simpl_readFifoMsg(_simpl_myStuff.fd, fifoBuf) != sizeof(FIFO_MSG))
	{
		_simpl_setErrorCode(FIFO_READ_FAILURE);
		_simpl_log((char *)"%s: unable to read fifo message\n", fn);
		close(_simpl_myStuff.fd);
		_simpl_myStuff.fd = -1;
		return(-1);
	} 

	// set the global for sender's shmid for Relay to work
	_simpl_sender_shmid = fifoMsg->shmid;

	// is the message a proxy?
	if (fifoMsg->shmid < 0)
	{
		return(-1+fifoMsg->shmid); // -2 or less (shmid is already negative)
	}

	/*
	Attach the sender's shmem to this process.
	Known to fail if sender suddenly disappears.
	Saving this value allows the Reply() to use the same shmem.
	 */
#ifdef _EXPMTL
	*sender = (char *)px_shmat(fifoMsg->shmid, 0, 0);
#else
	*sender = shmat(fifoMsg->shmid, 0, 0);
#endif
	if (*sender == (char *)-1)
	{
		_simpl_setErrorCode(CANNOT_ATTACH_SHMEM);
		_simpl_log((char *)"%s: shmid=%ld cannot attach to shmem-%s\n", fn, fifoMsg->shmid, strerror(errno));
		ReplyError(*sender, _simpl_myStuff);
		return(-1);
	}

	// line up on the message
	msgRec = (FCMSG_REC *)*sender;

	// copy the data out of the shmem or not?
	if (inBuffer != NULL)
	{
		// copy the shmem contents
		if ((unsigned int)msgRec->nbytes > maxBytes)
		{
			_simpl_setErrorCode(RECEIVE_MESSAGE_TOO_LARGE);
			_simpl_log((char *)"%s: message size %d > buffer size %d\n", fn, msgRec->nbytes, maxBytes);
			ReplyError(*sender, _simpl_myStuff);
			return(-1);
		}

		// copy the message
		memcpy(inBuffer, (void *)&msgRec->data, msgRec->nbytes);
	}

	// save this sender in case of failure before a reply can made
	saveSenderId(*sender);

	// return the size of the message
	return(msgRec->nbytes);
}

/**********************************************************************
FUNCTION:	int Send(int, void *, void *, unsigned, unsigned)

PURPOSE:	This function sends simpl messages to other processes.

RETURNS:	success: number of bytes from Reply >= 0
			failure: -1
***********************************************************************/

int Send(int fd, void *outBuffer, void *inBuffer, unsigned outBytes, unsigned inBytes, WHO_AM_I& _simpl_myStuff)
{
	const static char *fn = "Send";
	char fifoBuf[sizeof(FIFO_MSG)];
	FIFO_MSG *fifoMsg = (FIFO_MSG *)fifoBuf;
	unsigned bufSize;
	FCMSG_REC *msgPtr;
	// WHO_AM_I _simpl_myStuff is global
	// char *_simpl_fifoPath is global

	// is this program name attached?
	if (_simpl_check(_simpl_myStuff) == -1)
	{
		_simpl_setErrorCode(NO_NAME_ATTACHED);
		return(-1);
	}

	// check the simpl id == fd
	if (fd < 3)
	{
		_simpl_setErrorCode(SIMPL_ID_OUT_OF_RANGE);
		return(-1);
	}

	// calculate the largest buffer size
	bufSize = (outBytes >= inBytes) ? outBytes : inBytes;

	// build shmem as needed
	if (_simpl_myStuff.shmSize < (bufSize + sizeof(FCMSG_REC)))
	{
		// delete any past shmem
		if (_simpl_myStuff.shmSize)
		{
			// replaced 2008Dec17
			// _simpl_deleteShmem();
			_simpl_detachShmem(_simpl_myStuff);
		}
	
		// create new shmem
		if (_simpl_createShmem(bufSize, _simpl_myStuff) == -1)
		{
			return(-1);
		}
	}

	// copy the message into shmem to be read by the receiver
	msgPtr = (FCMSG_REC *)_simpl_myStuff.shmPtr;

	strcpy(msgPtr->whom, _simpl_myStuff.whom);
	msgPtr->pid = _simpl_myStuff.pid;
	msgPtr->shmid = _simpl_myStuff.shmid;
	msgPtr->shmsize = _simpl_myStuff.shmSize;
	msgPtr->nbytes = outBytes;
	msgPtr->ybytes = inBytes;
	if(outBuffer != NULL)
		memcpy((void *)&msgPtr->data, outBuffer, outBytes);

	// line up the triggering message for the fifo
	fifoMsg->shmid = _simpl_myStuff.shmid;

	// receiver reads the fifo and then sender's shmem
	if (write(fd, fifoBuf, sizeof(FIFO_MSG)) != sizeof(FIFO_MSG))
	{
		_simpl_setErrorCode(FIFO_WRITE_FAILURE);
		_simpl_log((char *)"%s: unable to write to fifo -%s\n", fn, strerror(errno));
		return(-1);
	}

	// wait for the receiver to send fifo message to trigger the reply
	if (_simpl_readFifoMsg(_simpl_myStuff.y_fd, fifoBuf) != sizeof(FIFO_MSG))
	{
		_simpl_setErrorCode(FIFO_READ_FAILURE);
		_simpl_log((char *)"%s: unable to read from fifo\n", fn);
		close(_simpl_myStuff.y_fd);
		_simpl_myStuff.y_fd = -1;
		return(-1);
	}

	// was there a problem in the send?
	if (fifoMsg->shmid == -1)
	{
		_simpl_setErrorCode(COMMUNICATION_ERROR);
		_simpl_log((char *)"%s: Receive/Reply problem\n", fn);
		return(-1);
	}

	if (inBuffer != NULL)
	{
		// copy the reply message
		if (msgPtr->nbytes)
		{ 
			memcpy(inBuffer, (void *)&msgPtr->data, msgPtr->nbytes);
		}
	}

	// return the sizeof the reply message
	return(msgPtr->nbytes);
}

/**********************************************************************
FUNCTION:	int Reply(char *, void *, unsigned)

PURPOSE:	This function replies simpl messages to sender processes.

RETURNS:	success: number of reply bytes (nbytes) >= 0
			failure: -1
***********************************************************************/

int Reply(char *sender, void *outBuffer, unsigned nbytes, int64_t& _simpl_sender_shmid, WHO_AM_I& _simpl_myStuff)
{
	const static char *fn = "Reply";
	char fifoBuf[sizeof(FIFO_MSG)];
	FIFO_MSG *fifoMsg = (FIFO_MSG *)fifoBuf;
	FCMSG_REC *msgPtr;
	char fifoName[128];
	int fd;
	int ret = -1;
	// WHO_AM_I _simpl_myStuff is global
	// char *_simpl_fifoPath is global
	// int _simpl_sender_shmid is global

	// is this program name attached?
	if (_simpl_check(_simpl_myStuff) == -1)
	{
		_simpl_setErrorCode(NO_NAME_ATTACHED);
		return(-1);
	}

	// set a pointer to the sender's shmem
	msgPtr = (FCMSG_REC *)sender;

	// set the sender's reply fifo path and name
	sprintf(fifoName, "%s/Y%s.%d", DEFAULT_FIFO_PATH, msgPtr->whom, msgPtr->pid);

	// check that sender's reply buffer is large enough
	if (nbytes > (unsigned int)msgPtr->ybytes)
	{
		// set up fifo trigger message for error
		fifoMsg->shmid = -1;

		// set error
		_simpl_setErrorCode(REPLY_MESSAGE_TOO_LARGE);
	}
	else
	{
		// set up fifo trigger message for success
		fifoMsg->shmid = 0;

		// set the reply message header
		msgPtr->nbytes = nbytes;

		// copy the reply message into sender's shmem
		if(outBuffer != NULL)
			memcpy((void *)&msgPtr->data, outBuffer, nbytes);

		ret = nbytes;
	}

	// detach sender's shmem from this process
#ifdef _EXPMTL
	px_shmdt(sender, _simpl_myStuff);
#else
	shmdt(sender);
#endif

	// open the sender's fifo
	fd = open(fifoName, O_WRONLY);
	if (fd == -1)
	{
		_simpl_setErrorCode(FIFO_OPEN_FAILURE);
		_simpl_log((char *)"%s: unable to open fifo-%s\n", fn, strerror(errno));
		return(-1);
	}

	// write the fifo message to unblock the sender
	if (write(fd, fifoBuf, sizeof(FIFO_MSG)) != sizeof(FIFO_MSG))
	{
		_simpl_setErrorCode(FIFO_WRITE_FAILURE);
		_simpl_log((char *)"%s: unable to write to fifo\n", fn);
		close(fd);
		return(-1);
	}

	// close the sender's fifo
	close(fd);

	// reset the _simpl_sender_shmid
	_simpl_sender_shmid = -1;

	// remove this sender that was saved in case of failure
	removeSenderId(sender);

	return(ret);
}

/**********************************************************************
FUNCTION:	int Relay(char *, int)

PURPOSE:	This function relays SIMPL message to another process.

RETURNS:	success: 0
			failure: -1
***********************************************************************/

int Relay(char *sender, int fd, int64_t& _simpl_sender_shmid, WHO_AM_I& _simpl_myStuff)
{
	const static char *fn = "Relay";
	char fifoBuf[sizeof(FIFO_MSG)];
	FIFO_MSG *fifoMsg = (FIFO_MSG *)fifoBuf;
	// WHO_AM_I _simpl_myStuff is global
	// int _simpl_sender_shmid is global

	fifoMsg->shmid = _simpl_sender_shmid;

	// write the fifo message to unblock the sender
	if (write(fd, fifoBuf, sizeof(FIFO_MSG)) != sizeof(FIFO_MSG))
	{
		_simpl_setErrorCode(FIFO_WRITE_FAILURE);
		_simpl_log((char *)"%s: unable to write to fifo\n", fn);
		close(fd);
		return(-1);
	}
	else
	{
		// remove this sender that was saved in case of failure
		removeSenderId(sender);

		// reset the _simpl_sender_shmid
		_simpl_sender_shmid = -1;

		// detach sender's shmem from this process
#ifdef _EXPMTL
		px_shmdt(sender, _simpl_myStuff);
#else
		shmdt(sender);
#endif
	}

	return(0);
}

/**********************************************************************
FUNCTION:	int Trigger(int, int)

PURPOSE:	This function sends a proxy to receiver type process.

RETURNS:	success: 0
			failure: -1
***********************************************************************/

int Trigger(int fd, int proxy, WHO_AM_I& _simpl_myStuff)
{
	// const static char *fn = "Trigger";
	char fifoBuf[sizeof(FIFO_MSG)];
	FIFO_MSG *fifoMsg = (FIFO_MSG *)fifoBuf;

	// is this program name attached?
	if (_simpl_check(_simpl_myStuff) == -1)
	{
		_simpl_setErrorCode(NO_NAME_ATTACHED);
		return(-1);
	}

	// check proxy value
	if (proxy < 1)
	{
		_simpl_setErrorCode(IMPROPER_PROXY);
		return(-1);
	}

	// negative value marks a proxy
	fifoMsg->shmid = -proxy;

	if (write(fd, fifoBuf, sizeof(FIFO_MSG)) != sizeof(FIFO_MSG))
	{
		_simpl_setErrorCode(FIFO_WRITE_FAILURE);
		return(-1);
	}

	return(0);
}

/**********************************************************************
FUNCTION:	char *whatsMyName(void)

PURPOSE:	Return the attached simpl name. 

RET:URNS:	success: name
			failure: NULL
***********************************************************************/

char *whatsMyName(WHO_AM_I& _simpl_myStuff)
{
	// WHO_AM_I _simpl_myStuff is global

	// is this program name attached?
	if (_simpl_check(_simpl_myStuff) == -1)
	{
		_simpl_setErrorCode(NO_NAME_ATTACHED);
		return( (char *)NULL );
	}

	return(_simpl_myStuff.whom);
} 

/**********************************************************************
FUNCTION:	int whatsMyRecvfd(void)

PURPOSE:	Return the receive fifo file descriptor of the calling
			process. 

			If there isn't one, a fifo will be made.
 
RETURNS:	success: file descriptor >= 0
			failure: -1
***********************************************************************/

int whatsMyRecvfd(WHO_AM_I& _simpl_myStuff)
{
	const static char *fn = "whatsMyRecvfd";
	// WHO_AM_I _simpl_myStuff is global
	// char *_simpl_fifoPath is global

	// is this program name attached?
	if (_simpl_check(_simpl_myStuff) == -1)
	{
		_simpl_setErrorCode(NO_NAME_ATTACHED);
		return(-1);
	}

	if (_simpl_myStuff.fd == -1)
	{
		char fifoName[128];

		sprintf(fifoName, "%s/%s.%d",
				DEFAULT_FIFO_PATH,
				_simpl_myStuff.whom,
				_simpl_myStuff.pid);

		_simpl_myStuff.fd = open(fifoName, O_RDWR);
		if (_simpl_myStuff.fd == -1)
		{
			_simpl_setErrorCode(FIFO_OPEN_FAILURE);
			_simpl_log((char *)"%s: unable to open reply fifo %s-%s\n", fn, fifoName, strerror(errno));
			return(-1);
		} 
	}

	return(_simpl_myStuff.fd);
} 

/**********************************************************************
FUNCTION:	int whatsMyReplyfd(void)

PURPOSE:	Return the reply fifo file descriptor of the calling
			process. 

			If there isn't one, a fifo will be made.
 
RETURNS:	success: file descriptor >= 0
			failure: -1
***********************************************************************/

int whatsMyReplyfd(WHO_AM_I& _simpl_myStuff)
{
	const static char *fn = "whatsMyReplyfd";
	// WHO_AM_I _simpl_myStuff is global
	// char *_simpl_fifoPath is global

	// is this program name attached?
	if (_simpl_check(_simpl_myStuff) == -1)
	{
		_simpl_setErrorCode(NO_NAME_ATTACHED);
		return(-1);
	}

	if (_simpl_myStuff.y_fd == -1)
	{
		char fifoName[128];

		sprintf(fifoName, "%s/Y%s.%d",
				DEFAULT_FIFO_PATH,
				_simpl_myStuff.whom,
				_simpl_myStuff.pid);

		_simpl_myStuff.y_fd = open(fifoName, O_RDWR);
		if (_simpl_myStuff.y_fd == -1)
		{
			_simpl_setErrorCode(FIFO_OPEN_FAILURE);
			_simpl_log((char *)"%s: unable to open reply fifo %s-%s\n", fn, fifoName, strerror(errno));
			return(-1);
		} 
	}

	return(_simpl_myStuff.y_fd);
}

/**********************************************************************
FUNCTION:	char *whatsMyError(void)

PURPOSE:	Return the text description based on the _simpl_errno. 

RETURNS:	Pointer to the global error string array
***********************************************************************/

//inline char *whatsMyError()
char *whatsMyError()
{
	// int _simpl_errno is global: it is set by _simpl_setErrorCode()

	return(_simpl_errstr[_simpl_errno]);
}
 
/**********************************************************************
FUNCTION:	char *whatsThisShmPtr(char *)

PURPOSE:	returns the pointer to the message in shm
		Follows a Receiver(&id, NULL, 0) call.

RETURNS:	pointer to message
***********************************************************************/

char *whatsThisShmPtr(char *sender)
{
	FCMSG_REC *msgPtr = (FCMSG_REC *)sender;

	return(&msgPtr->data);
} 

/**********************************************************************
FUNCTION:	char *whatsMyShmPtr()

PURPOSE:	returns the pointer to the message in shm
		Follows a Send(id, outbuf, NULL, sbytes, rbytes) call or
		precedes a Send(id, NULL, inbuf, sbytes, rbytes) call

RETURNS:	pointer to message
***********************************************************************/

char *whatsMyShmPtr(WHO_AM_I& _simpl_myStuff)
{
	FCMSG_REC *msgPtr = (FCMSG_REC *)_simpl_myStuff.shmPtr;

	return(&msgPtr->data);
} 

/**********************************************************************
FUNCTION:	void simplSetReceiverParms(char *, SIMPL_REC *)

PURPOSE:	Return information about a sender.
			Called from a receiver AFTER a Receive().

RETURNS:	success: 0
			failure: -1
***********************************************************************/

int simplSetReceiverParms(char *name, SIMPL_REC *rec)
{
	char fifoName[128];

	// set the receiver's name
	memcpy(rec->whom, name, MAX_PROGRAM_NAME_LEN + 1);

	// get receiver's receive fifo in order to extract its pid
	if (_simpl_getFifoName(name, fifoName) == -1)
	{
		return(-1);
	}

	// extract the pid; fifo name is of the form: _simpl_fifoPath/name.1234
	rec->pid = atoi(fifoName + strlen(DEFAULT_FIFO_PATH) + 1 + strlen(name) + 1);

	return(0);
}
 
/**********************************************************************
FUNCTION:	void simplSetSenderParms(char *, SIMPL_REC *)

PURPOSE:	Return information about a sender.
			Called from a receiver AFTER a Receive().

RETURNS:	nothing
***********************************************************************/

void simplSetSenderParms(char *sender, SIMPL_REC *rec)
{
	FCMSG_REC *ptr = (FCMSG_REC *)sender;

	memcpy(rec->whom, ptr->whom, MAX_PROGRAM_NAME_LEN + 1);
	rec->pid = ptr->pid;
}
 
/**********************************************************************
FUNCTION:	int simplCheckProcess(SIMPL_REC *)

PURPOSE:	Checks on sender's existence.

RETURNS:	sender exists: 0
			sender does not exist: -1
***********************************************************************/

int simplCheckProcess(SIMPL_REC *rec)
{
	// char *_simpl_fifoPath is global
	char fifoFile[128];
	int ret = 0;

	// check for actual process existence
	// errno must be cleared prior to getpriority() call
	errno = 0;
	if (getpriority(PRIO_PROCESS, rec->pid) == -1)
	{
		if (errno == ESRCH)
		{
			// this program is not running: remove old fifos if existent
			sprintf(fifoFile, "%s/%s.%d", DEFAULT_FIFO_PATH, rec->whom, rec->pid);
			if (access(fifoFile, F_OK) == 0)
			{
				remove(fifoFile);
			}
			sprintf(fifoFile, "%s/Y%s.%d", DEFAULT_FIFO_PATH, rec->whom, rec->pid);
			if (access(fifoFile, F_OK) == 0)
			{
				remove(fifoFile);
			}
			ret = -1;
		}
	}

	return(ret);
} 

/**********************************************************************
FUNCTION:	void simplRcopy(char *, void *, unsigned)

PURPOSE:	Copy unsigned nbytes from the sender's shmem pointer
			to the desired address void *dst 

			Follows a Receiver(&id, NULL, 0) call.

RETURNS:	nothing
***********************************************************************/

void simplRcopy(char *sender, void *dst, unsigned nbytes)
{
	FCMSG_REC *msgPtr = (FCMSG_REC *)sender;

	memcpy(dst, (void *)&msgPtr->data, nbytes);
} 
 
/**********************************************************************
FUNCTION:	void simplScopy(void *, unsigned)

PURPOSE:	Copy unsigned nbytes from the sender's global shmem pointer
			to the desired address void * 

			Follows a Send(id, void *, NULL, outSize, inSize) call

RETURNS:	nothing
***********************************************************************/

void simplScopy(void *dst, unsigned nbytes, WHO_AM_I& _simpl_myStuff)
{
	FCMSG_REC *msgPtr = (FCMSG_REC *)_simpl_myStuff.shmPtr;

	memcpy(dst, (void *)&msgPtr->data, nbytes);
}
 
/**********************************************************************
FUNCTION:	int simplReplySize(char *)

PURPOSE:	Return the size of the reply message specified in a sender's
			Send() call.

			Follows a Receive() and before the Reply().

RETURNS:	int
***********************************************************************/

int simplReplySize(char *sender)
{
	FCMSG_REC *msgPtr;

	// set a pointer to the sender's shmem
	msgPtr = (FCMSG_REC *)sender;

	return(msgPtr->ybytes);
} 

/**********************************************************************
FUNCTION:	int returnProxy(int)

PURPOSE:	Return the true value of a received proxy. 

RETURNS:	The value of the proxy should be > 0.
		(the value entered should be <= -2)
***********************************************************************/

//inline int returnProxy(int value)
int returnProxy(int value)
{
	return( abs(value + 1) );
}

