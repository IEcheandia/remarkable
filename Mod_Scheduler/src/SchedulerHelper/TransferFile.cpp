#include <string.h>
#include <unistd.h>

#include <iostream>
#include <filesystem>

#include "module/logType.h"

#include "commandLineParser.h"
#include "fileUpload.h"

#include "pipeLogger.h"

int main(int argc, char* argv[])
{
    precitec::scheduler::FileUpload m_oFileUpload{};

    int oReadPipeFd = atoi(argv[1]);              // argv[1] is the fd for the read side of a pipe
    int oWritePipeFd = atoi(argv[2]);             // argv[2] is the fd for the write side of a pipe

    close(oReadPipeFd); // we use the write side of the pipe so close the read fd

    precitec::pipeLogger::SendLoggerMessage(oWritePipeFd, precitec::eDebug, "Start of TransferFile\n");

    precitec::scheduler::parseCommandLine(argc, argv, m_oFileUpload);

    m_oFileUpload.SetWritePipeFd(oWritePipeFd);
    m_oFileUpload.SetDebugFileName("StdErr_Of_TransferFile.txt");
    m_oFileUpload.UploadFile();

    {
        std::stringstream oLoggerText{};
        oLoggerText << "TransferFile: NumberOfTransmittedFiles: " << m_oFileUpload.GetNumberOfTransmittedFiles() << std::endl;
        precitec::pipeLogger::SendLoggerMessage(oWritePipeFd, precitec::eInfo, oLoggerText.str().c_str());
    }
    {
        std::stringstream oLoggerText{};
        oLoggerText << "TransferFile: TotalTransmittedBytes:    " << m_oFileUpload.GetTotalTransmittedBytes() << std::endl;
        precitec::pipeLogger::SendLoggerMessage(oWritePipeFd, precitec::eInfo, oLoggerText.str().c_str());
    }
    {
        std::stringstream oLoggerText{};
        oLoggerText << "TransferFile: DurationOfTransmissionMS: " << m_oFileUpload.GetDurationOfTransmissionMS() << std::endl;
        precitec::pipeLogger::SendLoggerMessage(oWritePipeFd, precitec::eInfo, oLoggerText.str().c_str());
    }
    precitec::pipeLogger::SendLoggerMessage(oWritePipeFd, precitec::eDebug, "End of TransferFile\n");

    close(oWritePipeFd); // close the write fd

    return 0;
}

