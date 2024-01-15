#include <QCoreApplication>
#include <QCommandLineParser>

#include "backupHelper.h"
#include "pipeLogger.h"

using precitec::scheduler::BackupHelper;

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};

    precitec::scheduler::BackupHelper helper{};
    QObject::connect(&helper, &BackupHelper::finished, &app, &QCoreApplication::quit);

    QCommandLineParser parser;
    QCommandLineOption backupPathOption{QStringLiteral("backupPath")};
    backupPathOption.setValueName(QStringLiteral("path"));
    parser.addOption(backupPathOption);

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
    helper.start(parser.value(backupPathOption));

    return app.exec();
}
