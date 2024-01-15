#include <unistd.h>
#include <iostream>

#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTimer>

#include "module/logType.h"

#include "commandLineParser.h"
#include "fileUpload.h"

#include "pipeLogger.h"

#include "resultExcelFileFromProductInstance.h"
#include "../../App_Gui/plugins/general/weldmasterPaths.h"
#include "../../App_Gui/plugins/general/guiConfiguration.h"
#include "../../Mod_Storage/src/extendedProductInfoHelper.h"

#include <filesystem>
namespace fs = std::filesystem;
using namespace precitec::pipeLogger;
using precitec::gui::WeldmasterPaths;
using precitec::gui::GuiConfiguration;

void upload(int argc, char* argv[], const QString &directoryName, const QString &fileName)
{
    precitec::scheduler::FileUpload m_oFileUpload{};
    int oWritePipeFd = atoi(argv[1]);             // is the fd for the write side of a pipe

    precitec::scheduler::parseCommandLine(argc, argv, m_oFileUpload);

    m_oFileUpload.SetTargetFileName(fileName.toStdString());      // is the filename of the file on the remote target

    m_oFileUpload.SetWritePipeFd(oWritePipeFd);
    m_oFileUpload.SetDebugFileName("StdErr_Of_ResultExcelFileFromProductInstance.txt");

    m_oFileUpload.SetFileToSend(fileName.toStdString());         // is the filename of the source file
    m_oFileUpload.SetSourceDirectory(directoryName.toStdString());    // is the directory of the source file

    m_oFileUpload.UploadFile();
    {
        std::stringstream oLoggerText{};
        oLoggerText << "ResultExcelFileFromProductInstance: TotalTransmittedBytes:    " << m_oFileUpload.GetTotalTransmittedBytes() << std::endl;
        SendLoggerMessage(oWritePipeFd, precitec::eInfo, oLoggerText.str().c_str());
    }
    {
        std::stringstream oLoggerText{};
        oLoggerText << "ResultExcelFileFromProductInstance:: DurationOfTransmissionMS: " << m_oFileUpload.GetDurationOfTransmissionMS() << std::endl;
        SendLoggerMessage(oWritePipeFd, precitec::eInfo, oLoggerText.str().c_str());
    }
    SendLoggerMessage(oWritePipeFd, precitec::eDebug, "End of ResultExcelFileFromProductInstance\n");

    close(oWritePipeFd); // close the write fd
    QCoreApplication::instance()->quit();
};


int main(int argc, char* argv[])
{
    QCoreApplication app{argc, argv};

    QTemporaryDir temporaryDir;
    QString productInstanceDirectory(argv[2]); // is the directory with result product instance

    auto config = GuiConfiguration::instance();
    config->setConfigFilePath(WeldmasterPaths::instance()->configurationDir() + QStringLiteral("uiSettings"));

    precitec::scheduler::ResultExcelFileFromProductInstance resultExcelFileFromProductInstance;

    resultExcelFileFromProductInstance.setProductStorageDirectory(WeldmasterPaths::instance()->productDir());
    resultExcelFileFromProductInstance.setResultDirectory(temporaryDir.path() + QDir::separator());
    resultExcelFileFromProductInstance.setResultProductInstanceDirectory(productInstanceDirectory);
    resultExcelFileFromProductInstance.extendedProductHelper()->setSerialNumberFromExtendedProductInfo(GuiConfiguration::instance()->serialNumberFromExtendedProductInfo() != 0);
    resultExcelFileFromProductInstance.extendedProductHelper()->setSerialNumberFromExtendedProductInfoField(GuiConfiguration::instance()->serialNumberFromExtendedProductInfo() - 1);

    QTimer::singleShot(0, &resultExcelFileFromProductInstance, &precitec::scheduler::ResultExcelFileFromProductInstance::createResultExcelFile);

    QObject::connect(&resultExcelFileFromProductInstance,
                     &precitec::scheduler::ResultExcelFileFromProductInstance::resultFileIsCreated,
                     [&]
                     {
                         upload(argc, argv, temporaryDir.path(), resultExcelFileFromProductInstance.resultFileName());
                     });
    QObject::connect(&resultExcelFileFromProductInstance, &precitec::scheduler::ResultExcelFileFromProductInstance::failed, &app, &QCoreApplication::quit);

    return app.exec();
}
