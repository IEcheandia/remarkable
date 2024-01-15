#include "hardwareConfigurationBackupModel.h"
#include "hardwareConfigurationBackupChangeEntry.h"
#include "hardwareConfigurationRestoreChangeEntry.h"
#include "etherCATConfigurationController.h"
#include "systemHardwareBackupHelper.h"
#include "upsModel.h"
#include "common/calibrationConfiguration.h"
#include <QDir>

#include <precitec/userLog.h>
#include <precitec/connectionController.h>
#include <precitec/devicesModel.h>
#include <precitec/ipv4SettingModel.h>

namespace precitec
{
namespace gui
{

using components::network::ConnectionController;
using components::network::DevicesModel;

static const QString s_format{QStringLiteral("yyyyMMdd-HHmm")};

HardwareConfigurationBackupModel::HardwareConfigurationBackupModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &HardwareConfigurationBackupModel::backupDirChanged, this, &HardwareConfigurationBackupModel::init);
    connect(this, &HardwareConfigurationBackupModel::backupSucceeded, this, &HardwareConfigurationBackupModel::init);
}

HardwareConfigurationBackupModel::~HardwareConfigurationBackupModel() = default;

int HardwareConfigurationBackupModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_backups.size();
}

QVariant HardwareConfigurationBackupModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    if (role == Qt::DisplayRole)
    {
        return m_backups.at(index.row());
    }
    return {};
}

void HardwareConfigurationBackupModel::createBackup()
{
    if (m_configDir.isEmpty() || m_backupDir.isEmpty())
    {
        emit backupFailed();
        return;
    }
    const QString subDirectoryName{QDateTime::currentDateTime().toString(s_format)};
    QDir backupDir{QDir{m_backupDir}.filePath(subDirectoryName)};
    if (!backupDir.mkpath(backupDir.absolutePath()))
    {
        emit backupFailed();
        return;
    }
    SystemHardwareBackupHelper helper;
    helper.setDirectory(m_configDir);
    helper.backup();

    const QDir configDir{m_configDir};
    static const std::vector<QString> filesToCopy{
        helper.backupFile(),
        QStringLiteral("SystemConfig.xml"),
        QStringLiteral("VI_Config.xml"),
        QStringLiteral("calibrationData0.xml")
    };
    for (const auto &file : filesToCopy)
    {
        if (!QFile::copy(configDir.filePath(file), backupDir.filePath(file)))
        {
            backupDir.removeRecursively();
            emit backupFailed();
            return;
        }
    }
    
    //files that are not present in every system
    auto copyFile = [&] (const QString &file) -> bool
    {
        if (QFile::exists(configDir.filePath(file))
                && !QFile::copy(configDir.filePath(file), backupDir.filePath(file)))
        {
            backupDir.removeRecursively();
            return false;
        }
        return true;
    };
    static const std::vector<QString> optionalFilesToCopy{
        QString::fromStdString(coordinates::CalibrationConfiguration::getCSVFallbackBasename(0)),
        QStringLiteral("correctionGrid0.csv"),
        QStringLiteral("IDM_correctionGrid0.csv"),
        QStringLiteral("OCT.xml"),
    };
    for (const auto &file : optionalFilesToCopy)
    {
        if (!copyFile(file))
        {
            emit backupFailed();
            return;
        }
    }

    static const QStringList optionalFilesToCopyPattern{
        QStringLiteral("camera*.xml")
    };
    const auto optionalFilesToCopyByPattern = configDir.entryInfoList(optionalFilesToCopyPattern, QDir::Files);
    for (const auto &file : optionalFilesToCopyByPattern)
    {
        if (!copyFile(file.fileName()))
        {
            emit backupFailed();
            return;
        }
    }
    
    precitec::gui::components::userLog::UserLog::instance()->addChange(new HardwareConfigurationBackupChangeEntry);
    emit backupSucceeded();
}

void HardwareConfigurationBackupModel::restore(int row)
{
    if (uint(row) >= m_backups.size())
    {
        emit restoreFailed();
        return;
    }
    if (m_configDir.isEmpty())
    {
        emit restoreFailed();
        return;
    }
    QDir configDir{m_configDir};
    if (!configDir.exists())
    {
        emit restoreFailed();
        return;
    }
    const QString backupDate{m_backups.at(row).toString(s_format)};
    QDir backup{QDir{m_backupDir}.filePath(backupDate)};
    if (!backup.exists())
    {
        emit restoreFailed();
        return;
    }
    precitec::gui::components::userLog::UserLog::instance()->addChange(new HardwareConfigurationRestoreChangeEntry{backupDate});
    QString suffix{QStringLiteral(".") + QDateTime::currentDateTime().toString(s_format) + QStringLiteral(".bak")};
    std::map<QString, QString> renamedFiles;
    auto revert = [&renamedFiles]
    {
        for (auto pair : renamedFiles)
        {
            if (!QFile::exists(pair.second))
            {
                continue;
            }
            QFile::remove(pair.first);
            QFile::rename(pair.second, pair.first);
        }
    };

    SystemHardwareBackupHelper helper;
    helper.setDirectory(m_configDir);
    bool systemHardwareBackupRestored = false;
    const auto backupFiles{backup.entryInfoList(QDir::Files | QDir::Readable)};
    for (auto backupFile : backupFiles)
    {
        if (configDir.exists(backupFile.fileName()))
        {
            // rename
            QString targetName{configDir.filePath(backupFile.fileName())};
            if (!QFile::rename(targetName, targetName +  suffix))
            {
                revert();
                emit restoreFailed();
                return;
            }
            renamedFiles.emplace(targetName, targetName + suffix);
        }
        if (!QFile::copy(backupFile.absoluteFilePath(), configDir.filePath(backupFile.fileName())))
        {
            revert();
            emit restoreFailed();
            return;
        }
        if (!systemHardwareBackupRestored && backupFile.fileName() == helper.backupFile())
        {
            systemHardwareBackupRestored = true;
        }
    }
    if (systemHardwareBackupRestored)
    {
        helper.load();
        helper.upsModel()->save();

        DevicesModel networkDevices;
        const auto etherCATIndex = networkDevices.indexForMacAddress(helper.etherCATMacAddress());

        EtherCATConfigurationController etherCAT;
        etherCAT.setEnabled(helper.isEtherCATEnabled());
        if (etherCATIndex.isValid() || helper.etherCATMacAddress().isEmpty())
        {
            etherCAT.setMacAddress(helper.etherCATMacAddress().toUtf8());
        }
        etherCAT.save();

        const auto uuids = helper.connectionUUIDS();
        for (const auto &uuid : uuids)
        {
            ConnectionController controller;
            controller.setUuid(uuid);
            auto connection = controller.connection();
            if (!connection)
            {
                continue;
            }
            const auto macAddress = helper.macAddressForConnection(uuid);
            auto index = networkDevices.indexForMacAddress(macAddress);
            if (!index.isValid() && !macAddress.isEmpty())
            {
                // ignore devices we do not have
                continue;
            }
            controller.updateMacAddress(macAddress);
            auto ipModel = helper.ipForConnection(uuid);
            ipModel->setConnection(connection);
            ipModel->save();
            controller.save();
        }
    }

    emit restoreSucceeded();
}

void HardwareConfigurationBackupModel::init()
{
    beginResetModel();
    m_backups.clear();
    if (!m_backupDir.isEmpty())
    {
        QDir dir{m_backupDir};
        if (dir.exists())
        {
            const auto backups = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (auto backup: backups)
            {
                const auto dateTime = QDateTime::fromString(backup, s_format);
                if (dateTime.isValid())
                {
                    m_backups.push_back(dateTime);
                }
            }
        }
    }
    endResetModel();
}

void HardwareConfigurationBackupModel::setBackupDir(const QString &backupDir)
{
    if (m_backupDir == backupDir)
    {
        return;
    }
    m_backupDir = backupDir;
    emit backupDirChanged();
}

void HardwareConfigurationBackupModel::setConfigDir(const QString &configDir)
{
    if (m_configDir == configDir)
    {
        return;
    }
    m_configDir = configDir;
    emit configDirChanged();
}

}
}
