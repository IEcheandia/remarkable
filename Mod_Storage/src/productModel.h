#pragma once

#include <QAbstractListModel>
#include <QDir>
#include <QMutex>
#include <QUuid>

#include <vector>

class QFileSystemWatcher;

namespace precitec
{
namespace storage
{
class AbstractMeasureTask;
class Product;
class ParameterSet;

/**
 * @brief Model which holds all @link{Product}s.
 *
 * This is a small model which functions as the data holder for the
 * @link{Product}s. It provides the functionality to load all Products
 * from json and exposes the Product information through the model.
 *
 * The model provides the following roles:
 * @li display: The name of the Product
 * @li uuid: The uuid of the Product
 * @li product: The complete Product
 * @li active: The active property of the Product
 **/
class ProductModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * Property used to pause the automatic update/cleanup of Reference Curves and Scanfield Images
     **/
    Q_PROPERTY(bool cleanupEnabled READ cleanupEnabled WRITE setCleanupEnabled NOTIFY cleanupEnabledChanged)

public:
    explicit ProductModel(QObject *parent = nullptr);
    ~ProductModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * Provides the QUuid for the given @p index.
     *
     * Just a convienent for @link{data} with role @c Qt::UserRole which can be
     * invoked from Qml side.
     **/
    Q_INVOKABLE QUuid uuid(const QModelIndex &index) const;

    /**
     * Loads all the @link{Product}s from the @p productStorageDirectory.
     * All json files in the @p productStorageDirectory are considered and
     * added to the model.
     *
     * The method resets the model and all previously created Products are
     * destroyed.
     **/
    void loadProducts(const QDir &productStorageDirectory);

    /**
     * @returns The directory from which products where loaded.
     * @see loadProducts
     **/
    QDir productStorageDirectory() const
    {
        return m_productStorageDirectory;
    }

    QString referenceCurveStorageDirectory() const
    {
        return m_referenceCurveStorageDirectory;
    }
    void setReferenceStorageDirectory(const QString& dir);

    QString scanfieldImageStorageDirectory() const
    {
        return m_scanfieldImageStorageDirectory;
    }
    void setScanfieldImageStorageDirectory(const QString& dir);

    /**
     * @returns Direct reading access to the underlying Products.
     **/
    const std::vector<Product*> &products() const
    {
        return m_products;
    }

    /**
     * @returns the product with @p uuid, @c null if there is no such Product
     **/
    Q_INVOKABLE precitec::storage::Product *findProduct(const QUuid &uuid) const;

    /**
     * @returns the default product or @c null if there is no such Product
     **/
    Product *defaultProduct() const;

    /**
     * @returns The filter ParameterSet with the given @p uuid in any Product hold in the model.
     **/
    ParameterSet *findFilterParameterSet(const QUuid &uuid) const;

    /**
     * Ensures that all Filter ParameterSet of the Product containing the @p referenceParameterSet are loaded.
     **/
    void ensureAllFilterParameterSetsLoaded(const QUuid& referenceParameterSet);

    /**
     * Finds the AbstractMeasureTask with the given @p id or @c null if none exists.
     * Considers recursively all SeamSeries, Seams and SeamIntervals of all products.
     **/
    AbstractMeasureTask *findMeasureTask(const QUuid &id) const;

    /**
     * Mutex to protect changes on the hold products.
     * When accesing the Products from another Thread one should lock
     * this mutex to ensure that the objects don't get destroyed while
     * accessing them.
     **/
    QMutex *storageMutex()
    {
        return &m_storageMutex;
    }

    /**
     * Reloads the Product with the given @p uuid.
     **/
    void reloadProduct(const QUuid &uuid);

    bool cleanupEnabled() const
    {
        return m_cleanupEnabled;
    }
    void setCleanupEnabled(bool cleanupEnabled);

Q_SIGNALS:
    void referenceCurveStorageDirectoryChanged();
    void scanfieldImageStorageDirectoryChanged();
    void cleanupEnabledChanged();

private:
    void updateProduct(const QString &path);
    void updateReferenceCurves();
    void updateScanfieldImages();
    void clearJsonWatcher();
    void checkNewProducts();
    std::vector<Product*> m_products;
    QFileSystemWatcher *m_jsonWatcher;
    bool m_cleanupEnabled = true;
    QMutex m_storageMutex;
    QDir m_productStorageDirectory = QDir();
    QString m_referenceCurveStorageDirectory;
    QString m_scanfieldImageStorageDirectory;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ProductModel*)
