#pragma once

#include <QAbstractListModel>
#include <QUuid>
#include <set>
#include <vector>
#include <memory>
#include "event/schedulerEvents.proxy.h"
#include "separatelyProductExporter.h"
#include"event/controlSimulation.proxy.h"

namespace precitec
{
typedef std::shared_ptr<precitec::interface::TSchedulerEvents<precitec::interface::EventProxy>> SchedulerEventsProxy;
namespace gui
{

namespace components
{
namespace removableDevices
{

class CopyService;

}
}
}
}

namespace precitec
{

namespace storage
{
class Product;
class ProductModel;
class Seam;
class SeamInterval;
class SeamSeries;
}

namespace gui
{

/**
 * The ProductController is for editing Products.
 * It takes the ProductModel and allows to select a Product for modification.
 * As soon as a Product gets selected, a copy is created to store the modifications.
 * All changes can be stored to disk.
 *
 * The ProductController is a model providing the same contract as ProductModel.
 **/
class ProductController : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The ProductModel from which the Products are received
     **/
    Q_PROPERTY(precitec::storage::ProductModel *productModel READ productModel WRITE setProductModel NOTIFY productModelChanged)
    /**
     * The currently selected Product.
     * @see selectProduct
     **/
    Q_PROPERTY(precitec::storage::Product *currentProduct READ currentProduct NOTIFY currentProductChanged)
    /**
     * The currently selected SeamSeries of the currentProduct
     * @see selectSeamSeries
     **/
    Q_PROPERTY(precitec::storage::SeamSeries *currentSeamSeries READ currentSeamSeries NOTIFY currentSeamSeriesChanged)
    /**
     * The currently selected Seam of the currentProduct
     * @see selectSeam
     **/
    Q_PROPERTY(precitec::storage::Seam *currentSeam READ currentSeam NOTIFY currentSeamChanged)
    /**
     * The currently selected SeamInterval of the currentSeam
     * @see selectSeamInterval
     **/
    Q_PROPERTY(precitec::storage::SeamInterval *currentSeamInterval READ currentSeamInterval NOTIFY currentSeamIntervalChanged)
    /**
     * Whether any of the Products got modified.
     **/
    Q_PROPERTY(bool changes READ hasChanges NOTIFY hasChangesChanged)
    /**
     * Path to the scanfield image dir
     **/
    Q_PROPERTY(QString scanfieldPath READ scanfieldPath WRITE setScanfieldPath NOTIFY scanfieldPathChanged)
    /**
     * Export and import state information
     **/
    Q_PROPERTY(bool copyInProgress READ copyInProgress WRITE setCopyInProgress NOTIFY copyInProgressChanged)
    /**
     * The SchedulerEventsProxy is needed to trigger events manually.
    */
    Q_PROPERTY(precitec::SchedulerEventsProxy schedulerEventsProxy READ schedulerEventsProxy WRITE setSchedulerEventsProxy NOTIFY schedulerEventsProxyChanged)

public:
    explicit ProductController(QObject *parent = nullptr);
    ~ProductController() override;

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool copyInProgress()
    {
        return m_copyInProgress;
    }

   precitec::storage::ProductModel *productModel() const
    {
        return m_productModel;
    }
    void setProductModel(precitec::storage::ProductModel *productModel);

    precitec::storage::Product *currentProduct() const
    {
        return m_currentProduct;
    }

    precitec::storage::SeamSeries *currentSeamSeries() const
    {
        return m_currentSeamSeries;
    }

    precitec::storage::Seam *currentSeam() const
    {
        return m_currentSeam;
    }

    precitec::storage::SeamInterval *currentSeamInterval() const
    {
        return m_currentSeamInterval;
    }

    QString scanfieldPath() const
    {
        return m_scanfieldPath;
    }
    void setScanfieldPath(const QString& path);

    precitec::SchedulerEventsProxy schedulerEventsProxy() const
    {
        return m_schedulerEventsProxy;
    }

    void setSchedulerEventsProxy(SchedulerEventsProxy proxy);
    /**
     * Selects the Product with @p id. Creates a copy of the Product in the ProductModel if there is
     * no copy yet.
     **/
    Q_INVOKABLE void selectProduct(const QUuid &id, const QUuid &seamSeriesId = {}, const QUuid &seamId = {}, const QUuid &intervalId = {});

    /**
     * Selects the SeamSeries with @p id of the currentProduct.
     **/
    Q_INVOKABLE void selectSeamSeries(const QUuid &id, const QUuid &seamId = {}, const QUuid &intervalId = {});

    /**
     * Selects the Seam with @p id of the currentProduct.
     **/
    Q_INVOKABLE void selectSeam(const QUuid &id, const QUuid &intervalId = {});

    /**
     * Selects the SeamInterval with @p id of the currenSeam.
     **/
    Q_INVOKABLE void selectSeamInterval(const QUuid &id);

    /**
     * Creates a new Seam in the current Product and selects it.
     **/
    Q_INVOKABLE void createSeam();

    /**
     * Creates a new Seam in the current Product as a copy of the Seam identified by @p seamId and selects it.
     **/
    Q_INVOKABLE void createSeamCopy(const QUuid &seamId);

    /**
     * Creates a new Seam in the current Product as a copy of the Seam identified by @p seamId in the product @p productId and selects it.
     **/
    Q_INVOKABLE void createSeamCopy(const QUuid &productId, const QUuid &seamId, const quint16& numberOfCopies);

    /**
     * Creates a new SeamSeries in the current Product and selects it.
     **/
    Q_INVOKABLE void createSeamSeries();

    /**
     * Creates @p numberOfCopies new SeamSeries in the Product identified with @p productId as copies of the SeamSeries identified by @p seriesId.
     * If @p numberOfCopies is one, the new seam is selected.
     **/
    Q_INVOKABLE void createSeamSeriesCopy(const QUuid &productId, const QUuid& seriesId, const quint16& numberOfCopies);

    /**
     * Saves all modified Products to disk.
     * @returns @c true if all Products where successfully saved, @c false on failure.
     **/
    Q_INVOKABLE bool saveChanges();

    /**
     * Marks the current product as changed
     **/
    Q_INVOKABLE void markAsChanged();

    Q_INVOKABLE void markProductAsChanged(const QUuid& product);

    Q_INVOKABLE precitec::storage::Product* modifiedProduct(const QUuid& id);

    /**
     * Creates a new Product and adds it to the model.
     * The product is not yet saved.
     **/
    Q_INVOKABLE precitec::storage::Product *createProduct();

    /**
     * Creates a new Product as copy of the Product identfied by @p sourceId and adds it to the model.
     * The product is not yet saved.
     **/
    Q_INVOKABLE precitec::storage::Product *createProductAsCopy(const QUuid &sourceId);

    /**
     * Creates a new Product from the json at @p path and adds it to the model.
     * The imported product is not yet saved.
     * If there is a Product with the same uuid as in the imported file, this product is returned instead.
     **/
    Q_INVOKABLE precitec::storage::Product *importProduct(const QString &path);
    /**
     * Checks existence of the product folder @p pathToFolder/ @p folderName on removable device.
     * It is used for the export.
     **/
    Q_INVOKABLE bool separatedProductFolderWithNameExists(const QString &folderPath);
    /**
     * Exports current product and artifacts to the removable device in a separated productfolder (graphs should be added?).
     * It overwrites the folder in case of existence.
     **/
    Q_INVOKABLE void exportCurrentProductSeparately(const QString &path);
    /**
     * Checks products on disk for the import of the separated product on removable device.
     * Returned strings "" - means that there are no products with correspondent uuid.
     **/
    Q_INVOKABLE QString absolutePathToProduct(const QString &path);
    /**
     * Import current product from "path" (graphs should be added?).
     **/
    Q_INVOKABLE void importSeparatedProduct(const QString &path);

    /**
     * Deletes the Product with the @p id.
     **/
    Q_INVOKABLE void deleteProduct(const QUuid &id);

    /**
     * Discard all changes of the Product with @p id.
     * Might deactivate other Products.
     **/
    Q_INVOKABLE void discardChanges(const QUuid &id);

    bool hasChanges() const
    {
        return !m_changes.empty();
    }

    /**
     * Discards the temporary copy of the current product if the copy was not modified.
     **/
    Q_INVOKABLE void discardCopyWhenUnmodified();

    /**
     * Checks whether the Product with @p id has the video recorder hardware paramter enabled.
     * @returns an invalid QVariant if the hardware parameter is not present, otherwise the QVariant
     * contains whether the video recorder is enabled.
     **/
    Q_INVOKABLE QVariant isVideoRecorderEnabled(const QUuid &id) const;

    /**
     * Enables the video recorder hardware parameter for the Product with @p id.
     * If @p value is an invalid QVariant the hardware parameter is removed, otherwise the
     * hardware parameter is set or created with @p value interpreted as boolean.
     **/
    Q_INVOKABLE void setVideoRecorderEnabled(const QUuid &id, const QVariant &value);

    /**
     * Validates whether @p type is valid for product @p p.
     **/
    Q_INVOKABLE bool isTypeValid(precitec::storage::Product *p, int type) const;

    int nextType() const;

    Q_INVOKABLE int currentProductIndex();

    /**
     * Imports a Product from legacy Weldmaster Full xml provided at @p path.
     * @returns The imported Product on success, @c null on failure
     **/
    Q_INVOKABLE precitec::storage::Product *importFromXml(const QString &path);

    /**
     * Provides a list of possible xml files in @p basePath and it's subdirectories.
     **/
    Q_INVOKABLE QStringList productImportList(const QString &basePath);

    /**
     * Provides a list of possible separated products. Possible separated products
     * are located in @p basePath. They are essentially a list of subfolder names.
     **/
    Q_INVOKABLE QStringList separatedProductImportList(const QString &path);

    Q_INVOKABLE void manualTrigger(const QUuid &id,
        const QString& type,
        const QString& name,
        const QString& filePath);

Q_SIGNALS:
    void productModelChanged();
    void currentProductChanged();
    void currentSeamSeriesChanged();
    void currentSeamChanged();
    void currentSeamIntervalChanged();
    void hasChangesChanged();
    void scanfieldPathChanged();
    void copyInProgressChanged();
    void schedulerEventsProxyChanged();

private:
    void setCopyInProgress(bool isCopying);
    void addChange(const QUuid &id);
    int rowOf(const QUuid &id);
    void reselect();
    void duplicateScanfieldImage(const QString& source, const QString& destination);

    precitec::storage::Product *findProduct(const QUuid &id) const;
    precitec::storage::Product *findProduct(const QUuid &id, const std::vector<precitec::storage::Product *> &products) const;
    precitec::storage::Product *createProductCopy(precitec::storage::Product *p);
    void checkProductSize(precitec::storage::Product* product);
    precitec::storage::ProductModel *m_productModel = nullptr;
    precitec::storage::Product *m_currentProduct = nullptr;
    precitec::storage::SeamSeries *m_currentSeamSeries = nullptr;
    precitec::storage::Seam *m_currentSeam = nullptr;
    precitec::storage::SeamInterval *m_currentSeamInterval = nullptr;
    precitec::gui::components::removableDevices::CopyService *m_copyService = nullptr;
    QString m_scanfieldPath;
    bool m_copyInProgress{false};
    std::atomic<bool> m_copyServiceInProgress{false};

    std::vector<precitec::storage::Product *> m_modifiedProducts;
    std::vector<precitec::storage::Product *> m_newProducts;
    std::set<QUuid> m_changes;
    std::unique_ptr<SeparatelyProductExporter> m_exportProduct;
    SchedulerEventsProxy m_schedulerEventsProxy = nullptr;
    
    //interface::TControlSimulation<interface::EventProxy> m_controlSimulationProxy;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ProductController*)
