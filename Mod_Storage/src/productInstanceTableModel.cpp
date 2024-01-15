#include "productInstanceTableModel.h"
#include "product.h"
#include "productMetaData.h"
#include "seam.h"
#include "seamSeries.h"
#include "extendedProductInfoHelper.h"
#include "productInstanceDownloadHelper.h"
#include "precitec/downloadService.h"

#include <QFileSystemWatcher>
#include <QRegularExpression>
#include <QtConcurrentRun>
#include <QtConcurrentMap>
#include <QFutureWatcher>
#include <QDirIterator>
#include <QDir>
#include <QCoreApplication>

using precitec::gui::components::removableDevices::DownloadService;

namespace precitec
{
namespace storage
{

ProductInstanceTableModel::ProductInstanceTableModel(QObject* parent )
    : QAbstractTableModel(parent)
    , m_monitor(new QFileSystemWatcher{this})
    , m_extendedProductInfoHelper{new ExtendedProductInfoHelper{this}}
    , m_downloadHelper{new ProductInstanceDownloadHelper{this}}
{
    connect(this, &ProductInstanceTableModel::directoryChanged, this, &ProductInstanceTableModel::update);
    connect(this, &ProductInstanceTableModel::productChanged, this, &ProductInstanceTableModel::loadSeams);
    connect(this, &ProductInstanceTableModel::seamsChanged, this, &ProductInstanceTableModel::update);
    connect(this, &ProductInstanceTableModel::directoryChanged, this, &ProductInstanceTableModel::updateMonitor);
    connect(this, &ProductInstanceTableModel::productChanged, this, &ProductInstanceTableModel::updateMonitor);

    connect(this, &ProductInstanceTableModel::stationNameChanged, this,
        [this]
        {
            if (rowCount() == 0)
            {
                return;
            }
            emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::UserRole + 4});
        }
    );

    connect(m_monitor, &QFileSystemWatcher::directoryChanged, this, [this]
        {
            if (!m_monitoring)
            {
                m_updatePending = true;
                return;
            }
            updateModelData();
        }
    );

    connect(this, &ProductInstanceTableModel::monitoringChanged, this,
        [this]
        {
            if (!m_monitoring)
            {
                return;
            }
            if (m_updatePending)
            {
                m_updatePending = false;
                updateModelData();
            }
        }
    );
    connect(m_downloadHelper, &ProductInstanceDownloadHelper::downloadingChanged, this, &ProductInstanceTableModel::downloadingChanged);
}

ProductInstanceTableModel::~ProductInstanceTableModel() = default;

QHash<int, QByteArray> ProductInstanceTableModel::roleNames() const
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
        {Qt::UserRole + 9, QByteArrayLiteral("seamSeries")},
        {Qt::UserRole + 10, QByteArrayLiteral("seam")},
        {Qt::UserRole + 11, QByteArrayLiteral("partNumber")},
    };
}

QVariant ProductInstanceTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= int(m_productInstances.size()))
    {
        return {};
    }

    const auto& info = m_productInstances.at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        return info.serialNumber;
    case Qt::UserRole:
        return QVariant::fromValue(info.info);
    case Qt::UserRole + 1:
        if (info.date.isNull())
        {
            return info.lastModified;
        } else
        {
            return info.date;
        }
    case Qt::UserRole + 2:
        if (index.column() == 0)
        {
            return QVariant::fromValue(info.nio);
        }
        if (index.column() - 1 >= int(m_productSeams.size()))
        {
            return {};
        }
        if (auto seam = m_productSeams.at(index.column() - 1))
        {
            auto it = info.seamNio.find(seam->uuid());

            if (it != info.seamNio.end())
            {
                return QVariant::fromValue((*it).second);
            }
            else
            {
                return QVariant::fromValue(State::Unknown);
            }
        }
        return {};

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
    {
        if (index.column() > 0 && index.column() - 1 <= int(m_productSeams.size()))
        {
            return QVariant::fromValue(m_productSeams.at(index.column() - 1)->seamSeries());
        }
        return QVariant::fromValue(nullptr);
    }
    case Qt::UserRole + 10:
    {
        if (index.column() > 0 && index.column() - 1 <= int(m_productSeams.size()))
        {
            return QVariant::fromValue(m_productSeams.at(index.column() - 1));
        }
        return QVariant::fromValue(nullptr);
    }
    case Qt::UserRole + 11:
        return info.partNumber;
    }

    return {};
}

bool ProductInstanceTableModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (!idx.isValid() || idx.row() >= int(m_productInstances.size()) || role != Qt::UserRole + 7)
    {
        return false;
    }

    auto& info = m_productInstances.at(idx.row());

    if (role == Qt::UserRole + 7)
    {
        info.checked = value.toBool();

        emit dataChanged(index(idx.row(), 0), index(idx.row(), 0), {Qt::UserRole + 7});
    }

    return true;
}

int ProductInstanceTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_productInstances.size();
}

int ProductInstanceTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 1;
    }
    return 1 + m_productSeams.size();
}

Qt::ItemFlags ProductInstanceTableModel::flags(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant ProductInstanceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (orientation)
    {
        case Qt::Horizontal:
            return horizontalHeaderData(section, role);
        case Qt::Vertical:
            return {};
        default:
            Q_UNREACHABLE();
    }
}

QVariant ProductInstanceTableModel::horizontalHeaderData(int section, int role) const
{
    switch (role)
    {
        case Qt::DisplayRole:
        {
            if (section == 0)
            {
                return QStringLiteral("Seam");
            } else
            {
                const auto seamIndex = section - 1;

                if (seamIndex < 0 || seamIndex >= int(m_productSeams.size()))
                {
                    return {};
                }

                return m_productSeams.at(seamIndex)->visualNumber();
            }
        }

        case Qt::UserRole:
        {
            if (section == 0)
            {
                return QStringLiteral("Seam Series");
            } else
            {
                const auto seamIndex = section - 1;

                if (seamIndex < 0 || seamIndex >= int(m_productSeams.size()))
                {
                    return {};
                }

                auto seamAtIndex = m_productSeams.at(seamIndex);

                auto it = std::find_if(m_productSeams.begin(), m_productSeams.end(), [seamAtIndex] (auto seam) {
                    return seam->seamSeries()->number() == seamAtIndex->seamSeries()->number();
                });

                const auto firstSeamOfSeries = ((*it)->uuid() == seamAtIndex->uuid());

                return firstSeamOfSeries ? QString::number(m_productSeams.at(seamIndex)->seamSeries()->visualNumber()) : QStringLiteral("");
            }
        }
    }

    return {};
}

void ProductInstanceTableModel::setProduct(Product* product)
{
    if (m_product == product)
    {
        return;
    }

    if (m_product)
    {
        disconnect(m_productDestroyedConnection);
        disconnect(m_product, &Product::seamsChanged, this, &ProductInstanceTableModel::updateSeams);
    }

    m_product = product;

    if (m_product)
    {
        m_productDestroyedConnection = connect(m_product, &QObject::destroyed, this, std::bind(&ProductInstanceTableModel::setProduct, this, nullptr));
        connect(m_product, &Product::seamsChanged, this, &ProductInstanceTableModel::updateSeams);
    } else
    {
        m_productDestroyedConnection = {};
    }

    emit productChanged();
}

void ProductInstanceTableModel::loadSeams()
{
    if (!m_productSeams.empty())
    {
        // column 1 always available, displays instance info -> seam columns have offset 1
        beginRemoveColumns({}, 1, m_productSeams.size());
        m_productSeams.clear();
        endRemoveColumns();
    }

    if (m_product)
    {
        const auto& seams = m_product->seams();

        // column 1 always available, displays instance info -> seam columns have offset 1
        beginInsertColumns({}, 1, seams.size());
        m_productSeams = std::move(seams);

        std::sort(m_productSeams.begin(), m_productSeams.end(), [] (auto s1, auto s2) {
            if (s1->seamSeries()->uuid() == s2->seamSeries()->uuid())
            {
                return s1->number() < s2->number();
            }

            return s1->seamSeries()->number() < s2->seamSeries()->number();
        });
        endInsertColumns();
    }

    emit seamsChanged();
}

void ProductInstanceTableModel::updateSeams()
{
    if (!m_product)
    {
        return;
    }

    const auto& seams = m_product->seams();

    std::vector<Seam*> seamsToRemove;

    std::copy_if(m_productSeams.begin(), m_productSeams.end(), std::back_inserter(seamsToRemove), [&seams] (auto seam) {
        return std::none_of(seams.begin(), seams.end(), [seam] (auto s) {
            return s->uuid() == seam->uuid();
        });
    });

    // remove columns one by one, as there is no guarantee their indices are consecutive
    for (auto seam : seamsToRemove)
    {
        auto it = std::find(m_productSeams.begin(), m_productSeams.end(), seam);
        if (it == m_productSeams.end())
        {
            continue;
        }
        const auto column = int(std::distance(m_productSeams.begin(), it));

        // column 1 always available, displays instance info -> seam columns have offset 1
        beginRemoveColumns({}, column + 1, column + 1);
        m_productSeams.erase(it);
        endRemoveColumns();
    }

    std::vector<Seam*> seamsToAdd;

    std::copy_if(seams.begin(), seams.end(), std::back_inserter(seamsToAdd), [this] (auto seam) {
        return std::none_of(m_productSeams.begin(), m_productSeams.end(), [seam] (auto s) {
            return s->uuid() == seam->uuid();
        });
    });

    m_productSeams.reserve(m_productSeams.size() + seamsToAdd.size());

    for (auto seam : seamsToAdd)
    {
        auto it = std::upper_bound(m_productSeams.begin(), m_productSeams.end(), seam, [] (auto s1, auto s2) {
            if (s1->seamSeries()->uuid() == s2->seamSeries()->uuid())
            {
                return s1->number() < s2->number();
            }

            return s1->seamSeries()->number() < s2->seamSeries()->number();
        });

        const auto column = int(std::distance(m_productSeams.begin(), it));

        // column 1 always available, displays instance info -> seam columns have offset 1
        beginInsertColumns({}, column + 1, column + 1);

        m_productSeams.insert(it, seam);

        endInsertColumns();
    }

    emit seamsChanged();
}

void ProductInstanceTableModel::setStationName(const QString& stationName)
{
    if (m_stationName == stationName)
    {
        return;
    }
    m_stationName = stationName;
    emit stationNameChanged();
}

void ProductInstanceTableModel::setDirectory(const QString& directory)
{
    if (m_directory == directory)
    {
        return;
    }
    m_directory = directory;
    emit directoryChanged();
}

void ProductInstanceTableModel::setMonitoring(bool monitor)
{
    if (m_monitoring == monitor)
    {
        return;
    }
    m_monitoring = monitor;
    emit monitoringChanged();
}

void ProductInstanceTableModel::setLoading(bool loading)
{
    if (m_loading == loading)
    {
        return;
    }
    m_loading = loading;
    emit loadingChanged();
}

void ProductInstanceTableModel::setProductDirectoryName(ProductDirectoryName directoryName)
{
    if (m_productDirectoryName == directoryName)
    {
        return;
    }
    m_productDirectoryName = directoryName;
    emit productDirectoryNameChanged();
}

QString ProductInstanceTableModel::productDirectory() const
{
    if (!m_product)
    {
        return {};
    }
    const auto& dir = QDir{m_directory};

    switch (m_productDirectoryName)
    {
        case ProductDirectoryName::Uuid:
            return dir.absoluteFilePath(m_product->uuid().toString(QUuid::WithoutBraces));
        case ProductDirectoryName::ProductName:
            return dir.absoluteFilePath(m_product->name());
        default:
            Q_UNREACHABLE();
    }
}

std::pair<QList<ProductInstanceTableModel::InstanceFileData>, std::forward_list<QFileInfo>> ProductInstanceTableModel::loadInstances(const QString& path)
{
    std::function<InstanceFileData (const QFileInfo& info)> mapped = [] (const QFileInfo& info) -> InstanceFileData
    {
            const auto& exp = QRegularExpression{QStringLiteral("(.+)-SN-(.+)")};
            const auto& match = exp.match(info.fileName());

            if (!match.hasMatch())
            {
                return {info, QUuid{}, QString{}, QDateTime{}};
            }

            const auto& uuidPart = match.captured(1);
            const auto& serialNumberPart = match.captured(2);

            return {info, QUuid{uuidPart}, serialNumberPart, info.lastModified()};
        };

    std::unique_lock lock{m_mutex};

    std::forward_list<QFileInfo> existingFiles;
    std::forward_list<QFileInfo> filesToDelete;
    QList<QFileInfo> filesToAdd;

    QDirIterator it{path, QDir::NoDotAndDotDot | QDir::Dirs};
    while (it.hasNext())
    {
        it.next();

        const auto& info = it.fileInfo();
        if (!info.isDir())
        {
            continue;
        }

        // find all available files
        existingFiles.emplace_front(info);

        // find all files, still missing in the model data
        if (std::none_of(m_productInstances.cbegin(), m_productInstances.cend(), [&info] (const auto& instance) { return instance.info == info; }))
        {
            filesToAdd << info;
        }
    }

    // find all instances which are no longer present in the directory
    for (const auto& instance : m_productInstances)
    {
        if (std::none_of(existingFiles.cbegin(), existingFiles.cend(), [&instance] (const auto& file) { return file == instance.info; }))
        {
            filesToDelete.emplace_front(instance.info);
        }
    }
    lock.unlock();

    // now map the results by applying regular expression in parallel
    return {QtConcurrent::blockingMapped(filesToAdd, mapped), filesToDelete};
}

void ProductInstanceTableModel::clear()
{
    std::unique_lock lock{m_mutex};

    if (m_productInstances.empty())
    {
        return;
    }

    beginRemoveRows({}, 0, m_productInstances.size() - 1);
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
    endRemoveRows();
}

void ProductInstanceTableModel::update()
{
    clear();

    if (!m_product || m_directory.isEmpty())
    {
        return;
    }

    updateModelData();
}

void ProductInstanceTableModel::updateModelData()
{
    if (!m_product || m_loading || m_preparingDownload)
    {
        m_updatePending = true;
        return;
    }

    m_updatePending = false;
    setLoading(true);

    auto watcher = new QFutureWatcher<std::pair<QList<InstanceFileData>, std::forward_list<QFileInfo>>>{this};
    connect(watcher, &QFutureWatcher<std::pair<QList<InstanceFileData>, std::forward_list<QFileInfo>>>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            auto results = watcher->result();

            auto it = results.first.begin();
            while (it != results.first.end())
            {
                if ((*it).uuid.isNull())
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
                updateModelData();
            }
        }
    );
    watcher->setFuture(QtConcurrent::run(this, &ProductInstanceTableModel::loadInstances, productDirectory()));
}

void ProductInstanceTableModel::updateMonitor()
{
    // first remove all existing paths
    const auto& directories = m_monitor->directories();
    if (!directories.isEmpty())
    {
        m_monitor->removePaths(directories);
    }

    if (m_directory.isEmpty() || !m_product)
    {
        return;
    }

    // now add new paths
    QDir{}.mkpath(productDirectory());
    m_monitor->addPath(productDirectory());
}

void ProductInstanceTableModel::addInstances(const QList<InstanceFileData>& instances)
{
    if (instances.empty())
    {
        return;
    }

    std::unique_lock lock{m_mutex};

    const auto size = m_productInstances.size();

    beginInsertRows({}, size, size + instances.size() - 1);
    for (const auto& instance : instances)
    {
        m_productInstances.emplace_back(ProductInstace{instance.info, instance.uuid, instance.serialNumber, instance.lastModified});
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

    for (auto it = m_productInstances.begin() + size; it != m_productInstances.end(); it++)
    {
        auto& instance = *it;
        if (instance.loadingState != ProductInstace::MetaDataLoadingState::NotLoaded)
        {
            continue;
        }
        instance.loadingState = ProductInstace::MetaDataLoadingState::Loading;
        loadInstanceMetaData(instance.info);
    }
}

void ProductInstanceTableModel::removeInstances(const std::forward_list<QFileInfo>& instances)
{
    if (instances.empty())
    {
        return;
    }

    std::unique_lock lock{m_mutex};

    for (const auto& file : instances)
    {
        auto it = std::find_if(m_productInstances.begin(), m_productInstances.end(), [&file] (const auto& instance) { return instance.info == file; });
        if (it == m_productInstances.end())
        {
            continue;
        }
        const auto row = int(std::distance(m_productInstances.begin(), it));
        beginRemoveRows({}, row, row);
        m_productInstances.erase(it);
        endRemoveRows();
    }
}

void ProductInstanceTableModel::loadInstanceMetaData(const QFileInfo &instance)
{
    auto watcher = new QFutureWatcher<ProductMetaData>(this);
    connect(watcher, &QFutureWatcher<ProductMetaData>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();

            std::unique_lock lock{m_mutex};

            const auto &metaData = watcher->result();
            const QFileInfo metaDataDir{metaData.filePath().absolutePath()};

            auto it = std::find_if(m_productInstances.begin(), m_productInstances.end(),
                [&metaDataDir] (const auto &instance)
                {
                    return instance.info == metaDataDir;
                });
            if (it == m_productInstances.end())
            {
                // during the async load the m_productInstances were changed/reset
                return;
            }
            ProductInstace &instance = *it;
            instance.loadingState = ProductInstace::MetaDataLoadingState::Loaded;

            std::map<int, QVector<int>> changedRoles;

            auto addRolesForColumn = [&changedRoles] (int column, int role)
            {
                auto it = changedRoles.find(column);
                if (it == changedRoles.end())
                {
                    changedRoles.emplace(column, QVector<int>{role});
                } else
                {
                    (*it).second << role;
                }
            };

            if (metaData.isDateValid())
            {
                if (metaData.date() != instance.date)
                {
                    instance.date = metaData.date();
                    addRolesForColumn(0, Qt::UserRole + 1);
                    emit sortingDataChanged();
                }
            }

            if (metaData.isNioValid())
            {
                const auto nio = metaData.nio() ? State::Nio : State::Io;
                if (nio != instance.nio)
                {
                    instance.nio = nio;
                    addRolesForColumn(0, Qt::UserRole + 2);
                }
            }

            if (metaData.isNioSwitchedOffValid())
            {
                if (metaData.nioSwitchedOff() != instance.nioSwitchedOff)
                {
                    instance.nioSwitchedOff = metaData.nioSwitchedOff();
                    addRolesForColumn(0, Qt::UserRole + 3);
                }
            }

            if (metaData.isSeamsValid())
            {
                for (const auto& seam : metaData.seams())
                {
                    if (seam.isUuidValid() && seam.isNioValid())
                    {
                        auto it = instance.seamNio.find(seam.uuid());

                        const auto nio = seam.nio() ? State::Nio : State::Io;

                        auto changed = false;

                        if (it == instance.seamNio.end())
                        {
                            instance.seamNio.emplace(seam.uuid(), nio);

                            changed = true;
                        } else
                        {
                            if ((*it).second != nio)
                            {
                                (*it).second = nio;

                                changed = true;
                            }
                        }

                        if (changed)
                        {
                            auto seam_it = std::find_if(m_productSeams.begin(), m_productSeams.end(), [seam] (auto s) {
                                return s->uuid() == seam.uuid();
                            });

                            if (seam_it != m_productSeams.end())
                            {
                                // offset 1, as column 0 is reserved for the instance components
                                const int idx = std::distance(m_productSeams.begin(), seam_it) + 1;
                                addRolesForColumn(idx, Qt::UserRole + 2);
                            }
                        }
                    }
                }
            }

            if (const auto serialNumber{m_extendedProductInfoHelper->serialNumber(metaData.extendedProductInfo())})
            {
                instance.serialNumber = serialNumber.value();
                addRolesForColumn(0, Qt::DisplayRole);
            }
            if (const auto partNumber{m_extendedProductInfoHelper->partNumber(metaData.extendedProductInfo())})
            {
                instance.partNumber = partNumber.value();
                addRolesForColumn(0, Qt::UserRole + 11);
            }

            if (!changedRoles.empty())
            {
                auto it = std::find_if(m_productInstances.begin(), m_productInstances.end(), [&instance] (const auto& inst) { return instance.uuid == inst.uuid; });

                if (it != m_productInstances.end())
                {
                    const int idx = std::distance(m_productInstances.begin(), it);

                    for (auto seam : changedRoles)
                    {
                        emit dataChanged(index(idx, seam.first), index(idx, seam.first), seam.second);
                    }
                }
            }
        }
    );

    watcher->setFuture(QtConcurrent::run(qOverload<const QFileInfo &>(&ProductMetaData::parse), instance));
}

int ProductInstanceTableModel::seamResultIndex(int row, Seam* seam)
{
    std::vector<Seam*> seamsWithResultsInInstance;

    const auto& results = m_productInstances.at(row).seamNio;

    std::copy_if(m_productSeams.begin(), m_productSeams.end(), std::back_inserter(seamsWithResultsInInstance), [&results] (auto productSeam) {
        return std::any_of(results.begin(), results.end(), [productSeam] (auto& result) { return result.first == productSeam->uuid(); });
    });

    auto it = std::find_if(seamsWithResultsInInstance.begin(), seamsWithResultsInInstance.end(), [seam] (auto s) {
       return seam->uuid() == s->uuid();
    });

    if (it == seamsWithResultsInInstance.end())
    {
        return -1;
    }

    return std::distance(seamsWithResultsInInstance.begin(), it);
}

void ProductInstanceTableModel::checkAll()
{
    if (m_productInstances.empty())
    {
        return;
    }
    for (auto& productInstance : m_productInstances)
    {
        productInstance.checked = true;
    }
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::UserRole + 7});
}

void ProductInstanceTableModel::uncheckAll()
{
    if (m_productInstances.empty())
    {
        return;
    }
    for (auto& productInstance : m_productInstances)
    {
        productInstance.checked = false;
    }
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::UserRole + 7});
}

void ProductInstanceTableModel::setPreparingDownload(bool value)
{
    if (m_preparingDownload == value)
    {
        return;
    }
    m_preparingDownload = value;
    emit preparingDownloadChanged();
}

void ProductInstanceTableModel::downloadChecked(QString targetDir, QString subDir)
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

bool ProductInstanceTableModel::downloading() const
{
    return m_downloadHelper->downloading();
}


void ProductInstanceTableModel::cancelDownload()
{
    m_downloadHelper->cancelDownload();
}

}
}
