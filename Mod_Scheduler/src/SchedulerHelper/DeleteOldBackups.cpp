#include <unistd.h>
#include <iostream>

#include <backupModel.h>
#include <QCoreApplication>
#include <QDir>

#include "pipeLogger.h"

using namespace precitec::pipeLogger;
using precitec::gui::components::removableDevices::BackupModel;

int main(int argc, char* argv[])
{
    QCoreApplication app{argc, argv};
    int oWritePipeFd = atoi(argv[1]);
    std::string directoryWithAllBackups = argv[2];
    unsigned int timeToLiveDays = atoi(argv[3]);

    BackupModel backupModel;
    QObject::connect(&backupModel, &BackupModel::loadingChanged,
        [&]
        {
            if (backupModel.isLoading())
            {
                return;
            }
            std::string message = "Number of backup directories " + std::to_string(backupModel.rowCount()) + "\n";
            SendLoggerMessage(oWritePipeFd, precitec::eDebug, message.c_str());

            unsigned int numberOfDeletedBackupDirectories = 0;
            for (int i = 0; i < backupModel.rowCount(); i++)
            {
                const auto index{backupModel.index(i, 0)};
                if (index.data(Qt::DisplayRole).toDateTime().daysTo(QDateTime::currentDateTime()) > timeToLiveDays)
                {
                    QDir dir(index.data(Qt::UserRole + 1).toString());
                    dir.removeRecursively();
                    numberOfDeletedBackupDirectories++;
                }
            }

            message = "Number of deleted backup directories " + std::to_string(numberOfDeletedBackupDirectories) + "\n";
            SendLoggerMessage(oWritePipeFd, precitec::eDebug, message.c_str());

            close(oWritePipeFd); // close the write fd
            app.quit();
        }
    );
    backupModel.setProductName(QStringLiteral("weldmaster"));
    backupModel.setBackupPath(QString::fromStdString(directoryWithAllBackups));

    return app.exec();
}
