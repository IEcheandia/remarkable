#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QFileInfo>
#include <QUuid>
#include <QFutureWatcher>

#include <memory>
#include <vector>
#include <forward_list>

#include "productMetaData.h"

class QFileSystemWatcher;
class QMutex;

namespace precitec
{
namespace gui
{
namespace components
{
namespace removableDevices
{

class DownloadService;

}
}
}
namespace storage
{
class ExtendedProductInfoHelper;
class Product;
class ProductInstanceSortModel;
class ProductInstanceDownloadHelper;

/**
 * A model providing all product instances for a provided Product and path
 **/
class ProductInstanceModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The directory in which to look for Product instances.
     * Default is an empty, not valid directory path.
     **/
    Q_PROPERTY(QString directory READ directory WRITE setDirectory NOTIFY directoryChanged)
    /**
     * The Product for which the instances should be provided.
     * Default is @c null.
     **/
    Q_PROPERTY(precitec::storage::Product *product READ product WRITE setProduct NOTIFY productChanged)
    /**
     * Whether the model is currently loading data
     **/
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

    /**
     * A filter model on this ProductInstanceModel
     **/
    Q_PROPERTY(precitec::storage::ProductInstanceSortModel *filterModel READ filterModel CONSTANT)
    /**
     * How the Product directory is named, either Uuid or ProductName
     **/
    Q_PROPERTY(ProductDirectoryName productDirectoryName READ productDirectoryName WRITE setProductDirectoryName NOTIFY productDirectoryNameChanged)

    /**
     * Whether the ProductInstanceModel should monitor the directory for new product instances.
     * Default is @c false
     **/
    Q_PROPERTY(bool monitoring READ isMonitoring WRITE setMonitoring NOTIFY monitoringChanged)

    /**
     * The name of the station. Added to the relative path role if not empty
     **/
    Q_PROPERTY(QString stationName READ stationName WRITE setStationName NOTIFY stationNameChanged)

    /**
     * Full path to the directory with product instances
     **/
    Q_PROPERTY(QString productDirectory READ productDirectory NOTIFY productDirectoryChanged)

    Q_PROPERTY(precitec::storage::ExtendedProductInfoHelper *extendedProductInfoHelper READ extendedProductInfoHelper CONSTANT)

    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

public:
    explicit ProductInstanceModel(QObject *parent = nullptr);
    ~ProductInstanceModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setDirectory(const QString &directory);
    QString directory() const
    {
        return m_directory;
    }

    void setProduct(Product *product);
    Product *product() const
    {
        return m_product;
    }

    bool isLoading() const;

    ProductInstanceSortModel *filterModel() const
    {
        return m_filterModel;
    }

    QString stationName() const
    {
        return m_stationName;
    }
    void setStationName(const QString &stationName);

    enum class State {
        Unknown,
        Nio,
        Io
    };
    Q_ENUM(State)

    /**
     * Enum describing what identifier to use for the Product directory name.
     **/
    enum class ProductDirectoryName {
        /**
         * Directory name is the Product's UUID
         **/
        Uuid,
        /**
         * Directory name is the Product's name
         **/
        ProductName
    };
    Q_ENUM(ProductDirectoryName)

    ProductDirectoryName productDirectoryName() const
    {
        return m_productDirectoryName;
    }

    QString productDirectory() const
    {
        return subDirectory();
    }

    void setProductDirectoryName(ProductDirectoryName directoryName);

    bool isMonitoring() const
    {
        return m_monitoring;
    }
    void setMonitoring(bool monitor);

    ExtendedProductInfoHelper *extendedProductInfoHelper() const
    {
        return m_extendedProductInfoHelper;
    }

    bool preparingDownload() const
    {
        return m_preparingDownload;
    }
    void setPreparingDownload(bool value);

    bool downloading() const;

    Q_INVOKABLE void ensureMetaDataLoaded(int idx);

    Q_INVOKABLE void ensureAllMetaDataLoaded();

    /**
     * Toggle the checked state of the item at @p index.
     **/
    Q_INVOKABLE void toggleChecked(int idx);

    /**
     * Check all items.
     **/
    Q_INVOKABLE void checkAll();
    /**
     * Uncheck all items.
     **/
    Q_INVOKABLE void uncheckAll();

    Q_INVOKABLE void downloadChecked(QString targetDir, QString subDir);

    Q_INVOKABLE void cancelDownload();

Q_SIGNALS:
    void directoryChanged();
    void productChanged();
    void loadingChanged();
    void productDirectoryNameChanged();
    void monitoringChanged();
    void stationNameChanged();
    void productDirectoryChanged();
    void preparingDownloadChanged();
    void downloadingChanged();
private:
    void update();
    void updateDirectory();
    void clear();
    void setLoading(bool set);
    void addInstance(const QFileInfo &info, const QUuid &uuid);
    void addInstances(const QList<std::tuple<QFileInfo, QUuid, QString, QDateTime>> &data);
    void removeInstances(const std::forward_list<QFileInfo> &instances);
    QModelIndex sourceIndex(int idx) const;
    std::pair<QList<std::tuple<QFileInfo, QUuid, QString, QDateTime>>, std::forward_list<QFileInfo>> load(const QString &path);
    void updateMonitor();
    QString subDirectory() const;
    QString m_directory;
    Product *m_product = nullptr;
    QString m_stationName;
    QMetaObject::Connection m_productDestroyedConnection;
    bool m_loading = false;
    struct ProductInstace {
        QFileInfo info;
        QUuid uuid;
        QString serialNumber;
        QDateTime lastModified;
        State nio = State::Unknown;
        QDateTime date;
        bool nioSwitchedOff = false;
        precitec::gui::components::removableDevices::DownloadService* downloadService = nullptr;
        QMetaObject::Connection connection;
        enum class MetaDataLoadingState {
            NotLoaded,
            Loading,
            Loaded
        };
        MetaDataLoadingState loadingState = MetaDataLoadingState::NotLoaded;
        bool checked = false;
        QString partNumber;
    };
    std::vector<ProductInstace> m_productInstances;
    std::unique_ptr<QMutex> m_mutex;
    ProductInstanceSortModel *m_filterModel;
    ProductDirectoryName m_productDirectoryName = ProductDirectoryName::Uuid;
    bool m_monitoring = false;
    QFileSystemWatcher *m_monitor;
    bool m_updatePending = false;
    QFutureWatcher<std::pair<QList<std::tuple<QFileInfo, QUuid, QString, QDateTime>>, std::forward_list<QFileInfo>>> *m_loadWatcher = nullptr;
    std::vector<QFutureWatcher<ProductMetaData>*> m_metaDataWatchers;
    ExtendedProductInfoHelper *m_extendedProductInfoHelper;
    bool m_preparingDownload = false;
    ProductInstanceDownloadHelper * m_downloadHelper;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ProductInstanceModel*)
