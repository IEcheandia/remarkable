#include <unistd.h>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTemporaryDir>

#include "exportHelper.h"
#include "commandLineParser.h"
#include "fileUpload.h"
#include "pipeLogger.h"
#include "../../App_Gui/plugins/general/weldmasterPaths.h"

using precitec::scheduler::ExportHelper;
using precitec::gui::WeldmasterPaths;

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};

    precitec::scheduler::ExportHelper helper{};
    QObject::connect(&helper, &ExportHelper::succeeded, &app, &QCoreApplication::quit);
    int writePipeFd = atoi(argv[1]);             // argv[1] is the fd for the write side of a pipe
    const auto uuid = std::string(argv[2]);

    precitec::scheduler::FileUpload fileUpload{};
    precitec::scheduler::parseCommandLine(argc, argv, fileUpload);
    fileUpload.SetWritePipeFd(writePipeFd);
    fileUpload.SetDebugFileName("StdErr_Of_ExportProject.txt");

    QTemporaryDir temporaryDir;
    temporaryDir.setAutoRemove(false);
    helper.setLogFd(writePipeFd);
    helper.initExportService(uuid, WeldmasterPaths::instance()->productDir(), temporaryDir.path());

    fileUpload.SetSourceDirectory(temporaryDir.path().toStdString());
    fileUpload.SetDirectoryToSend(helper.getProductName().toStdString());
    fileUpload.SetTargetDirectoryName(helper.getProductName().toStdString());
    QObject::connect(&helper, &ExportHelper::succeeded,
        [&helper, &writePipeFd, &fileUpload]
        {
            fileUpload.UploadDirectory();

            {
                std::stringstream loggerText{};
                loggerText << "ExportProduct: NumberOfTransmittedFiles: " << fileUpload.GetNumberOfTransmittedFiles() << std::endl;
                precitec::pipeLogger::SendLoggerMessage(writePipeFd, precitec::eInfo, loggerText.str().c_str());
            }
            {
                std::stringstream loggerText{};
                loggerText << "ExportProduct: TotalTransmittedBytes:    " << fileUpload.GetTotalTransmittedBytes() << std::endl;
                precitec::pipeLogger::SendLoggerMessage(writePipeFd, precitec::eInfo, loggerText.str().c_str());
            }
            {
                std::stringstream loggerText{};
                loggerText << "ExportProduct: DurationOfTransmissionMS: " << fileUpload.GetDurationOfTransmissionMS() << std::endl;
                precitec::pipeLogger::SendLoggerMessage(writePipeFd, precitec::eInfo, loggerText.str().c_str());
            }
            precitec::pipeLogger::SendLoggerMessage(writePipeFd, precitec::eDebug, "End of ExportProduct\n");
            close(writePipeFd); // close the write fd
        });

    helper.start();
    return app.exec();
}
