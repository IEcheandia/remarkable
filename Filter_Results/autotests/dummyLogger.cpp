dummyLogger.cpp/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Stefan Birmanns (SB), changes: Daniel Feist (Ft)
 *  @date       2012
 *  @brief      This is a simple dummy logger, which can be used in projects without any modules, like the filtertest.
 */
// Poco includes
#include <Poco/Mutex.h>
#include <Poco/Timestamp.h>
// clib includes
#include <cstdarg>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <string.h>
// WM includes
#include <module/moduleLogger.h>
#include <common/logMessage.h>

Poco::FastMutex 	    g_oLoggerMutex;		///< Only a single thread can write into logger at the same time


FILE * g_pLoggerStream = stdout;
bool g_oLoggerStreamOpened = false; // so that we do not close stdout

namespace precitec {

/**
 * wmLog(), wmLogTr() and wmFatal() are implemented in module/moduleLogger.h and make use of the two free functions 
 * getLogMessageOfBaseModuleLogger and invokeIncreaseWriteIndex defined and implemented in moduleLogger.cpp.
 * They, however, access a ModuleLogger object the BaseModule class owns.
 * Solution: define custom versions of these functions right here.
 */
LogMessage* getLogMessageOfBaseModuleLogger()
{
    static LogMessage msg;
    return &msg;
}

void invokeIncreaseWriteIndex()
{
}

/**
 * This implementation inserts the LogMessages to std::cout.
 */
void redirectLogMessage(LogMessage* msg)
{
    fprintf(g_pLoggerStream, "%s", msg->format().c_str());
}

} // namespace precitec
