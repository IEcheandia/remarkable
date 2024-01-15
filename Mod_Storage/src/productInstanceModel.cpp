#include "productInstanceModel.h"
#include "productInstanceSortModel.h"
#include "product.h"
#include "productMetaData.h"
#include "extendedProductInfoHelper.h"
#include "productInstanceDownloadHelper.h"
#include "precitec/downloadService.h"

#include <QDateTime>
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QtConcurrentRun>
#include <QtConcurrentFilter>
#include <QtConcurrentMap>
#include <QMutex>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QCoreApplication>

#include <forward_list>

using precitec::gui::components::removableDevices::DownloadService;

namespace precitec
{
namespace storage
{

ProductInstanceModel::ProductInstanceModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_mutex(std::make_unique<QMutex>())
    , m_filterModel(new ProductInstanceSortModel(this))
    , m_monitor(new QFileSystemWatcher{this})
    , m_extendedProductInfoHelper{new ExtendedProductInfoHelper{this}}
    , m_downloadHelper{new ProductInstanceDownloadHelper{this}}
{
    connect(this, &ProductInstanceModel::directoryChanged, this, &ProductInstanceModel::update);
    connect(this, &ProductInstanceModel::productChanged, this, &ProductInstanceModel::update);
    connect(this, &ProductInstanceModel::stationNameChanged, this,
        [this]
        {
            if (rowCount() == 0)
            {
                return;
            }
            emit dataChanged(index(0, 0), index(rowCount()-1, 0), {Qt::UserRole + 4});
        }
    );

    connect(this, &ProductInstanceModel::directoryChanged, this, &ProductInstanceModel::productDirectoryChanged);
    connect(this, &ProductInstanceModel::productChanged, this, &ProductInstanceModel::productDirectoryChanged);

    m_filterModel->setSourceModel(this);
    m_filterModel->setSortRole(Qt::UserRole + 1);

    connect(m_filterModel, &ProductInstanceSortModel::filterTypeChanged, this,
        [this]
        {
            if (m_filterModel->filterType() != ProductInstanceSortModel::FilterType::All)
            {
                ensureAllMetaDataLoaded();
            }
        }
    );

    connect(m_monitor, &QFileSystemWatcher::directoryChanged, this, [this]
        {
            if (!m_monitoring)
            {
                m_updatePending = true;
                return;
            }
            updateDirectory();
        }
    );

    connect(this, &ProductInstanceModel::monitoringChanged, this,
        [this]
        {
            if (!m_monitoring)
            {
                return;
            }
            if (m_updatePending)
            {
                m_updatePending = false;
                updateDirectory();
            }
        }
    );
    connect(this, &ProductInstanceModel::directoryChanged, this, &ProductInstanceModel::updateMonitor);
    connect(this, &ProductInstanceModel::productChanged, this, &ProductInstanceModel::updateMonitor);

    connect(m_downloadHelper, &ProductInstanceDownloadHelper::downloadingChanged, this, &ProductInstanceModel::downloadingChanged);
}

ProductInstanceModel::~ProductInstanceModel()
{
    if (m_loadWatcher)
    {
        m_loadWatcher->waitForFinished();
    }
    for (auto watcher : m_metaDataWatchers)
    {
        watcher->waitForFinished();
    }
}

QVariant ProductInstanceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant{};
    }
    const auto &info = m_productInstances.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return info.serialNumber;
    case Qt::UserRole:
        return QVariant::fromValue(info.info);
    case Qt::UserRole + 1:
        if (info.date == QDateTime{})
        {
            return info.lastModified;
        } else {
            return info.date;
        }
    case Qt::UserRole + 2:
        return QVariant::fromValue(info.nio);
    case Qt::UserRole + 3:
        return info.nioSwitchedOff;
    case Qt::UserRole + 4:
        {
            auto path = QDir{m_directory}.relativeFilePath(info.info.filePath());
            path = path.mid(path.indexOf(QLatin1String("/")));
            QString prefix = m_stationName;
            if (!prefix.isEmpty())
            {
                prefix.append(QLatin1String("/"));
            }
            return prefix + m_product->name() + path;
        }
    case Qt::UserRole + 5:
        return QVariant::fromValue(info.downloadService);
    case Qt::UserRole + 6:
        return info.info.absoluteFilePath();
    case Qt::UserRole + 7:
        return info.checked;
    case Qt::UserRole + 8:
        return info.info.fileName();
    case Qt::UserRole + 9:
        return info.uuid;
    case Qt::UserRole + 11:
        return info.partNumber;
    }
    return QVariant{};
}

int ProductInstanceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_productInstances.size();
}

QHash<int, QByteArray> ProductInstanceModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("fileInfo")},
        {Qt::UserRole + 1, QByteArrayLiteral("date")},
        {Qt::UserRole + 2, QByteArrayLiteral("nio")},
        {Qt::UserRole + 3, QByteArrayLiteral("nioResultsSwitchedOff")},
        {Qt::UserRole + 4, QByteArrayLiteral("relativePath")},
        {Qt::UserRole + 5, QByteArrayLiteral("downloadService")},
        {Qt::UserRole + 6, QByteArrayLiteral("path")},
        {Qt::UserRole + 7, QByteArrayLiteral("checked")},
        {Qt::UserRole + 8, QByteArrayLiteral("directoryName")},
        {Qt::UserRole + 9, QByteArrayLiteral("uuid")},
        // skipping 10 to have same number as ProductInstanceTableModel
        {Qt::UserRole + 11, QByteArrayLiteral("partNumber")},
    };
}

void ProductInstanceModel::setDirectory(const QString &directory)
{
    if (m_directory == directory)
    {
        return;
    }
    m_directory = directory;
    emit directoryChanged();
}

void ProductInstanceModel::setProduct(Product *product)
{
    if (m_product == product)
    {
        return;
    }
    disconnect(m_productDestroyedConnection);
    m_product = product;
    if (m_product)
    {
        m_productDestroyedConnection = connect(m_product, &QObject::destroyed, this,
            [this]
            {
                m_product = nullptr;
                emit productChanged();
            }
        );
    }

    emit productChanged();
}

void ProductInstanceModel::update()
{
    clear();
    if (!m_product || m_directory.isEmpty())
    {
        // nothing to do
        return;
    }
    // start iterating
    updateDirectory();
}

void ProductInstanceModel::updateDirectory()
{
    if (isLoading() || !m_product || m_preparingDownload)
    {
        m_updatePending = true;
        return;
    }
    m_updatePending = false;
    setLoading(true);
    m_loadWatcher = new QFutureWatcher<std::pair<QList<std::tuple<QFileInfo, QUuid, QString, QDateTime>>, std::forward_list<QFileInfo>>>{this};
    connect(m_loadWatcher, &QFutureWatcher<std::pair<QList<std::tuple<QFileInfo, QUuid, QString, QDateTime>>, std::forward_list<QFileInfo>>>::finished, this,
        [this]
        {
            m_loadWatcher->deleteLater();
            auto results = m_loadWatcher->result();
            m_loadWatcher = nullptr;
            auto it = results.first.begin();
            while (it != results.first.end())
            {
                if (std::get<1>(*it).isNull())
                {
                    it = results.first.erase(it);
                } else
                {
                    it++;
                }
            }
            removeInstances(results.second);
            addInstances(results.first);
            setLoading(false);
            if (m_updatePending)
            {
                updateDirectory();
            }
        }
    );
    m_loadWatcher->setFuture(QtConcurrent::run(this, &ProductInstanceModel::load, subDirectory()));
}

QString ProductInstanceModel::subDirectory() const
{
    QString subdir;
    switch (m_productDirectoryName)
    {
    case ProductDirectoryName::Uuid:
        subdir = m_product->uuid().toString(QUuid::WithoutBraces);
        break;
    case ProductDirectoryName::ProductName:
        subdir = m_product->name();
        break;
    }
    QString result = m_directory;
    if (!result.endsWith(QDir::separator()))
    {
        result.append(QDir::separator());
    }
    result.append(subdir);
    return result;
}

void ProductInstanceModel::clear()
{
    QMutexLocker lock{m_mutex.get()};
    beginResetModel();
    for (const auto &productInstance : m_productInstances)
    {
        auto downloadService = productInstance.downloadService;
        disconnect(productInstance.connection);
        if (downloadService == nullptr)
        {
            continue;
        }
        if (!downloadService->isBackupInProgress())
        {
            downloadService->deleteLater();
        }
        else
        {
            connect(downloadService, &DownloadService::finished, downloadService, &DownloadService::deleteLater);
        }
    }
    m_downloadHelper->clear();
    m_productInstances.clear();
    endResetModel();
}

bool ProductInstanceModel::isLoading() const
{
    return m_loading;
}

void ProductInstanceModel::setLoading(bool set)
{
    if (m_loading == set)
    {
        return;
    }
    m_loading = set;
    emit loadingChanged();
}

void ProductInstanceModel::addInstance(const QFileInfo &info, const QUuid &uuid)
{
    // requires mutex to be locked
    auto it = std::find_if(m_productInstances.begin(), m_productInstances.end(), [uuid] (const auto &instance) { return instance.uuid == uuid; });
    if (it == m_productInstances.end())
    {
        return;
    }
    if ((*it).loadingState != ProductInstace::MetaDataLoadingState::NotLoaded)
    {
        return;
    }
    (*it).loadingState = ProductInstace::MetaDataLoadingState::Loading;

    auto *watcher = new QFutureWatcher<ProductMetaData>(this);
    connect(watcher, &QFutureWatcher<ProductMetaData>::finished, this,
        [this, uuid, watcher]
        {
            watcher->deleteLater();
            if (auto it = std::find(m_metaDataWatchers.begin(), m_metaDataWatchers.end(), watcher); it != m_metaDataWatchers.end())
            {
                m_metaDataWatchers.erase(it);
            }
            QMutexLocker lock{m_mutex.get()};
            auto it = std::find_if(m_productInstances.begin(), m_productInstances.end(), [uuid] (const auto &instance) { return instance.uuid == uuid; });
            if (it == m_productInstances.end())
            {
                return;
            }
            (*it).loadingState = ProductInstace::MetaDataLoadingState::Loaded;
            const auto metaData = watcher->result();
            QVector<int> changedRoles;
            if (metaData.isDateValid())
            {
                if (metaData.date() != it->date)
                {
                    it->date = metaData.date();
                    changedRoles << Qt::UserRole + 1;
                }
            }
            if (metaData.isNioValid())
            {
                const auto nio = metaData.nio() ? State::Nio : State::Io;
                if (nio != it->nio)
                {
                    it->nio = nio;
                    changedRoles << Qt::UserRole + 2;
                }
            }
            if (metaData.isNioSwitchedOffValid())
            {
                if (metaData.nioSwitchedOff() != it->nioSwitchedOff)
                {
                    it->nioSwitchedOff = metaData.nioSwitchedOff();
                    changedRoles << Qt::UserRole + 3;
                }
            }
            if (const auto serialNumber{m_extendedProductInfoHelper->serialNumber(metaData.extendedProductInfo())})
            {
                it->serialNumber = serialNumber.value();
                changedRoles << Qt::DisplayRole;
            }
            if (const auto partNumber{m_extendedProductInfoHelper->partNumber(metaData.extendedProductInfo())})
            {
                it->partNumber = partNumber.value();
                changedRoles << Qt::UserRole + 11;
            }
            if (!changedRoles.empty())
            {
                const QModelIndex idx = index(std::distance(m_productInstances.begin(), it), 0);
                emit dataChanged(idx, idx, changedRoles);
            }
        }
    );

    m_metaDataWatchers.push_back(watcher);
    watcher->setFuture(QtConcurrent::run([info]
        {
            return ProductMetaData::parse(info);
        }
    ));
}

void ProductInstanceModel::addInstances(const QList<std::tuple<QFileInfo, QUuid, QString, QDateTime>> &instances)
{
    if (instances.empty())
    {
        return;
    }
    QMutexLocker lock{m_mutex.get()};
    beginInsertRows(QModelIndex(), m_productInstances.size(), m_productInstances.size() + instances.size() - 1);
    for (const auto &instance : instances)
    {
        const auto &info = std::get<QFileInfo>(instance);
        const auto &uuid = std::get<QUuid>(instance);
        const auto &serialNumber = std::get<QString>(instance);
        const auto &lastModified = std::get<QDateTime>(instance);
        m_productInstances.emplace_back(ProductInstace{info, uuid, serialNumber, lastModified});

        auto& productInstance = m_productInstances.back();

        productInstance.downloadService = new DownloadService{this};
        productInstance.downloadService->setProductName(QCoreApplication::applicationName());
        productInstance.downloadService->setValidate(false);
        productInstance.downloadService->setCompactBackupStatus(true);

        const auto uncheckHandler = [this] (const QUuid id) {
            auto it = std::find_if(m_productInstances.begin(), m_productInstances.end(), [&id] (const auto& instance) { return instance.uuid == id; });

            if (it == m_productInstances.end())
            {
                return;
            }

            //uncheck the item if the download is done
            if (!it->downloadService->isBackupInProgress())
            {
                it->checked = false;
                const auto idx = std::distance(m_productInstances.begin(), it);
                emit dataChanged(index(idx, 0), index(idx, 0), {Qt::UserRole + 7});
            }
        };

        productInstance.connection = connect(productInstance.downloadService, &DownloadService::finished, this, std::bind(uncheckHandler, productInstance.uuid));
    }
    endInsertRows();
    if (m_filterModel->filterType() == ProductInstanceSortModel::FilterType::OnlyNIO)
    {
        for (const auto &instance : instances)
        {
            const auto &info = std::get<QFileInfo>(instance);
            const auto &uuid = std::get<QUuid>(instance);
            addInstance(info, uuid);
        }
    }
}

void ProductInstanceModel::removeInstances(const std::forward_list<QFileInfo> &instances)
{
    if (instances.empty())
    {
        return;
    }
    QMutexLocker lock{m_mutex.get()};
    for (const auto &file : instances)
    {
        auto it = std::find_if(m_productInstances.begin(), m_productInstances.end(), [&file] (const auto &instance) { return instance.info == file; });
        if (it == m_productInstances.end())
        {
            continue;
        }
        const int row = int(std::distance(m_productInstances.begin(), it));
        beginRemoveRows({}, row, row);
        m_productInstances.erase(it);
        endRemoveRows();
    }
}

void ProductInstanceModel::ensureMetaDataLoaded(int idx)
{
    const auto& index = sourceIndex(idx);
    if (!index.isValid())
    {
        return;
    }
    QMutexLocker lock{m_mutex.get()};
    const auto &productInstance = m_productInstances.at(index.row());
    if (productInstance.loadingState == ProductInstace::MetaDataLoadingState::NotLoaded)
    {
        addInstance(productInstance.info, productInstance.uuid);
    }
}

void ProductInstanceModel::ensureAllMetaDataLoaded()
{
    QMutexLocker lock{m_mutex.get()};
    for (const auto &productInstance : m_productInstances)
    {
        addInstance(productInstance.info, productInstance.uuid);
    }
}

std::pair<QList<std::tuple<QFileInfo, QUuid, QString, QDateTime>>, std::forward_list<QFileInfo>> ProductInstanceModel::load(const QString& path)
{
    std::function<std::tuple<QFileInfo, QUuid, QString, QDateTime> (const QFileInfo &info)> mapped = [] (const QFileInfo &info)
        {
            QRegularExpression exp{QStringLiteral("(.+)-SN-(.+)")};
            auto match = exp.match(info.fileName());
            if (!match.hasMatch())
            {
                return std::make_tuple(info, QUuid{}, QString{}, QDateTime{});
            }
            const QString uuidPart = match.captured(1);
            const QString serialNumberPart = match.captured(2);
            const QUuid uuid{uuidPart};
            return std::make_tuple(info, uuid, serialNumberPart, info.lastModified());
        };

    QMutexLocker lock{m_mutex.get()};
    QDirIterator it{path, QDir::NoDotAndDotDot | QDir::Dirs};
    std::forward_list<QFileInfo> existingFiles;
    std::forward_list<QFileInfo> filesToDelete;
    QList<QFileInfo> filteredInfos;
    while (it.hasNext())
    {
        it.next();
        const auto &info = it.fileInfo();
        if (!info.isDir())
        {
            continue;
        }
        existingFiles.emplace_front(info);
        if (std::none_of(m_productInstances.cbegin(), m_productInstances.cend(), [&info] (const auto &instance) { return instance.info == info; }))
        {
            filteredInfos << info;
        }
    }
    // find all files which are no longer present in the directory
    for (const auto &instance : m_productInstances)
    {
        if (std::none_of(existingFiles.cbegin(), existingFiles.cend(), [&instance] (const auto &file) { return file == instance.info; }))
        {
            filesToDelete.emplace_front(instance.info);
        }
    }
    lock.unlock();

    // now map the the results by applying regular expression in parallel
    return std::make_pair(QtConcurrent::blockingMapped(filteredInfos, mapped), filesToDelete);
}

void ProductInstanceModel::setProductDirectoryName(ProductDirectoryName directoryName)
{
    if (m_productDirectoryName == directoryName)
    {
        return;
    }
    m_productDirectoryName = directoryName;
    emit productDirectoryNameChanged();
}

void ProductInstanceModel::setMonitoring(bool monitor)
{
    if (m_monitoring == monitor)
    {
        return;
    }
    m_monitoring = monitor;
    emit monitoringChanged();
}

void ProductInstanceModel::updateMonitor()
{
    // first remove all existing paths
    const auto directories = m_monitor->directories();
    if (!directories.isEmpty())
    {
        m_monitor->removePaths(directories);
    }
    if (m_directory.isEmpty() || !m_product)
    {
        return;
    }
    // now add new paths
    QDir{}.mkpath(subDirectory());
    m_monitor->addPath(subDirectory());
}

void ProductInstanceModel::setStationName(const QString &stationName)
{
    if (m_stationName == stationName)
    {
        return;
    }
    m_stationName = stationName;
    emit stationNameChanged();
}

void ProductInstanceModel::toggleChecked(int idx)
{
    const auto& index = sourceIndex(idx);
    if (!index.isValid())
    {
        return;
    }
    m_productInstances.at(index.row()).checked = !m_productInstances.at(index.row()).checked;
    emit dataChanged(index, index, {Qt::UserRole + 7});
}

void ProductInstanceModel::checkAll()
{
    if (m_productInstances.empty())
    {
        return;
    }

    for (auto i = 0; i < m_filterModel->rowCount(); i++)
    {
        auto index = m_filterModel->mapToSource(m_filterModel->index(i, 0));
        if (!index.isValid())
        {
            continue;
        }
        m_productInstances.at(index.row()).checked = true;
    }
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::UserRole + 7});
}

void ProductInstanceModel::uncheckAll()
{
    if (m_productInstances.empty())
    {
        return;
    }
    for (auto i = 0; i < m_filterModel->rowCount(); i++)
    {
        auto index = m_filterModel->mapToSource(m_filterModel->index(i, 0));
        if (!index.isValid())
        {
            continue;
        }
        m_productInstances.at(index.row()).checked = false;
    }
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::UserRole + 7});
}

QModelIndex ProductInstanceModel::sourceIndex(int idx) const
{
    return m_filterModel->mapToSource(m_filterModel->index(idx, 0));
}

void ProductInstanceModel::setPreparingDownload(bool value)
{
    if (m_preparingDownload == value)
    {
        return;
    }
    m_preparingDownload = value;
    emit preparingDownloadChanged();
}

void ProductInstanceModel::downloadChecked(QString targetDir, QString subDir)
{
    setPreparingDownload(true);
    QtConcurrent::run([this, targetDir, subDir]()
    {
        std::vector<ProductInstanceDownloadHelper::DownloadInstance> downloads;
        for (auto& productInstance : m_productInstances)
        {
            if (productInstance.checked)
            {
                downloads.push_back({productInstance.info, productInstance.uuid, productInstance.serialNumber, productInstance.downloadService});
            }
        }
        m_downloadHelper->download(std::move(downloads), m_stationName, m_product->name(), m_directory,
                                                targetDir, subDir);
    });
    setPreparingDownload(false);
}

bool ProductInstanceModel::downloading() const
{
    return m_downloadHelper->downloading();
}

void ProductInstanceModel::cancelDownload()
{
    m_downloadHelper->cancelDownload();
 }

}
}
