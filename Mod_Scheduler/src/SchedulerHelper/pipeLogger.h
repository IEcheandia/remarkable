#pragma once

#include "module/logType.h"

namespace precitec
{

namespace pipeLogger
{

    void SendLoggerMessage(int p_oWritePipeFd, precitec::LogType p_oLogType, const char* p_pLogMsg);

}
}
