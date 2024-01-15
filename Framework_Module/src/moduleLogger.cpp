/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Stefan Birmanns (SB)
 *  @date       2012
 *  @brief      The module-internal logger. A central logger server will collect all log-messages and send them to windows (depending on the type of the log message).
 */

// Poco includes
#include <Poco/Mutex.h>
#include <Poco/SharedMemory.h>
#include <Poco/Timestamp.h>
// clib includes
#include <cstdarg>
// WM includes
#include <module/moduleLogger.h>
#include <common/logMessage.h>
#include <module/baseModule.h>
#include <common/defines.h>

namespace precitec {

bool g_hasLostMessages = false;

std::string formatErrorType( LogErrorType p_oType )
{
	std::stringstream oSt;

	if ( p_oType & eLighting)
		oSt << "Lighting ";
	if ( p_oType & eImageAcquisition )
		oSt << "ImageAcquisition ";
	if ( p_oType & eAxis )
		oSt << "Axis ";
	if ( p_oType & eInternalError )
		oSt << "InternalError ";
	if ( p_oType & eAirCondition )
		oSt << "AirCondition ";
	if ( p_oType & eBusSystem)
		oSt << "BusSystem ";
	if ( p_oType & eDataConsistency )
		oSt << "DataConsistency ";
	if ( p_oType & eComputerHardware )
		oSt << "ComputerHardware ";
	if ( p_oType & eHeadMonitor)
		oSt << "HeadMonitor ";
	if ( p_oType & eExtEquipment)
		oSt << "ExtEquipment ";

	return oSt.str();

} // formatErrorType

namespace
{
bool redirectToStdOut()
{
    static const bool s_redirect = (getenv("WM_LOG_STDOUT") != nullptr);
    return s_redirect;
}
}

LogMessage* getLogMessageOfBaseModuleLogger()
{
    if (redirectToStdOut())
    {
        static LogMessage msg;
        return &msg;
    }
    return precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
}


void invokeIncreaseWriteIndex()
{
    if (auto logger = precitec::framework::module::BaseModule::m_pBaseModuleLogger.get())
    {
        logger->increaseWriteIndex();
    }
}

void redirectLogMessage(LogMessage *msg)
{
    if (redirectToStdOut())
    {
        fprintf(stdout, "%s", msg->format().c_str());
    }
}

ModuleLogger::ModuleLogger( std::string p_oModuleName )
{
    m_oModuleName = p_oModuleName;
    m_oModuleName = m_oModuleName.substr( 0, m_oModuleName.find('_') );
    std::cout << "ShMemStream: ModuleName: " << m_oModuleName << std::endl;

    // create the shared memory file name
    const std::string stationName = std::string(getenv("WM_STATION_NAME") ? getenv("WM_STATION_NAME") : "");
    std::string oModuleName = "wmLog" + stationName + p_oModuleName;
    oModuleName = oModuleName.substr( 0, oModuleName.find('_') );
    std::cout << "ShMemStream: Shared Memory Name: " << oModuleName << std::endl;

    // create the SharedMemory buffer for the log-messages
    try {

        g_oMem = Poco::SharedMemory( oModuleName, sizeof( LogShMemContent ), Poco::SharedMemory::AM_WRITE, 0, true );
        m_bShMemReady = true;

    } catch(...) {

        std::cout << "wmShMemStream: Error, was not able to create SharedMemory region ... " << std::endl;
        m_bShMemReady = false;
        return;
    }

    // init the content
    g_pContent = reinterpret_cast<LogShMemContent*>(g_oMem.begin());
    g_pContent->m_oWriteIndex = 0;
    g_pContent->m_oReadIndex = 0;
    g_pContent->m_oReadIndexGui = 0;
    g_pContent->m_bRollOver = false;
    g_pContent->m_bRollOverGui = false;

    // copy the module name to all the messages in this logger
    for( unsigned int i=0; i< LogMessageCapacity; ++i)
    {
    	m_oModuleName.copy( g_pContent->m_oMessages[ i ].m_oModule, LogModuleNameLength, 0 );
    	g_pContent->m_oMessages[ i ].m_oModule[ m_oModuleName.length() ] = '\0';
    }

} // ModuleLogger


LogMessage* ModuleLogger::getMessage()
{
	return &g_pContent->m_oMessages[ g_pContent->m_oWriteIndex ];

} // getFreeMessage



void ModuleLogger::increaseWriteIndex()
{
    g_pContent->increaseWriteIndex();

} // increaseWriteIndex



} // namespace precitec
