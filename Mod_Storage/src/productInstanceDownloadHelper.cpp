#include "productInstanceDownloadHelper.h"
#include "precitec/downloadService.h"
#include <QtConcurrent>
#include <QDebug>

namespace precitec
{
namespace storage
{
using gui::components::removableDevices::DownloadService;
ProductInstanceDownloadHelper::ProductInstanceDownloadHelper(QObject* parent)
    : QObject(parent)
{
}

void ProductInstanceDownloadHelper::download(std::vector<DownloadInstance>&& productInstances, QString stationName, QString productName, QString productDirectory,
                                                        QString targetDir, QString subDir)
{
    std::unique_lock<std::mutex> lock{m_mutex};
    setDownloading(true);

    for (auto&& productInstance : productInstances)
    {
        m_downloadsInProgress.push_back(std::move(productInstance));
    }

    auto toDirPath = [](const QString& name)
    {
        if (!name.isEmpty() && !name.endsWith("/"))
        {
            return name + "/";
        }
        return name;
    };

    QString targetPath = toDirPath(subDir) + toDirPath(stationName) + toDirPath(productName);

    for (auto& productInstance : m_downloadsInProgress)
    {
        auto downloadService = productInstance.downloadService;
        if (m_downloadCanceled)
        {
            downloadService.clear();
        }
        if (downloadService == nullptr)
        {
            continue;
        }
        downloadService->setBackupPath(targetDir);
        QString productInstancePath = QDir{productDirectory}.relativeFilePath(productInstance.info.filePath());
        auto productInstanceFolder = productInstancePath.mid(productInstancePath.indexOf(QLatin1String("/")) + 1);
        productInstance.connection = connect(productInstance.downloadService, &DownloadService::finished,
                                                this, &ProductInstanceDownloadHelper::updateDownloadingStatus);

        downloadService->performDownload(productInstance.info.filePath(),
                                            targetPath + productInstanceFolder,
                                            tr("Download %1 of %2 to attached removable device").arg(productInstance.serialNumber).arg(productName));
    }
}



void ProductInstanceDownloadHelper::updateDownloadingStatus()
{
    if (m_downloadsInProgress.empty() 
        || std::all_of(m_downloadsInProgress.begin(), m_downloadsInProgress.end(),
                    [](const auto& p){ return  p.downloadService.isNull() || !p.downloadService->isBackupInProgress(); }))
    {
        setDownloading(false);
        m_downloadCanceled = false;
        if (!m_downloadsInProgress.empty())
        {
            m_downloadsInProgress.clear();
        }
    }
}


void ProductInstanceDownloadHelper::setDownloading(bool value)
{
    if (value == m_downloading)
    {
        return;
    }
    m_downloading = value;
    emit downloadingChanged();
}


void ProductInstanceDownloadHelper::clear()
{
    {
        std::unique_lock<std::mutex> lock{m_mutex};
        for (auto & instance : m_downloadsInProgress)
        {
            if (instance.downloadService)
            {
                disconnect(instance.connection);
            }
        }
        m_downloadsInProgress.clear(); //the download jobs are not interrupted, but the download property is correct only as long the model has not been changed
    } //release lock
    updateDownloadingStatus();
}

void ProductInstanceDownloadHelper::cancelDownload()
{
    m_downloadCanceled = true; //the downloads not yet started will be skipped
    for (const auto &productInstance : m_downloadsInProgress)
    {
        if (auto downloadService = productInstance.downloadService)
        {
            downloadService->cancel(); //this signal has effect only on the running downloads
        }
    }

}

}
}
