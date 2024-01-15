/**
 *  @defgroup Logger Logger
 *
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *  @author         Stefan Birmanns (SB)
 *	@date           28.03.2012
 *	@brief          This is the central QNX logger server, that collects all the log messages from the modules (and their associated shared-memory regions) and provides them
 *	to the windows side. The wmHost process will poll the messages.
 */
#ifndef APPLOGGERSERVER_H_
#define APPLOGGERSERVER_H_
#pragma once

// WM includes
#include "module/baseModule.h"
//#include "module/logMessage.h"
#include "module/moduleLogger.h"
#include "message/loggerGlobal.interface.h"
#include "event/inspectionCmd.proxy.h"
// Poco includes
#include "Poco/SharedMemory.h"
// stl includes
#include <deque>
#include <iostream>
#include <fstream>

namespace precitec {
using framework::module::BaseModule;

namespace inspect {

// how many log items are transmitted to windows in a single logs() call? Attention: The messages have to fit into the interface buffer, if the number here is increased,
// likely one also has to increase the buffer size in loggerGlobal.interface.h.
const unsigned int LogServerMaxItems = 20;

/**
 * @brief Logger server class of the ipc framework.
 * @ingroup Logger
 */
class LoggerServer
{
public:

	/**
	 * CTor.
	 */
    LoggerServer() {}
    /**
     * DTor.
     */
    virtual ~LoggerServer() {}

public:

public:
    std::set< precitec::interface::wmLogItem, precitec::interface::wmLogItemComp >          m_oWriteBuffer;     ///< The central logger buffer where all the log-messages are stored.
};


/**
 * @brief Application module of the logger server.
 */
class AppLoggerServer : public BaseModule
{
public:

    /// BaseModule does all the real work
    AppLoggerServer();
    /// ensure proper cleanup
    virtual ~AppLoggerServer() { uninitialize(); }

    /// activate the direct connections to wmHost.
    bool activateHostConnections();

    /// this function retrieves the log messages from the shared memory.
    virtual void runClientCode();

public:

    /// replaces main method - publish & subscribe here.
    virtual int init(int argc, char * argv[]);
    /// called by signal handler and dTor: ensure proper cleanup.
    virtual void uninitialize();

private:

    LoggerServer                                            m_oLoggerServer;        ///< The server - does the actual work.

    TInspectionCmd<EventProxy>                              m_oInspectionCmdProxy;  ///< InspectionCmd proxy for the signalNotReady call.

    std::vector< Poco::SharedMemory >                       m_oMemRegions;          ///< Vector of Poco SharedMemory objects.
    std::vector< LogShMemContent* >                         m_oContentPtr;          ///< Direct pointer into the SharedMemory region.
    bool m_fileLogging = true;
    bool m_stdOutLogging = false;
};



} // namespace inspect
} // namespace precitec

#endif // APPLOGGERSERVER_H_
