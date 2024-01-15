#pragma once

#include <QAbstractListModel>
#include <QDateTime>

#include <vector>

namespace precitec
{
namespace gui
{

/**
 * Model to support hardware configuration backups. It provides the functionality to create a
 * local backup of hardware configuration (VI_Config.xml, SystemConfig.xml) and to restore such
 * a backup.
 *
 * As a model it lists all the available previous backups.
 **/
class HardwareConfigurationBackupModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * Base directory where backups are located
     **/
    Q_PROPERTY(QString backupDir READ backupDir WRITE setBackupDir NOTIFY backupDirChanged)
    /**
     * The configuration directory from where the hardware configuration is copied
     **/
    Q_PROPERTY(QString configDir READ configDir WRITE setConfigDir NOTIFY configDirChanged)
public:
    HardwareConfigurationBackupModel(QObject *parent = nullptr);
    ~HardwareConfigurationBackupModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;

    QString backupDir() const
    {
        return m_backupDir;
    }
    void setBackupDir(const QString &backupDir);
    QString configDir() const
    {
        return m_configDir;
    }
    void setConfigDir(const QString &configDir);

    /**
     * Creates a new backup
     **/
    Q_INVOKABLE void createBackup();

    /**
     * Restores the backup identified by @p row.
     **/
    Q_INVOKABLE void restore(int row);

Q_SIGNALS:
    void backupDirChanged();
    void configDirChanged();
    void backupSucceeded();
    void backupFailed();
    void restoreSucceeded();
    void restoreFailed();

private:
    void init();
    QString m_backupDir;
    QString m_configDir;
    std::vector<QDateTime> m_backups;
};

}
}
