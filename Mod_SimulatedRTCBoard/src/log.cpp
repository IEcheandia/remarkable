#include "../include/log.h"
#include <thread>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <iostream>
#include <variant>
#include <algorithm>
#include <tuple>



using namespace std;

namespace RTCLogging
{
    
RTCLog::RTCLog():
m_oWriteIndex(MAX_LINE_NUMBER), 
m_FileCount(0)
{   
}



void RTCLog::createNewLogFile()
{
		std::stringstream oSt_LogFilePath;
        oSt_LogFilePath << getenv("WM_BASE_DIR") << "/logfiles/RTC_Log_";
        oSt_LogFilePath << std::setfill('0') << std::setw(2)  << m_oWriteIndex << ".txt";

 		// now close old file
		if (m_oFile.is_open())
		{
			m_oFile << std::endl;
			m_oFile << "Creating new file " << oSt_LogFilePath.str() << std::endl;
			m_oFile.close();
		}

		// open new file
	    m_oFile.open( oSt_LogFilePath.str() );
 }
}

