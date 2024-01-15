/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Stefan Birmanns (SB), HS
 *  @date       2014
 *  @brief      Type of log message.
 */
#ifndef LOGTYPE_H_
#define LOGTYPE_H_

namespace precitec {

/**
 * Type of log message. eDebug messages are not send to the GUI - but are stored, as the other messages, on the QNX side in the log files. Although this does not cause any network traffic to the windows side, please keep in mind
 * that the bandwidth to the hard disk is also limited and shared with other processes (e.g. video-recorder).
 */
enum LogType
{
	eInfo = 1,				///< Information message, not critical.
	eWarning = 2, 			///< Warning, something has happened, which should not have happened, but which has no immediate, drastic consequences.
	eError = 4,				///< Something really bad has happened. This does not mean that we cannot inspect parts anymore, so this will not set the state machine into the NotReadyState.
	eFatal = 8,				///< A fatal error has occurred, inspection of parts is not possible anymore. Be careful, this will get signaled to the SPS and will therefore typically prohibit the machine to produce parts.
	eDebug = 16,			///< Debug messages are not send to the GUI - but are stored, as the other messages, on the QNX side in the log files. Although this does not cause any network traffic to the windows side, please keep in mind that the bandwidth to the hard disk is also limited and shared with other processes (e.g. video-recorder).
	eStartup = 32,			///< This message type is only used internally, to signal the windows side that a process has been started ...
	eTracker = 64			///< This is a message, generated out of serial messages of the scantracker
};

} // namespace precitec

#endif /* LOGTYPE_H_ */
