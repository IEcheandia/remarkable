#include <string.h>
#include <unistd.h>
#include <iostream>

#include "pipeLogger.h"

namespace precitec
{

namespace pipeLogger
{

const int MAX_LOGGER_MESSAGE_LENGTH = 150;

void SendLoggerMessage(int p_oWritePipeFd, precitec::LogType p_oLogType, const char* p_pLogMsg)
{
    char oBuffer[MAX_LOGGER_MESSAGE_LENGTH + 1]{};

    // first character in LoggerString # -> eError
    // first character in LoggerString $ -> eWarning
    // first character in LoggerString & -> eInfo
    // first character in LoggerString ? -> eDebug
    switch (p_oLogType)
    {
        case precitec::eError:
        {
            oBuffer[0] = '#';
            break;
        }
        case precitec::eWarning:
        {
            oBuffer[0] = '$';
            break;
        }
        case precitec::eInfo:
        {
            oBuffer[0] = '&';
            break;
        }
        case precitec::eDebug:
        default:
        {
            oBuffer[0] = '?';
            break;
        }
    }
    strncpy(&oBuffer[1], p_pLogMsg, MAX_LOGGER_MESSAGE_LENGTH);
    oBuffer[MAX_LOGGER_MESSAGE_LENGTH] = 0x00; // shorten logger message if it is too long

    if (p_oWritePipeFd > 0)
    {
        long int oReturn = write(p_oWritePipeFd, oBuffer, (MAX_LOGGER_MESSAGE_LENGTH + 1));
        if (oReturn != static_cast<int>(MAX_LOGGER_MESSAGE_LENGTH + 1))
        {
            std::cerr << "Problem with writing to pipe ! " << errno << "," << strerror(errno) << std::endl;
        }
    }
}

}
}
