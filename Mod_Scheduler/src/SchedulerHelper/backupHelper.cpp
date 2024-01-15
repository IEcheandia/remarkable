#include "backupHelper.h"
#include "pipeLogger.h"

#include "../../App_Gui/plugins/general/guiConfiguration.h"
#include "../../App_Gui/plugins/general/weldmasterPaths.h"
#include "../../App_Gui/plugins/configuration/systemHardwareBackupHelper.h"

#include <precitec/backupService.h>
#include <precitec/dateNamedDirectoryModel.h>

#include <QCommandLineParser>
#include <QStandardPaths>
#include <QTimer>

using namespace precitec::gui::components::removableDevices;
using precitec::gui::WeldmasterPaths;
using precitec::gui::GuiConfiguration;
using precitec::gui::SystemHardwareBackupHelper;

namespace precitec
{
namespace scheduler
{

BackupHelper::BackupHelper(QObject* parent)
    : QObject(parent)
    , m_service(new BackupService{this})
{
    connect(m_service, &BackupService::backupInProgressChanged,
        [this]
        {
            if (!m_service->isBackupInProgress())
            {
                const auto &errorMessages = m_service->errorMessages();
                if (errorMessages.isEmpty())
                {
                    precitec::pipeLogger::SendLoggerMessage(m_fd, precitec::eInfo, "Scheduled backup performed successfully\n");
                    emit succeeded();
                }
                else
                {
                    precitec::pipeLogger::SendLoggerMessage(m_fd, precitec::eError, "Scheduled backup failed!\n");
                    for (const auto &message: errorMessages)
                    {
                        precitec::pipeLogger::SendLoggerMessage(m_fd, precitec::eDebug, message.toUtf8().append(QByteArrayLiteral("\n").constData()));
                    }
                    emit failed();
                }
                emit finished();
            }
        });
}

BackupHelper::~BackupHelper() = default;

void BackupHelper::initCommandLineParser(QCommandLineParser& parser)
{
    QCommandLineOption configOption{QStringLiteral("config")};
    parser.addOption(configOption);
    QCommandLineOption logsOption{QStringLiteral("logs")};
    parser.addOption(logsOption);
    QCommandLineOption screenshotsOption{QStringLiteral("screenshots")};
    parser.addOption(screenshotsOption);
    QCommandLineOption softwareOption{QStringLiteral("software")};
    parser.addOption(softwareOption);
}

void BackupHelper::initBackupService(const QCommandLineParser& parser)
{
    auto config = GuiConfiguration::instance();
    config->setConfigFilePath(WeldmasterPaths::instance()->configurationDir() + QStringLiteral("uiSettings"));

    m_service->setProductName(QStringLiteral("weldmaster"));
    m_service->setStationId(config->stationId());
    m_service->setStationName(config->stationName());
    m_service->setValidate(false);

    if (parser.isSet(QStringLiteral("config")))
    {
        SystemHardwareBackupHelper helper;
        helper.setDirectory(WeldmasterPaths::instance()->configurationDir());
        helper.backup();
        m_service->addBackupDirectory(QStringLiteral("config"), WeldmasterPaths::instance()->configurationDir());
    }
    if (parser.isSet(QStringLiteral("logs")))
    {
        m_service->addBackupDirectory(QStringLiteral("logs"), WeldmasterPaths::instance()->logfilesDir());
    }
    if (parser.isSet(QStringLiteral("screenshots")))
    {
        m_service->addBackupDirectory(QStringLiteral("screenshots"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    }
    if (parser.isSet(QStringLiteral("software")))
    {
        DateNamedDirectoryModel archiveModel;
        archiveModel.setPath(WeldmasterPaths::instance()->updateArchiveDir());
        if (archiveModel.rowCount() > 0)
        {
            m_service->addBackupDirectory(QStringLiteral("software"), archiveModel.data(archiveModel.index(0, 0), Qt::UserRole + 1).toString());
        }
    }
}

void BackupHelper::start(const QString& backupPath)
{
    m_service->setBackupPath(backupPath);

    QTimer::singleShot(0, m_service, &BackupService::performBackup);
}


}
}
