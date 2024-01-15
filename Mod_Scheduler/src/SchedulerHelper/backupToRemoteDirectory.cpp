#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QTemporaryDir>

#include <precitec/backupModel.h>

#include "commandLineParser.h"
#include "backupHelper.h"
#include "fileUpload.h"
#include "pipeLogger.h"

using precitec::scheduler::BackupHelper;
using precitec::gui::components::removableDevices::BackupModel;

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};

    precitec::scheduler::BackupHelper helper{};
    QObject::connect(&helper, &BackupHelper::failed, &app, &QCoreApplication::quit);

    QCommandLineParser parser;
    QCommandLineOption ipOption{QStringLiteral("ip")};
    ipOption.setValueName(QStringLiteral("address"));
    parser.addOption(ipOption);
    QCommandLineOption portOption{QStringLiteral("port")};
    portOption.setValueName(QStringLiteral("port"));
    portOption.setDefaultValue(QStringLiteral("22"));
    parser.addOption(portOption);
    QCommandLineOption userNameOption{QStringLiteral("user")};
    userNameOption.setValueName(QStringLiteral("name"));
    parser.addOption(userNameOption);
    QCommandLineOption passwordOption{QStringLiteral("password")};
    passwordOption.setValueName(QStringLiteral("password"));
    parser.addOption(passwordOption);
    QCommandLineOption remotePathOption{QStringLiteral("remotePath")};
    remotePathOption.setValueName(QStringLiteral("path"));
    parser.addOption(remotePathOption);
    QCommandLineOption debugOption{QStringLiteral("debug")};
    parser.addOption(debugOption);
    QCommandLineOption protocolOption{QStringLiteral("protocol")};
    protocolOption.setValueName(QStringLiteral("protocol"));
    protocolOption.setDefaultValue(QStringLiteral("sftp"));
    parser.addOption(protocolOption);
    QCommandLineOption httpMethodOption{QStringLiteral("httpMethod")};
    httpMethodOption.setValueName(QStringLiteral("method"));
    httpMethodOption.setDefaultValue(QStringLiteral("POST"));
    parser.addOption(httpMethodOption);

    helper.initCommandLineParser(parser);

    parser.process(app);

    // find pipe
    int fd = -1;
    const auto arguments = parser.positionalArguments();
    if (!arguments.isEmpty())
    {
        bool ok = false;
        fd = arguments.at(0).toInt(&ok);
        if (!ok)
        {
            fd = -1;
        }
    }
    helper.setLogFd(fd);

    precitec::pipeLogger::SendLoggerMessage(fd, precitec::eInfo, "Starting scheduled backup\n");

    helper.initBackupService(parser);
    QTemporaryDir dir{};
    if (!dir.isValid())
    {
        precitec::pipeLogger::SendLoggerMessage(fd, precitec::eError, "Failed to create temporary directory for backup\n");
        return 0;
    }

    precitec::scheduler::FileUpload fileUpload{};
    precitec::scheduler::parseCommandLine(argc, argv, fileUpload);

    fileUpload.SetWritePipeFd(fd);
    fileUpload.SetDebugFileName("StdErr_Of_TransferDirectory.txt");

    QObject::connect(&helper, &BackupHelper::succeeded, &app,
        [&dir, fd, &app, &fileUpload]
        {
            auto model = new BackupModel{&app};
            QObject::connect(model, &BackupModel::loadingChanged,
                    [model, fd, &fileUpload]
                    {
                        if (model->isLoading())
                        {
                            return;
                        }
                        if (model->rowCount() != 1)
                        {
                            precitec::pipeLogger::SendLoggerMessage(fd, precitec::eError, "Did not find the backup\n");
                            return;
                        }
                        const auto directory = model->index(0, 0).data(Qt::UserRole).value<QFileInfo>();

                        precitec::pipeLogger::SendLoggerMessage(fd, precitec::eDebug, "Start of TransferDirectory\n");

                        fileUpload.SetSourceDirectory(directory.dir().path().toStdString());
                        fileUpload.SetDirectoryToSend(directory.fileName().toStdString());

                        fileUpload.SetTargetDirectoryName(directory.fileName().toStdString());

                        fileUpload.UploadDirectory();

                        {
                            std::stringstream oLoggerText{};
                            oLoggerText << "TransferDirectory: NumberOfTransmittedFiles: " << fileUpload.GetNumberOfTransmittedFiles() << std::endl;
                            precitec::pipeLogger::SendLoggerMessage(fd, precitec::eInfo, oLoggerText.str().c_str());
                        }
                        {
                            std::stringstream oLoggerText{};
                            oLoggerText << "TransferDirectory: TotalTransmittedBytes:    " << fileUpload.GetTotalTransmittedBytes() << std::endl;
                            precitec::pipeLogger::SendLoggerMessage(fd, precitec::eInfo, oLoggerText.str().c_str());
                        }
                        {
                            std::stringstream oLoggerText{};
                            oLoggerText << "TransferDirectory: DurationOfTransmissionMS: " << fileUpload.GetDurationOfTransmissionMS() << std::endl;
                            precitec::pipeLogger::SendLoggerMessage(fd, precitec::eInfo, oLoggerText.str().c_str());
                        }
                        precitec::pipeLogger::SendLoggerMessage(fd, precitec::eDebug, "End of TransferDirectory\n");

                        QCoreApplication::instance()->quit();
                    });
            model->setPath(dir.path());
            model->setProductName(QStringLiteral("weldmaster"));

        });

    helper.start(dir.path());

    return app.exec();
}
