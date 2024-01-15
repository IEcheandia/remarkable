/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Stefan Birmanns (SB)
 *  @date       2012
 *  @brief      Here the log-messages are defined. The log messages are stored in a shared-memory region and collected by the logger server.
 */

// WM includes
#include <common/logMessage.h>
#include <module/moduleLogger.h>
// clib includes
#include <string>
#include <iostream>
#include <iomanip>
// Poco includes
#include <Poco/DateTime.h>
#include <Poco/LocalDateTime.h>

static bool g_hasLostMessages;


namespace {

    void formatTimestamp(std::ostream& oSt, const Poco::Timestamp& rTimestamp)
    {
        Poco::LocalDateTime oTime( rTimestamp );
        oSt << std::setfill('0') << std::setw(2) << oTime.day() << std::setw(0) << "." << std::setw(2) << oTime.month() << std::setw(0) << "." << oTime.year() << " - " << std::setw(2) << oTime.hour() << std::setw(0) << ":" << std::setw(2) << oTime.minute() << std::setw(0) << ":" << std::setw(2) << oTime.second() << "." << std::setw(3) << oTime.millisecond() << std::setw(0) << " | " << std::setfill( ' ');

    }
}

namespace precitec {


std::string LogMessage::format()
{
	std::stringstream oSt;
    Poco::LocalDateTime oTime( m_oTimestamp );

    // type
	switch( m_oType )
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
	oSt << std::string( m_oModule ).substr(0,10) << " | ";

	// message
	oSt << formatParams();

	return oSt.str();

} // format



std::string LogMessage::formatParams( )
{
  std::stringstream oSt;
  unsigned int iCount = 0;
  for( unsigned int i=0; i<LogMessageLength && m_oBuffer[i] != '\0'; ++i )
  {
	  if ( m_oBuffer[i] != '%' )
		  oSt << m_oBuffer[i];
	  else if( m_oBuffer[i+1] != '\0' && i+1<LogMessageLength)
	  {
		  // number parameter expected
		  if ( m_oBuffer[i+1] == 'd' || m_oBuffer[i+1] == 'i')
		  {
			  oSt << int(m_oParams[iCount].value());
			  iCount++;
		  }
		  if ( m_oBuffer[i+1] == 'u')
		  {
			  oSt << uint(m_oParams[iCount].value());
			  iCount++;
		  }
		  if ( m_oBuffer[i+1] == 'x' || m_oBuffer[i+1] == 'X' )
		  {
			  oSt << std::hex << uint(m_oParams[iCount].value()) << std::dec;
			  iCount++;
		  }
		  if ( m_oBuffer[i+1] == 'f' )
		  {
			  oSt << double(m_oParams[iCount].value());
			  iCount++;
		  }
		  // string parameter expected
		  if ( m_oBuffer[i+1] == 's' )
		  {
			  oSt << m_oParams[iCount].string();
			  iCount++;
		  }
		  // Print a % character (The entire specification is %%).
		  if ( m_oBuffer[i+1] == '%' )
		  {
			  oSt << "%%";
			  iCount++;
		  }

		  ++i;
		  if (iCount > LogMessageParams)
			break;
	  }
  }

  return oSt.str();

} // formatParams

bool LogShMemContent::increaseWriteIndex()
{
    // if the read process is run over completely, we need to destroy messages, sorry, at least they are stored in the log file ...
    if  (
            ( m_bRollOver == true  && m_oWriteIndex >= m_oReadIndex-1 ) ||
            ( m_bRollOver == false && m_oWriteIndex >= LogMessageCapacity-1 && m_oReadIndex == 0 )
        )
    {
        if (!g_hasLostMessages)
        {
            formatTimestamp(std::cout, Poco::Timestamp());
            std::cout << "Attention: Log-messages lost!" <<  std::endl;
        }
        else
        {
            std::cout << "!" << std::flush;
        }
        g_hasLostMessages = true;
        return false;
    }

    if (g_hasLostMessages)
    {
        formatTimestamp(std::cout, Poco::Timestamp());
        std::cout << "can increase writeIndex again" <<  std::endl;
        g_hasLostMessages = false;
    }

    // very simple ring buffer approach ...
    m_oWriteIndex++;
    if (m_oWriteIndex >= LogMessageCapacity)
    {
        m_oWriteIndex = 0;
        m_bRollOver = true;
        m_bRollOverGui = true;
    }
    return true;
}


} // namespace precitec
