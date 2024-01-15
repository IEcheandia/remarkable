#pragma once
#include <QObject>
#include <QFileInfo>
#include <QUuid>
#include <QPointer>
#include <mutex>
#include "precitec/downloadService.h"

namespace precitec
{
namespace storage
{

class ProductInstanceDownloadHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

public:
    struct DownloadInstance {
        QFileInfo info;
        QUuid uuid;
        QString serialNumber;
        QPointer<precitec::gui::components::removableDevices::DownloadService> downloadService;
        QMetaObject::Connection connection;
    };
    ProductInstanceDownloadHelper(QObject * parent = nullptr);

    bool downloading() const
    {
        return m_downloading;
    }
    void setDownloading(bool value);

    void clear(); //clears queue, but doesn't stop downloads already started

    void download(std::vector<DownloadInstance>&& productInstances, QString stationName, QString productName, QString productDirectory,
                                         QString targetDir, QString subDir);

    void cancelDownload();
    void updateDownloadingStatus();

Q_SIGNALS:
    void downloadingChanged();
private:
    bool m_downloading = false;
    std::atomic<bool> m_downloadCanceled = false;
    std::mutex m_mutex;
    std::vector<DownloadInstance> m_downloadsInProgress;
};

}
}
