/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Stefan Birmanns (SB), changes: Daniel Feist (Ft)
 *  @date       2012
 *  @brief      The module-internal logger. A central logger server will collect all log-messages and send them to windows (depending on the type of the log message).
 */
#ifndef MODULELOGGER_H_
#define MODULELOGGER_H_

// Poco includes
#include <Poco/Mutex.h>
#include <Poco/SharedMemory.h>
#include <Poco/Timestamp.h>

// local includes
#include "logType.h"

// clib includes
#include <string>
#include <cstdarg>
#include <cstring>

// WM includes

#include "common/logMessage.h"

#if !defined(THELOGGER_API)
	#define THELOGGER_API
#endif

namespace precitec {

/**
 * The following error-type represent categories of problems that can lead to a not-ready state of the system. They are defined from a user
 * perspective, i.e. lighting, axis, etc. are categories that the user knows and cares about, he has no knowledge about e.g. the processes running on the real-time
 * system. If something purely software related happens, e.g. if a process crashes, one has to raise an eInternalError.
 */
enum LogErrorType
{
	eNone = 0,					///< No error, every component of the system is up and running.
	eLighting = 1,				///< No flash, no laser line detected, etc.
	eImageAcquisition = 2,		///< Grabber not ready, not present, no camera, grabber initialization failed, etc.
	eAxis = 4,					///< Y or Z axis problems
	eInternalError = 8,			///< Process has crashed, or cannot be contacted anymore, etc.
	eAirCondition = 16,			///< Temperature in control cabinet is too high ...
	eBusSystem = 32,			///< Problems with sps communication, etc.
	eDataConsistency = 64,		///< Data consistency, e.g. if the product configuration is corrupt, filter graphs cannot be executed, etc.
	eComputerHardware = 128,	///< CPU temp. or hard-disk problems, etc.
	eHeadMonitor = 256,			///< Problem with head monitor - glas dirty, etc.
	eExtEquipment = 512			///< Problem with external Equipment, e.g. LaserControl, etc.
};

static Poco::FastMutex         g_oLoggerMutex;     ///< Only a single thread can write into logger at the same time

/**
 * @brief Get LogMessages from a baseModule object.
 */
LogMessage* getLogMessageOfBaseModuleLogger();

/**
 * @brief Advance the ring buffer the LogMessages are stored in.
 */
void invokeIncreaseWriteIndex();

/**
 * @brief Every LogMessage is passed to this function - to redirect LogMessages, implement redirectLogMessage in corresponding cpp file.
 */
void redirectLogMessage(LogMessage* msg);

/**
 * @brief Format an error code into a string. Will return a string of variable length that will also reflect combinations of several error codes.
 * @param p_oType LogErrorType with the error code that is supposed to be converted into a std::string.
 * @return Resulting string.
 */
std::string THELOGGER_API formatErrorType( LogErrorType p_oType );


/**
 * @brief
 */
template <class... Ts>
void fillLogMessage( LogMessage* pMsg, std::string p_oString, std::string p_oIntKey, LogType p_oType, unsigned int p_oErrorCode, Ts... rest )
{
    // set timestamp
    pMsg->m_oTimestamp = Poco::Timestamp();

    // copy message string (but only upto the max length)
    p_oString.copy( pMsg->m_oBuffer, LogMessageLength-2, 0 );

    if ( p_oString.length() < LogMessageLength )
    {
        pMsg->m_oBuffer[p_oString.length()] = '\0';
    }
    else 
    {
        // if string is longer we have to cut it ...
        pMsg->m_oBuffer[LogMessageLength-2] = '\n';
        pMsg->m_oBuffer[LogMessageLength-1] = '\0';
    }
       
    std::strncpy(pMsg->m_oIntKey, p_oIntKey.c_str(), sizeof(pMsg->m_oIntKey)-1);
    pMsg->m_oIntKey[sizeof(pMsg->m_oIntKey)-1] = '\0';
    
    // copy type
    pMsg->m_oType = p_oType;

    // set error code
    pMsg->m_oErrorCode = p_oErrorCode;

    // expand parameter pack
    pMsg->extractParams(p_oString, rest...);
}


/**
 * @brief Output a log message. The message will appear in a module-specific log file (e.g. /wm_inst/data/WorkflowModule.log_XXXXX.txt).
 *
 * If logtype > eDebug, the message will be send to windows. Debug messages are only stored on the QNX side, in the module specific log file.
 * The message can contain numbers and string parameters. In that case, the parameters have to be separated from the main text (for translation) - they are simply appended
 * after the message text. The message text has to use the well known printf formatting syntax to embed the parameters into the text:
 *
 * wmLog( eInfo, "Product-ID: %s - ProductNr: %i\n", oProductID, oProductNr );
 *
 * String-parameters can only be 40 characters long and are simply cut-off if longer. One can only use up to 5 parameters.
 *
 * @param p_oString Message string.
 */
template <class... Ts>
void THELOGGER_API wmLog( LogType p_oType, std::string p_oString, Ts... rest )
{
    Poco::FastMutex::ScopedLock lock( g_oLoggerMutex );

    LogMessage* pMsg = getLogMessageOfBaseModuleLogger();

    //              LogMessage*, p_oString, p_oIntKey, LogType, p_oErrorCode, Ts...
    fillLogMessage( pMsg,        p_oString, "\0",      p_oType, eNone,        rest... );

    // now increase the write index
    invokeIncreaseWriteIndex();

    // to redirect LogMessages, implement redirectLogMessage in corresponding cpp file.
    redirectLogMessage(pMsg);
}


/**
 * @brief Output a log message that is supposed to be translated on the GUI. The message will appear in a module-specific log file (e.g. /wm_inst/data/WorkflowModule.log_XXXXX.txt).
 *
 * If logtype > eDebug, the message will be send to windows. Debug messages are only stored on the QNX side, in the module specific log file.
 * The message can contain numbers and string parameters. In that case, the parameters have to be separated from the main text (for translation) - they are simply appended
 * after the message text. The message text has to use the well known printf formatting syntax to embed the parameters into the text:
 *
 * wmLogTr( eInfo, "Precitec.Hyper.Mega", "Product-ID: %s - ProductNr: %i\n", oProductID, oProductNr );
 *
 * String-parameters can only be 40 characters long and are simply cut-off if longer. One can only use up to 5 parameters.
 * The internationalization key can only be up to 40 characters and is cut-off otherwise.
 *
 * @param p_oIntKey internationalization key.
 * @param p_oString Message string.
 */
template <typename... Ts>
void THELOGGER_API wmLogTr( LogType p_oType, std::string p_oIntKey, std::string p_oString, Ts... rest )
{
    Poco::FastMutex::ScopedLock lock( g_oLoggerMutex );

    LogMessage* pMsg = getLogMessageOfBaseModuleLogger();

    //              LogMessage*, p_oString, p_oIntKey, LogType, p_oErrorCode, Ts...
    fillLogMessage( pMsg,        p_oString, p_oIntKey, p_oType, eNone,        rest... );

    // now increase the write index
    invokeIncreaseWriteIndex();

    // to redirect LogMessages, implement redirectLogMessage in corresponding cpp file.
    redirectLogMessage(pMsg);
}


/**
 * @brief Send a system exception. A system exception is a fatal error, the state machine will go into a not-ready state and the system will for example not inspect parts any longer.
 * As this is a very important call, one has to specify an internationalization key (every fatal error has to be translated so that the user can understand the reason).
 *
 * wmFatal( eImageAcquisition, "Grabber.Init", "Was not able to initialize camera.\n" );
 *
 * @param p_oErrorCode the error code.
 * @param p_oIntKey internationalization key.
 * @param p_oString message string.
 */
template <typename... Ts>
void THELOGGER_API wmFatal( LogErrorType p_oErrorCode, std::string p_oIntKey, std::string p_oString, Ts... rest )
{
    Poco::FastMutex::ScopedLock lock( g_oLoggerMutex );

    LogMessage* pMsg = getLogMessageOfBaseModuleLogger();

    //              LogMessage*, p_oString, p_oIntKey, LogType, p_oErrorCode, Ts...
    fillLogMessage( pMsg,        p_oString, p_oIntKey, eFatal,  p_oErrorCode, rest... );

    // now increase the write index
    invokeIncreaseWriteIndex();

    // to redirect LogMessages, implement redirectLogMessage in corresponding cpp file.
    redirectLogMessage(pMsg);
}

// forward decl.
class LogMessage;

/**
 * @brief SharedMemory Logger
 */
class ModuleLogger
{
public:
	/**
	 * @brief CTor.
	 */
	ModuleLogger( std::string p_oModuleName );

	/**
	 * @brief Get the current message, that the logger should write to.
	 * @return Pointer to wmShMemMessage object.
	 */
	LogMessage* getMessage();
	/**
	 * @brief Increase the write index after the message was changed.
	 */
	void increaseWriteIndex();

private:
    Poco::SharedMemory  g_oMem;             ///< Poco SharedMemory object.
    LogShMemContent*    g_pContent;         ///< Direct pointer into the SharedMemory region.

    std::string         m_oModuleName;      ///< Name of the module this logger belongs to.
    bool                m_bShMemReady;      ///< Is the shared memory region ready and initialized?
};

} // namespace precitec

#endif /* MODULELOGGER_H_ */
