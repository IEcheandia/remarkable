#pragma once

#include <QObject>

class QCommandLineParser;

namespace precitec
{

namespace gui
{
namespace components
{
namespace removableDevices
{
class BackupService;
}
}
}

namespace scheduler
{

class BackupHelper : public QObject
{
    Q_OBJECT
public:
    BackupHelper(QObject *parent = nullptr);
    ~BackupHelper() override;

    void initCommandLineParser(QCommandLineParser &parser);
    void initBackupService(const QCommandLineParser &parser);

    void start(const QString &backupPath);

    void setLogFd(int fd)
    {
        m_fd = fd;
    }

Q_SIGNALS:
    void succeeded();
    void failed();
    void finished();

private:
    precitec::gui::components::removableDevices::BackupService *m_service;
    int m_fd = -1;
};

}
}
