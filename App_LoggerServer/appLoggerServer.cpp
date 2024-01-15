/**
 *  @file
 *  @copyright      Precitec Vision GmbH & Co. KG
 *  @author         Stefan Birmanns (SB)
 *  @date           03.05.2012
 *  @brief          This is the central QNX logger server, that collects all the log messages from the modules (and their associated shared-memory regions) and provides them
 *  to the windows side. The wmHost process will poll the messages.
 */

// clib includes
#include <sstream>
// WM includes
#include <weldmasterVersion.h>
#include <changesetId.h>
#include <module/moduleLogger.h>
#include <common/logMessage.h>
#include <appLoggerServer.h>
#include <common/connectionConfiguration.h>
// Poco includes
#include <Poco/Thread.h>
#include <Poco/LocalDateTime.h>
#include <Poco/Mutex.h>
#include <Poco/File.h>
#include <Poco/Runnable.h>

#include <iomanip>

namespace precitec {
namespace inspect {



Poco::FastMutex g_pLoggerWriterMutex; ///< Synchronize access to writer buffer.



std::string formatParams( wmLogItem& p_rItem )
{
	std::stringstream oSt;
	unsigned int iCount = 0;
	bool oIntMsg = p_rItem.key()[0] != '\0';

	for( unsigned int i=0; i<LogMessageLength && p_rItem.message()[i] != '\0'; ++i )
	{
	  if ( p_rItem.message()[i] != '%' || iCount >= p_rItem.getParams().size() || iCount >= LogMessageParams )
		  oSt << p_rItem.message()[i];
	  else if( p_rItem.message()[i+1] != '\0' && i+1<LogMessageLength)
	  {
		  // number parameter expected
		  if ( p_rItem.message()[i+1] == 'd' || p_rItem.message()[i+1] == 'i')
		  {
			  oSt << int(p_rItem.getParams()[iCount].value());
			  iCount++;
		  }
		  if ( p_rItem.message()[i+1] == 'u')
		  {
			  oSt << uint(p_rItem.getParams()[iCount].value());
			  iCount++;
		  }
		  if ( p_rItem.message()[i+1] == 'x')
		  {
			  oSt << std::hex << uint(p_rItem.getParams()[iCount].value()) << std::dec;
			  iCount++;
		  }
		  if ( p_rItem.message()[i+1] == 'f' )
		  {
			  oSt << double(p_rItem.getParams()[iCount].value());
			  iCount++;
		  }
		  // string parameter expected
		  if ( p_rItem.message()[i+1] == 's' )
		  {
			  oSt << p_rItem.getParams()[iCount].string();
			  iCount++;
		  }
          // Print a % character (The entire specification is %%).
          if ( p_rItem.message()[i+1] == '%' )
          {
        	  oSt << "%";

        	  // in case of wmLogTr, there is a parameter that we have to skip, in case of wmLog, there are no parameters attached to the message.
        	  if ( oIntMsg )
        		  iCount++;
          }

		  ++i;
	  }
	}

	return oSt.str();

} // formatParams



std::string formatMsg( wmLogItem p_rItem )
{
	std::stringstream oSt;
    Poco::LocalDateTime oTime( p_rItem.timestamp() );

    // type
	switch( p_rItem.type() )
	{
	case eInfo:
		oSt << "INFO | ";
		break;
	case eWarning:
		oSt << "WARN | ";
		break;
	case eError:
		oSt << "ERR  | ";
		break;
	case eFatal:
		oSt << "FATAL| ";
		break;
	case eDebug:
		oSt << "DEBUG| ";
		break;
	case eStartup:
		oSt << "START| ";
		break;
	case eTracker:
		oSt << "TRACK| ";
		break;
	default:
		oSt << "DEFLT| ";
		break;
	}

	// timestamp
	oSt << std::setfill('0') << std::setw(2) << oTime.day() << std::setw(0) << "." << std::setw(2) << oTime.month() << std::setw(0) << "." << oTime.year() << " - " << std::setw(2) << oTime.hour() << std::setw(0) << ":" << std::setw(2) << oTime.minute() << std::setw(0) << ":" << std::setw(2) << oTime.second() << "." << std::setw(3) << oTime.millisecond() << std::setw(0) << " | " << std::setfill( ' ');

	// module name
	oSt << std::string( p_rItem.moduleName() ).substr(0,10) << " | ";

	// message
	oSt << formatParams( p_rItem );

	return oSt.str();

} // formatMsg




wmLogParam convertParam( LogParam& p_rParam )
{
	wmLogParam oParam;

	if (p_rParam.isString())
		oParam.setString( p_rParam.string() );
	else
		oParam.setValue( p_rParam.value() );

	return oParam;

} // convertParam



wmLogItem convertMsg( LogMessage& p_rMessage, unsigned int p_oIndex )
{
    // OK, now create the wmLogItem:
	if ( p_rMessage.m_oIntKey[0] == '\0' )
	{
		// if we do not have an internationalization key, then the message is send directly and convert all parameters into constant string:
		std::string oMessage = p_rMessage.formatParams();
		wmLogItem oItem( p_rMessage.m_oTimestamp, p_oIndex, oMessage, std::string(""), p_rMessage.m_oModule, p_rMessage.m_oType );
		return oItem;

	} else {

		wmLogItem oItem( p_rMessage.m_oTimestamp, p_oIndex, p_rMessage.m_oBuffer, p_rMessage.m_oIntKey, p_rMessage.m_oModule, p_rMessage.m_oType );
		// now we have to add all valid parameters
		for( unsigned int i=0; i<LogMessageParams; ++i )
			if (p_rMessage.m_oParams[i].isValid())
				oItem.addParam( convertParam( p_rMessage.m_oParams[i] ) );

		return oItem;

	}

} // convertMsg


class WriterRunnable : public Poco::Runnable
{

public:

	/**
	 * @brief CTor.
	 */
	WriterRunnable( LoggerServer& p_rLoggerServer ) : m_rLoggerServer( p_rLoggerServer ), m_oHour( -1 ), m_terminating(false)
	{
	}

    void init()
    {
        if (!m_fileLogging)
        {
            return;
        }
		// open file by appending to the existing content ...
		m_oHour = criterion();
		std::stringstream oSt; oSt << getenv("WM_BASE_DIR") << "/logfiles/wmLog_";
		oSt << std::setfill('0') << std::setw(2) << m_oHour << ".txt";
	    m_oFilestream.open( oSt.str(), std::ofstream::out | std::ofstream::app );
		m_oFilestream << std::endl << "System booting Weldmaster Version " << WELDMASTER_VERSION_STRING 
                        << " id " << WELDMASTER_CHANGESET_ID << " " << WELDMASTER_BUILD_TIMESTAMP << " ... " << std::endl << std::endl;
	}

	int criterion()
	{
		Poco::LocalDateTime oTime;

		return oTime.hour();
	}

	void createFile()
	{
        if (!m_fileLogging)
        {
            return;
        }
		// filename consists of the number / hour, so that the files are overwritten every 24 hours ...
		int oCrit = criterion();
		std::stringstream oSt; oSt << getenv("WM_BASE_DIR") << "/logfiles/wmLog_";
		oSt << std::setfill('0') << std::setw(2) << oCrit << ".txt";

		// now close old file
		if (m_oFilestream.is_open())
		{
			m_oFilestream << std::endl;
			m_oFilestream << "Creating new file " << oSt.str() << std::endl;
			m_oFilestream.close();
		}

		// open new file
	    m_oFilestream.open( oSt.str() );
	    m_oHour = oCrit;
	}

	void checkFileExpired()
	{
        if (!m_fileLogging)
        {
            return;
        }
		int oCrit = criterion();

		if ( oCrit != m_oHour )
			createFile();
	}

	/**
	 * @brief Function running in thread.
	 */
	virtual void run()
	{
        init();
		while(!m_terminating)
		{
			Poco::ScopedLockWithUnlock<Poco::FastMutex> lock{g_pLoggerWriterMutex};
		    unsigned int oCounter = 0;
		    auto oIter = m_rLoggerServer.m_oWriteBuffer.begin();
		    for( ; oIter != m_rLoggerServer.m_oWriteBuffer.end(); ++oIter )
		    {
                if (m_stdOutLogging || m_fileLogging)
                {
                    std::string oMsg = formatMsg( (*oIter) );
                    if (m_stdOutLogging)
                    {
                        std::cout << oMsg;
                    }
                    if (m_fileLogging)
                    {
                        // write message into file - if this creates too much io-load, one should implement a smarter scheme, that flushes only every second or so ...
                        m_oFilestream << oMsg; m_oFilestream.flush();
                    }
                }

		        // make sure only blocks of LogServerMaxItems strings are transmitted to windows ...
		        oCounter++;
		        if (oCounter > LogServerMaxItems)
		        {
		        	++oIter;
		        	break;
		        }
		    }

            if ( oCounter > 0 )
            {
                m_rLoggerServer.m_oWriteBuffer.erase( m_rLoggerServer.m_oWriteBuffer.begin(), oIter );
            }

			lock.unlock();

			// if there were no new messages, lets sleep quite a bit longer ...
			if (oCounter == 0)
				Poco::Thread::sleep( 100 );
			else
				Poco::Thread::sleep( 5 );

			// check if a new file has to be created - once per hour a new file is created.
			checkFileExpired();
		}
	}
	
	void markForExit()
	{
		m_terminating = true;
	}

    void setFileLogging(bool set)
    {
        m_fileLogging = set;
    }

    void setStdOutLogging(bool set)
    {
        m_stdOutLogging = set;
    }

public:

	LoggerServer&	m_rLoggerServer;
    std::ofstream 	m_oFilestream;
    int				m_oHour;
	bool m_terminating;
    bool m_fileLogging = true;
    bool m_stdOutLogging = false;

}; // WriterRunnable



AppLoggerServer::AppLoggerServer() : BaseModule(system::module::LoggerServer),
        m_oInspectionCmdProxy()
{
	ConnectionConfiguration::instance().setInt( pidKeys[LOGGERSERVER_KEY_INDEX], getpid() ); // let ConnectServer know our pid
} // AppLoggerServer



int AppLoggerServer::init(int argc, char * argv[])
{
	processCommandLineArguments(argc, argv);
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-n") == 0)
        {
            m_fileLogging = false;
            continue;
        }
        if (strcmp(argv[i], "--stdout") == 0)
        {
            m_stdOutLogging = true;
            continue;
        }
    }
	// open dev/shmem to find all wmLog shared memory regions, in order to automatically open them here in the logger server.
#ifdef __QNX__
	Poco::File oShDir( "/dev/shmem");
#else
	Poco::File oShDir( "/dev/shm");
#endif
	const std::string stationName = std::string(getenv("WM_STATION_NAME") ? getenv("WM_STATION_NAME") : "");
	std::string oPrefix( "wmLog" + stationName );
	std::vector<std::string> oFiles;
	oShDir.list( oFiles );
	std::vector< std::string > oSharedMem;

	for( auto oIter = begin( oFiles ); oIter != end( oFiles ); ++oIter )
	{
		if ( (*oIter).compare( 0, oPrefix.length(), oPrefix ) == 0 )
		{
			std::cout << "Found logger shared memory region: " << (*oIter) << std::endl;
			oSharedMem.push_back( (*oIter ) );
		}
	}

    // now open the shared memory regions
    for( auto oIter = begin( oSharedMem ); oIter != end( oSharedMem ); ++oIter)
    {
        try {

            m_oMemRegions.push_back( Poco::SharedMemory( (*oIter), sizeof( LogShMemContent ), Poco::SharedMemory::AccessMode(Poco::SharedMemory::AM_READ | Poco::SharedMemory::AM_WRITE), 0, false ) );
            m_oContentPtr.push_back( reinterpret_cast<LogShMemContent*>(m_oMemRegions.back().begin()) );

        } catch(...) {

            // TODO: Unfortunately, we cannot log this, as the base-module is not fully initialized yet - have to find a solution to push it out to the logger, so that the GUI sees the message!
            //wmLog( eError, "wmShMemStream: Error, was not able to open a SharedMemory region, most likely process of %s is not running!\n", (*oIter).c_str() );
            std::cout << "wmShMemStream: Error, was not able to open a SharedMemory region, most likely process of " << (*oIter).c_str() << " is not running!\n";
        }
    }

    // Mark logger server for registration - the GUI client retrieves the log messages using this interface.
    // The LoggerHandler only needs to be registered if the (debug-)LoggerQNX application is supposed to consume the logs. Typically the wmHost would consume the logs, and the following line should be
    // commented out ...

    // call basemodule::initialize (a Poco::App method) - this sends the register-lists to the module manager and starts the servers.
    initialize(this);
    return 0;

} // init



bool AppLoggerServer::activateHostConnections()
{
    wmLog( eDebug, "LoggerServer::activateHostConnections()\n");

    registerPublication(&m_oInspectionCmdProxy);
    subscribeAllInterfaces();
    publishAllInterfaces();

    return true;

} // activateHostConnections



void AppLoggerServer::runClientCode()
{
    activateHostConnections();

    WriterRunnable oWriter( m_oLoggerServer );
    oWriter.setFileLogging(m_fileLogging);
    oWriter.setStdOutLogging(m_stdOutLogging);
    Poco::Thread oThread;
    oThread.start( oWriter );

    notifyStartupFinished();

    bool oFound = true;
    while( !markedForExit() )
    {
        if ( oFound == false )
            Poco::Thread::sleep( 100 );
        else
            oFound = false;

        for( auto oContentIter = begin( m_oContentPtr ); oContentIter != m_oContentPtr.end() ; oContentIter++)
        {
            auto pContent = (*oContentIter);

            if ( pContent->m_oWriteIndex != pContent->m_oReadIndex || pContent->m_bRollOver == true )
            {
				g_pLoggerWriterMutex.lock();

				if ( m_oLoggerServer.m_oWriteBuffer.size() > 10000 )
				{
					auto oIter = m_oLoggerServer.m_oWriteBuffer.begin(); advance( oIter, 100 );
					m_oLoggerServer.m_oWriteBuffer.erase( begin( m_oLoggerServer.m_oWriteBuffer ), oIter );
				}
				m_oLoggerServer.m_oWriteBuffer.insert( convertMsg(pContent->m_oMessages[pContent->m_oReadIndex], pContent->m_oReadIndex) );

				// lets see if the message is an error - in that case we have to stop the system
				if (pContent->m_oMessages[pContent->m_oReadIndex].m_oType == eFatal)
				{
					try
					{
						m_oInspectionCmdProxy.signalNotReady( pContent->m_oMessages[pContent->m_oReadIndex].m_oErrorCode );
					}
					catch(...)
					{
						system::logExcpetion(__FUNCTION__, std::current_exception());
						wmLog(eError, "Remote call 'signalNotReady' failed.\n");
					} // catch

					// Bursts of signalNotReady calls without any break do not work well, after some calls the proxy dies, so lets give the interface some time ...
					Poco::Thread::sleep( 5 );
				}

                // ring-buffer
                pContent->m_oReadIndex++;
                if (pContent->m_oReadIndex >= LogMessageCapacity)
                {
                    pContent->m_oReadIndex = 0;
                    pContent->m_bRollOver = false;
                }
                oFound = true;

				g_pLoggerWriterMutex.unlock();
            }

        } // for
    }

	oWriter.markForExit();
	oThread.join();

} // runClientCode



void AppLoggerServer::uninitialize()
{
} // uninitialize



} // namespace inspect
} // namespace precitec



int main(int argc, char** argv)
{
    using precitec::inspect::AppLoggerServer;
    Poco::AutoPtr<AppLoggerServer> pApp = new AppLoggerServer;
    try {
        pApp->init(argc, argv);
    } catch (Poco::Exception& exc) {
        pApp->logger().log(exc);
        return Poco::Util::Application::EXIT_CONFIG;
    }
    return pApp->run();
}

