#include <unistd.h>
#include <iostream>

#include "pipeLogger.h"

int main(int argc, char* argv[])
{
    if (argc >= 3)
    {
        int oReadPipeFd = atoi(argv[1]);              // argv[1] is the fd for the read side of a pipe
        int oWritePipeFd = atoi(argv[2]);             // argv[2] is the fd for the write side of a pipe

        close(oReadPipeFd); // we use the write side of the pipe so close the read fd

        precitec::pipeLogger::SendLoggerMessage(oWritePipeFd, precitec::eDebug, "Start\n");
        precitec::pipeLogger::SendLoggerMessage(oWritePipeFd, precitec::eInfo, "Work\n");
        precitec::pipeLogger::SendLoggerMessage(oWritePipeFd, precitec::eDebug, "End\n");

        close(oWritePipeFd); // close the write fd
    }
    return 0;
}
