#pragma once

#include <QAbstractTableModel>
#include <QDateTime>
#include <QFileInfo>
#include <QUuid>

#include <mutex>
#include <memory>
#include <vector>
#include <forward_list>

class QMutex;
class QFileSystemWatcher;
class TestProductInstanceTableModel;

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
class Seam;
class ProductInstanceDownloadHelper;

/**
 * A model providing a table of all processed seams of all product instances for a provided Product and path
 **/
class ProductInstanceTableModel : public QAbstractTableModel
{
    Q_OBJECT

    /**
     * The Product for which the instances should be provided.
     * Default is @c null.
     **/
    Q_PROPERTY(precitec::storage::Product* product READ product WRITE setProduct NOTIFY productChanged)

    /**
     * The directory in which to look for Product instances.
     * Default is an empty, invalid directory path.
     **/
    Q_PROPERTY(QString directory READ directory WRITE setDirectory NOTIFY directoryChanged)

    /**
     * Whether the ProductInstanceModel should monitor the directory for new product instances.
     * Default is @c false
     **/
    Q_PROPERTY(bool monitoring READ monitoring WRITE setMonitoring NOTIFY monitoringChanged)

    /**
     * Whether the model is currently loading data
     **/
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

    /**
     * How the Product directory is named, either Uuid or ProductName
     **/
    Q_PROPERTY(ProductDirectoryName productDirectoryName READ productDirectoryName WRITE setProductDirectoryName NOTIFY productDirectoryNameChanged)

    /**
     * The name of the station. Added to the relative path role if not empty
     **/
    Q_PROPERTY(QString stationName READ stationName WRITE setStationName NOTIFY stationNameChanged)

    Q_PROPERTY(precitec::storage::ExtendedProductInfoHelper *extendedProductInfoHelper READ extendedProductInfoHelper CONSTANT)

    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

public:
    explicit ProductInstanceTableModel(QObject* parent = nullptr);
    ~ProductInstanceTableModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Product* product() const
    {
        return m_product;
    }
    void setProduct(Product* product);

    QString stationName() const
    {
        return m_stationName;
    }
    void setStationName(const QString& stationName);

    QString directory() const
    {
        return m_directory;
    }
    void setDirectory(const QString& directory);

    bool monitoring() const
    {
        return m_monitoring;
    }
    void setMonitoring(bool monitor);

    bool loading() const
    {
        return m_loading;
    }

    ExtendedProductInfoHelper *extendedProductInfoHelper() const
    {
        return m_extendedProductInfoHelper;
    }

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
    void setProductDirectoryName(ProductDirectoryName directoryName);

    bool preparingDownload() const
    {
        return m_preparingDownload;
    }
    void setPreparingDownload(bool value);

    bool downloading() const;

    Q_INVOKABLE int seamResultIndex(int row, precitec::storage::Seam* seam);

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
    void productChanged();
    void seamsChanged();
    void stationNameChanged();
    void directoryChanged();
    void monitoringChanged();
    void loadingChanged();
    void productDirectoryNameChanged();
    void sortingDataChanged();
    void preparingDownloadChanged();
    void downloadingChanged();

private:
    void setLoading(bool loading);
    void clear();
    void update();
    void updateModelData();
    void updateMonitor();
    void loadSeams();
    void updateSeams();
    QString productDirectory() const;

    struct InstanceFileData {
        QFileInfo info;
        QUuid uuid;
        QString serialNumber;
        QDateTime lastModified;
    };

    std::pair<QList<InstanceFileData>, std::forward_list<QFileInfo>> loadInstances(const QString& path);
    void addInstances(const QList<InstanceFileData>& instances);
    void removeInstances(const std::forward_list<QFileInfo>& instances);
    QVariant horizontalHeaderData(int section, int role = Qt::DisplayRole) const;

    struct ProductInstace {
        QFileInfo info;
        QUuid uuid;
        QString serialNumber;
        QDateTime lastModified;
        std::map<QUuid, State> seamNio;
        State nio = State::Unknown;
        QDateTime date;
        bool nioSwitchedOff = false;
        enum class MetaDataLoadingState {
            NotLoaded,
            Loading,
            Loaded
        };
        MetaDataLoadingState loadingState = MetaDataLoadingState::NotLoaded;
        precitec::gui::components::removableDevices::DownloadService* downloadService = nullptr;
        QMetaObject::Connection connection;
        bool checked = false;
        QString partNumber;
    };
    std::vector<ProductInstace> m_productInstances;
    void loadInstanceMetaData(const QFileInfo &instance);

    std::vector<Seam*> m_productSeams;
    QString m_stationName;
    QString m_directory;
    bool m_monitoring = false;
    bool m_loading = false;
    bool m_updatePending = false;
    ProductDirectoryName m_productDirectoryName = ProductDirectoryName::Uuid;
    Product* m_product = nullptr;
    QMetaObject::Connection m_productDestroyedConnection;
    std::mutex m_mutex;
    QFileSystemWatcher* m_monitor;
    ExtendedProductInfoHelper *m_extendedProductInfoHelper;
    bool m_preparingDownload = false;
    ProductInstanceDownloadHelper * m_downloadHelper;

    friend TestProductInstanceTableModel;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::ProductInstanceTableModel*)

