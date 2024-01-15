#include "productController.h"
#include "copyMode.h"
#include "parameter.h"
#include "parameterSet.h"
#include "productAddedChangeEntry.h"
#include "productDeletedChangeEntry.h"
#include "product.h"
#include "productModel.h"
#include "productModifiedChangeEntry.h"
#include "seam.h"
#include "seamInterval.h"
#include "seamSeries.h"
#include "event/schedulerEvents.interface.h"

#include <precitec/notificationSystem.h>
#include <precitec/userLog.h>
#include <precitec/copyService.h>
#include <precitec/utils.h>

#include <QDirIterator>
#include <QJsonObject>
#include <QDir>
#include <QUuid>
#include <QFile>
#include <QFutureWatcher>
#include <QtConcurrent>

#include <optional>

#include "removableDevicePaths.h"
#include "separatelyProductExporter.h"

using precitec::storage::CopyMode;
using precitec::storage::Parameter;
using precitec::storage::ParameterSet;
using precitec::storage::ProductModel;
using precitec::storage::Product;

using precitec::gui::components::userLog::UserLog;
using precitec::gui::components::notifications::NotificationSystem;
using precitec::gui::components::removableDevices::CopyService;
using precitec::gui::RemovableDevicePaths;

using precitec::interface::TSchedulerEvents;
using precitec::interface::EventProxy;
using precitec::interface::SchedulerEvents;

namespace precitec
{
namespace gui
{

namespace
{
auto hasUuid(const QUuid& id)
{
    return [id] (Product* p) { return p->uuid() == id; };
}
static constexpr std::size_t s_mebibyte = 1024 * 1024;
static constexpr std::size_t s_maxProductFileSize = 30;
static constexpr std::size_t s_maxProductFileSizeInBytes = s_maxProductFileSize * s_mebibyte;
static constexpr std::size_t s_maxDirectorySize = 150;
static constexpr std::size_t s_maxDirectorySizeInBytes = s_maxDirectorySize * s_mebibyte;
}

Poco::UUID convertQUuidToPoco(const QUuid& qUuid);

ProductController::ProductController(QObject *parent)
    : QAbstractListModel(parent)
    , m_copyService(new CopyService)
    , m_exportProduct(std::make_unique<SeparatelyProductExporter>(m_copyService))
{
    connect(m_copyService, &CopyService::backupInProgressChanged, this, [this]
    {
            m_copyServiceInProgress = this->m_copyService->isBackupInProgress();
            setCopyInProgress(m_copyServiceInProgress);
    }, Qt::QueuedConnection);

    connect(m_exportProduct.get(), &SeparatelyProductExporter::copyInProgressChanged, this, &ProductController::setCopyInProgress);
}

ProductController::~ProductController() = default;

QVariant ProductController::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    if (!m_productModel)
    {
        return {};
    }
    Product *p = nullptr;
    if (index.row() >= m_productModel->rowCount())
    {
        const std::size_t row = index.row() - m_productModel->rowCount();
        if (row >= m_newProducts.size())
        {
            return {};
        }
        p = m_newProducts.at(row);
    }
    if (!p)
    {
        p = m_productModel->index(index.row(), 0).data(Qt::UserRole + 1).value<Product*>();
    }
    if (!p)
    {
        return {};
    }
    auto modified = findProduct(p->uuid());
    if (modified)
    {
        p = modified;
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return p->name();
    case Qt::UserRole:
        return p->uuid();
    case Qt::UserRole+1:
        return QVariant::fromValue(p);
    case Qt::UserRole+2:
        return m_changes.count(p->uuid()) != 0;
    default:
        return {};
    }
}

int ProductController::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    if (!m_productModel)
    {
        return 0;
    }
    return m_productModel->rowCount() + m_newProducts.size();
}

QHash<int, QByteArray> ProductController::roleNames() const
{
    return QHash<int, QByteArray>{
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("uuid")},
        {Qt::UserRole+1, QByteArrayLiteral("product")},
        {Qt::UserRole+2, QByteArrayLiteral("changed")}
    };
}

void ProductController::setProductModel(ProductModel *productModel)
{
    if (productModel == m_productModel)
    {
        return;
    }
    beginResetModel();
    m_productModel = productModel;
    if (m_productModel)
    {
        connect(m_productModel, &QAbstractItemModel::modelAboutToBeReset, this, &ProductController::beginResetModel);
        connect(m_productModel, &QAbstractItemModel::modelReset, this, &ProductController::endResetModel);
        connect(m_productModel, &QAbstractItemModel::rowsInserted, this,
            [this] (const QModelIndex &parent, int rowFirst, int rowLast)
            {
                bool wasProduct = false;
                beginResetModel();
                for (int i = rowFirst; i <= rowLast; i++)
                {
                    const auto id = m_productModel->uuid(m_productModel->index(i, 0, parent));
                    auto it = std::find_if(m_newProducts.begin(), m_newProducts.end(), hasUuid(id));
                    if (it != m_newProducts.end())
                    {
                        UserLog::instance()->addChange(new ProductAddedChangeEntry{*it});
                        (*it)->deleteLater();
                        m_newProducts.erase(it);
                        if (m_currentProduct)
                        {
                            if (m_currentProduct->uuid() == (*it)->uuid())
                            {
                                wasProduct = true;
                            }
                        }
                    }
                    const bool wasEmpty = m_changes.empty();
                    m_changes.erase(id);
                    if (wasEmpty != m_changes.empty())
                    {
                        emit hasChangesChanged();
                    }
                }
                endResetModel();
                if (wasProduct)
                {
                    reselect();
                }
            }
        );
        connect(m_productModel, &QAbstractItemModel::rowsAboutToBeRemoved, this,
            [this] (const QModelIndex &parent, int rowFirst, int rowLast)
            {
                const bool wasEmpty = m_changes.empty();
                beginResetModel();
                for (int i = rowFirst; i <= rowLast; i++)
                {
                    const auto id = m_productModel->uuid(m_productModel->index(i, 0, parent));
                    auto it = std::find_if(m_modifiedProducts.begin(), m_modifiedProducts.end(), hasUuid(id));
                    if (it != m_modifiedProducts.end())
                    {
                        m_changes.erase((*it)->uuid());
                        if ((*it) == m_currentProduct)
                        {
                            m_currentProduct = nullptr;
                            if (m_currentSeam)
                            {
                                m_currentSeam = nullptr;
                                emit currentSeamChanged();
                            }
                            emit currentProductChanged();
                        }
                        (*it)->deleteLater();
                        m_modifiedProducts.erase(it);
                    }
                }
                if (wasEmpty != m_changes.empty())
                {
                    emit hasChangesChanged();
                }
            }
        );
        connect(m_productModel, &QAbstractItemModel::rowsRemoved, this, &ProductController::endResetModel);
        connect(m_productModel, &QAbstractItemModel::dataChanged, this,
            [this] (const QModelIndex &topLeft, const QModelIndex &bottomLeft)
            {
                const bool wasEmpty = m_changes.empty();
                for (int row = topLeft.row(); row <= bottomLeft.row(); row++)
                {
                    const auto uuid = m_productModel->uuid(m_productModel->index(row, 0));
                    m_changes.erase(uuid);
                    auto it = std::find_if(m_modifiedProducts.begin(), m_modifiedProducts.end(), [uuid] (auto p) { return p->uuid() == uuid; });
                    if (it != m_modifiedProducts.end())
                    {
                        UserLog::instance()->addChange(new ProductModifiedChangeEntry{(*it)->changes()});
                        (*it)->deleteLater();
                        m_modifiedProducts.erase(it);
                    }
                    emit dataChanged(index(row, 0), index(row, 0), {Qt::UserRole+1, Qt::UserRole+2});
                    if (m_currentProduct && m_currentProduct->uuid() == uuid)
                    {
                        reselect();
                    }
                }
                if (wasEmpty != m_changes.empty())
                {
                    emit hasChangesChanged();
                }
            }
        );
    }
    endResetModel();
    emit productModelChanged();
}

void ProductController::reselect()
{
    const auto seamSeriesId = m_currentSeamSeries ? m_currentSeamSeries->uuid() : QUuid{};
    const auto seamId = m_currentSeam ? m_currentSeam->uuid() : QUuid{};
    const auto intervalId = m_currentSeamInterval ? m_currentSeamInterval->uuid() : QUuid{};
    selectProduct(m_currentProduct->uuid(), seamSeriesId, seamId, intervalId);
}

void ProductController::selectProduct(const QUuid &id, const QUuid &seamSeriesId, const QUuid &seamId, const QUuid &intervalId)
{
    auto p = modifiedProduct(id);

    if (p != m_currentProduct)
    {
        m_currentProduct = p;
        selectSeamSeries(seamSeriesId, seamId, intervalId);
        emit currentProductChanged();
    }
}



void ProductController::setScanfieldPath(const QString& path)
{
    if (m_scanfieldPath.compare(path) == 0)
    {
        return;
    }
    m_scanfieldPath = path;
    emit scanfieldPathChanged();
}

Product* ProductController::modifiedProduct(const QUuid& id)
{
    auto p = findProduct(id);

    if (!p)
    {
        if (!m_productModel)
        {
            return nullptr;
        }
        p = m_productModel->findProduct(id);
        if (!p)
        {
            return nullptr;
        }
        p = createProductCopy(p);
    }

    return p;
}

Product *ProductController::createProductCopy(Product *p)
{
    auto copy = p->duplicate(CopyMode::Identical, this);
    copy->setReferenceCurveStorageDir(p->referenceCruveStorageDir());
    copy->setFilePath(p->filePath());
    m_modifiedProducts.push_back(copy);
    p = copy;
    auto changed = std::bind(&ProductController::addChange, this, p->uuid());
    connect(p, &Product::nameChanged, this, changed);
    connect(p, &Product::typeChanged, this, changed);
    connect(p, &Product::endlessChanged, this, changed);
    connect(p, &Product::triggerModeChanged, this, changed);
    connect(p, &Product::startPositionYAxisChanged, this, changed);
    connect(p, &Product::hardwareParametersChanged, this, changed);
    connect(p, &Product::defaultProductChanged, this, changed);
    connect(p, &Product::seamsChanged, this, changed);
    connect(p, &Product::lengthUnitChanged, this, changed);
    connect(p, &Product::assemblyImageChanged, this, changed);
    connect(p, &Product::seamSeriesChanged, this, changed);
    connect(p, &Product::signalyQualityColorMapChanged, this, changed);
    connect(p, &Product::errorLevelColorMapChanged, this, changed);
    connect(p, &Product::qualityNormChanged, this, changed);
    connect(p, &Product::laserControlPresetChanged, this, changed);
    connect(p, &Product::referenceCurveDataChanged, this, changed);

    connect(p, &Product::nameChanged, this,
        [p, this]
        {
            const int i = rowOf(p->uuid());
            if (i != -1)
            {
                emit dataChanged(index(i, 0), index(i, 0), {Qt::DisplayRole});
            }
        }
    );

    p->setChangeTrackingEnabled(true);

    const int i = rowOf(p->uuid());
    emit dataChanged(index(i, 0), index(i, 0), {Qt::UserRole+1});
    return p;
}

void ProductController::selectSeamSeries(const QUuid &id, const QUuid &seamId, const QUuid &intervalId)
{
    if (!m_currentProduct)
    {
        if (m_currentSeamSeries)
        {
            m_currentSeamSeries = nullptr;
            emit currentSeamSeriesChanged();
        }
        return;
    }
    if (id.isNull())
    {
        m_currentSeamSeries = nullptr;
    } else
    {
        auto seamSeries = m_currentProduct->seamSeries();
        auto it = std::find_if(seamSeries.begin(), seamSeries.end(), [id] (auto s) { return s->uuid() == id; });
        if (it == seamSeries.end())
        {
            m_currentSeamSeries = nullptr;
        } else
        {
            m_currentSeamSeries = *it;
        }
    }
    selectSeam(seamId, intervalId);
    emit currentSeamSeriesChanged();
}

void ProductController::selectSeam(const QUuid &id, const QUuid &intervalId)
{
    if (!m_currentProduct)
    {
        if (m_currentSeam)
        {
            m_currentSeam = nullptr;
            emit currentSeamChanged();
        }
        return;
    }
    if (id.isNull())
    {
        m_currentSeam = nullptr;
    } else
    {
        m_currentSeam = m_currentProduct->findSeam(id);
    }
    selectSeamInterval(intervalId);
    if (m_currentSeam)
    {
        auto series = m_currentSeam->seamSeries();
        if (series != m_currentSeamSeries)
        {
            m_currentSeamSeries = series;
            emit currentSeamSeriesChanged();
        }
    }
    emit currentSeamChanged();
}

void ProductController::selectSeamInterval(const QUuid &id)
{
    if (!m_currentProduct)
    {
        if (m_currentSeam)
        {
            m_currentSeam = nullptr;
            emit currentSeamChanged();
        }
        if (m_currentSeamInterval)
        {
            m_currentSeamInterval = nullptr;
            emit currentSeamIntervalChanged();
        }
        return;
    }
    if (!m_currentSeam)
    {
        if (m_currentSeamInterval)
        {
            m_currentSeamInterval = nullptr;
            emit currentSeamIntervalChanged();
        }
        return;
    }
    if (id.isNull())
    {
        m_currentSeamInterval = nullptr;
    } else
    {
        m_currentSeamInterval = m_currentSeam->findSeamInterval(id);
    }
    emit currentSeamIntervalChanged();
}

void ProductController::createSeam()
{
    if (m_currentSeamSeries)
    {
        selectSeam(m_currentSeamSeries->createSeam()->uuid());
    } else if (m_currentProduct)
    {
        selectSeam(m_currentProduct->createSeam()->uuid());
    }
}

void ProductController::createSeamSeries()
{
    if (!m_currentProduct)
    {
        return;
    }
    selectSeamSeries(m_currentProduct->createSeamSeries()->uuid());
}

void ProductController::createSeamCopy(const QUuid &seamId)
{
    if (!m_currentProduct)
    {
        return;
    }
    if (auto seam = m_currentProduct->findSeam(seamId))
    {
        auto mode = CopyMode::WithDifferentIds;
        if (m_currentSeamSeries)
        {
            selectSeam(m_currentSeamSeries->createSeamCopy(mode, seam)->uuid());
        } else
        {
            selectSeam(seam->seamSeries()->createSeamCopy(mode, seam)->uuid());
        }
    }
}

void ProductController::createSeamCopy(const QUuid &productId, const QUuid &seamId, const quint16& numberOfCopies)
{
    if (!m_currentProduct)
    {
        return;
    }
    if (auto product = m_productModel->findProduct(productId))
    {
        if (auto seam = product->findSeam(seamId))
        {
            auto mode = CopyMode::WithDifferentIds;
            if (m_currentSeamSeries)
            {
                if (numberOfCopies == 1)
                {
                    selectSeam(m_currentSeamSeries->createSeamCopy(mode, seam)->uuid());
                }
                else
                {
                    for (auto i = 0; i < numberOfCopies; i++)
                    {
                        m_currentSeamSeries->createSeamCopy(mode, seam)->uuid();
                    }
                }
            }
            else
            {
                if (m_currentProduct->seamSeries().empty())
                {
                    m_currentProduct->createFirstSeamSeries();
                }
                if (numberOfCopies == 1)
                {
                    selectSeam(m_currentProduct->seamSeries().front()->createSeamCopy(mode, seam)->uuid());
                }
                else
                {
                    for (auto i = 0; i < numberOfCopies; i++)
                    {
                        m_currentProduct->seamSeries().front()->createSeamCopy(mode, seam)->uuid();
                    }
                }
            }
        }
    }
}

void ProductController::createSeamSeriesCopy(const QUuid &productId, const QUuid& seriesId, const quint16& numberOfCopies)
{
    if (!m_currentProduct)
    {
        return;
    }
    if (auto product = m_productModel->findProduct(productId))
    {
        if (auto seamSeries = product->findSeamSeries(seriesId))
        {
            auto mode = CopyMode::WithDifferentIds;
            if (numberOfCopies == 1)
            {
                auto duplicated = m_currentProduct->createSeamSeriesCopy(mode, seamSeries);
                duplicateScanfieldImage(seamSeries->uuid().toString(QUuid::WithoutBraces), duplicated->uuid().toString(QUuid::WithoutBraces));
                selectSeamSeries(duplicated->uuid());
            }
            else
            {
                for (auto i = 0; i < numberOfCopies; i++)
                {
                    auto duplicated = m_currentProduct->createSeamSeriesCopy(mode, seamSeries);
                    duplicateScanfieldImage(seamSeries->uuid().toString(QUuid::WithoutBraces), duplicated->uuid().toString(QUuid::WithoutBraces));
                }
            }
        }
    }
}

void ProductController::duplicateScanfieldImage(const QString& sourceId, const QString& destinationId)
{
    if (m_scanfieldPath.isEmpty())
    {
        return;
    }

    auto scanfieldDir = QDir{m_scanfieldPath};

    if (!scanfieldDir.exists(sourceId) || !scanfieldDir.mkdir(destinationId))
    {
        return;
    }

    const auto& destinationDir = scanfieldDir.absoluteFilePath(destinationId);

    scanfieldDir.cd(sourceId);

    const auto& files = scanfieldDir.entryInfoList(QDir::Files | QDir::Readable);

    for (const auto& file : files)
    {
        QFile::copy(file.absoluteFilePath(), QDir{destinationDir}.absoluteFilePath(file.fileName()));
    }
}


Product *ProductController::findProduct(const QUuid &id) const
{
    if (auto p = findProduct(id, m_modifiedProducts))
    {
        return p;
    }
    if (auto p = findProduct(id, m_newProducts))
    {
        return p;
    }
    return nullptr;
}

precitec::storage::Product *ProductController::findProduct(const QUuid &id, const std::vector<precitec::storage::Product *> &products) const
{
    auto it = std::find_if(products.begin(), products.end(), hasUuid(id));
    if (it == products.end())
    {
        return nullptr;
    }
    return *it;
}

void ProductController::addChange(const QUuid &id)
{
    const bool wasEmpty = m_changes.empty();
    const auto changed = m_changes.emplace(id);
    if (wasEmpty != m_changes.empty())
    {
        emit hasChangesChanged();
    }
    if (std::get<1>(changed))
    {
        const int i = rowOf(id);
        if (i != -1)
        {
            emit dataChanged(index(i, 0), index(i, 0), {Qt::UserRole+2});
        }
    }
    //m_controlSimulationProxy.productChanged(convertQUuidToPoco(id));
}

int ProductController::rowOf(const QUuid &id)
{
    if (!m_productModel)
    {
        return -1;
    }
    const auto &products = m_productModel->products();
    auto it = std::find_if(products.begin(), products.end(), hasUuid(id));
    if (it != products.end())
    {
        return std::distance(products.begin(), it);
    }

    it = std::find_if(m_newProducts.cbegin(), m_newProducts.cend(), hasUuid(id));
    if (it != m_newProducts.cend())
    {
        return products.size() + std::distance(m_newProducts.cbegin(), it);
    }

    return -1;
}

bool ProductController::saveChanges()
{
    if (m_changes.empty())
    {
        return false;
    }
    auto checkType = [this] (auto *product)
    {
        if (!isTypeValid(product, product->type()))
        {
            product->setType(nextType());
            NotificationSystem::instance()->warning(tr("Product type of \"%1\" updated to %2 to prevent conflicts.").arg(product->name()).arg(product->type()));
        }
    };
    for (auto changeIt = m_changes.begin(); changeIt != m_changes.end(); )
    {
        auto id = *changeIt;

        auto it = std::find_if(m_newProducts.begin(), m_newProducts.end(), hasUuid(id));
        if (it != m_newProducts.end())
        {
            // save the new product
            checkType(*it);
            if (!(*it)->saveReferenceCurves())
            {
                return false;
            }
            if (!(*it)->save())
            {
                return false;
            }
            checkProductSize(*it);
            // don't remove from vector, we do this when loaded by ProductModel.
            const int row = rowOf((*it)->uuid());
            changeIt = m_changes.erase(changeIt);
            emit dataChanged(index(row, 0), index(row, 0), {Qt::UserRole+2});
            continue;
        }

        it = std::find_if(m_modifiedProducts.begin(), m_modifiedProducts.end(), hasUuid(id));
        if (it == m_modifiedProducts.end())
        {
            // product not found in modified products, should not happen
            return false;
        }
        checkType(*it);
        if (!(*it)->saveReferenceCurves())
        {
            return false;
        }
        if (!(*it)->save())
        {
            return false;
        }
        checkProductSize(*it);
        const int row = rowOf((*it)->uuid());
        if (*it != m_currentProduct)
        {
            // it's no longer modified, so can be deleted
            (*it)->deleteLater();
            m_modifiedProducts.erase(it);

            emit dataChanged(index(row, 0), index(row, 0), {Qt::UserRole+1});
        }
        changeIt = m_changes.erase(changeIt);
        emit dataChanged(index(row, 0), index(row, 0), {Qt::UserRole+2});
    }
    if (m_changes.empty())
    {
        emit hasChangesChanged();
    }
    return true;
}

void ProductController::markAsChanged()
{
    if (m_currentProduct)
    {
       addChange(m_currentProduct->uuid());
    }
}

void ProductController::markProductAsChanged(const QUuid& product)
{
    addChange(product);
}

precitec::storage::Product *ProductController::createProduct()
{
    if (!m_productModel)
    {
        return nullptr;
    }
    auto product = new Product{QUuid::createUuid(), this};
    QString uuidString = product->uuid().toString(QUuid::WithoutBraces);
    product->setReferenceCurveStorageDir(m_productModel->referenceCurveStorageDirectory());
    product->setFilePath(m_productModel->productStorageDirectory().absoluteFilePath(uuidString + QStringLiteral(".json")));
    product->createFirstSeamSeries();
    product->setType(nextType());
    const int start = m_productModel->rowCount();

    beginInsertRows(QModelIndex(), start + m_newProducts.size(), start + m_newProducts.size());
    m_newProducts.push_back(product);
    endInsertRows();

    addChange(product->uuid());
    return product;
}

Product *ProductController::createProductAsCopy(const QUuid &sourceId)
{
    if (!m_productModel)
    {
        return nullptr;
    }
    auto p = findProduct(sourceId);
    if (!p)
    {
        p = m_productModel->findProduct(sourceId);
    }
    if (!p)
    {
        return nullptr;
    }

    auto product = p->duplicate(CopyMode::WithDifferentIds, this);
    product->setReferenceCurveStorageDir(m_productModel->referenceCurveStorageDirectory());
    product->setFilePath(m_productModel->productStorageDirectory().absoluteFilePath(product->uuid().toString(QUuid::WithoutBraces) + QStringLiteral(".json")));
    product->setName(tr("Copy of %1").arg(p->name()));
    product->setType(nextType());

    const auto& sourceSeries = p->seamSeries();
    const auto& destiantionSeries = product->seamSeries();

    for (std::size_t i = 0u; i < sourceSeries.size(); i++)
    {
        duplicateScanfieldImage(sourceSeries.at(i)->uuid().toString(QUuid::WithoutBraces), destiantionSeries.at(i)->uuid().toString(QUuid::WithoutBraces));
    }

    const int start = m_productModel->rowCount();
    beginInsertRows(QModelIndex(), start + m_newProducts.size(), start + m_newProducts.size());
    m_newProducts.push_back(product);
    endInsertRows();

    addChange(product->uuid());
    return product;
}

int ProductController::nextType() const
{
    int type = 0;
    auto findType = [&type] (auto products)
    {
        auto it = std::max_element(products.begin(), products.end(), [] (auto a, auto b) { return a->type() < b->type(); });
        if (it != products.end())
        {
            type = std::max(type, (*it)->type());
        }
    };
    if (m_productModel)
    {
        findType(m_productModel->products());
    }
    findType(m_newProducts);
    findType(m_modifiedProducts);
    return type + 1;
}

precitec::storage::Product *ProductController::importProduct(const QString &path)
{
    if (!m_productModel)
    {
        return nullptr;
    }

    auto product = Product::fromJson(path);
    if (!product)
    {
        return nullptr;
    }
    // We use the following complication (2 lines) to get reed of
    // the problem "QObject Cannot create children for a parent that is in a different thread"
    // during multi-threading usage of QtConcurrent::run().
    product->moveToThread(this->thread());
    product->setParent(this);

   if (auto p = findProduct(product->uuid()))
    {
        product->deleteLater();
        return p;
    }
    if (auto p = m_productModel->findProduct(product->uuid()))
    {
        product->deleteLater();
        return p;
    }
    product->setReferenceCurveStorageDir(m_productModel->referenceCurveStorageDirectory());
    product->setFilePath(m_productModel->productStorageDirectory().absoluteFilePath(product->uuid().toString(QUuid::WithoutBraces) + QStringLiteral(".json")));
    if (!isTypeValid(product, product->type()))
    {
        product->setType(nextType());
    }
    const int start = m_productModel->rowCount();

    beginInsertRows(QModelIndex(), start + m_newProducts.size(), start + m_newProducts.size());
    m_newProducts.push_back(product);
    endInsertRows();

    addChange(product->uuid());
    return product;
}

bool ProductController::separatedProductFolderWithNameExists(const QString &folderPath)
{
    return QDir(folderPath).exists();
}

void ProductController::exportCurrentProductSeparately(const QString &path)
{
    const auto product = currentProduct();
    m_exportProduct->start(path, m_scanfieldPath, product);
}

QString ProductController::absolutePathToProduct(const QString &path)
{
    std::vector<QString> products;

    QDirIterator productJsonIterator{path, {"*.json"}, QDir::Files};
    // Check files on removable device
    while (productJsonIterator.hasNext())
    {
        productJsonIterator.next();
        products.push_back(productJsonIterator.fileInfo().baseName());
    }

    if (products.size() != 1)
    {
        return QLatin1String("");
    }

    auto productUuidString = products.front();
    // check uuid of the products in the system
    const QUuid uuid(productUuidString);

    if (auto productInController = findProduct(uuid))
    {
       return QFileInfo(productInController->filePath()).absolutePath();
    }

    if (auto productInProductModel = findProduct(uuid, m_productModel->products()))
    {
        return QFileInfo(productInProductModel->filePath()).absolutePath();
    }

    return QLatin1String("");
}

void ProductController::importSeparatedProduct(const QString &path)
{
    setCopyInProgress(true);

    const auto &productSubfolderName = RemovableDevicePaths::instance()->separatedProductJsonDir();
    const auto &referenceCurveSubfolderName = RemovableDevicePaths::instance()->separatedProductReferenceCurveDir();
    const auto &scanFieldImageFolderName = RemovableDevicePaths::instance()->separatedProductScanFieldImagesDir();

    std::shared_ptr<bool> toCopyScanfieldImages = std::make_shared<bool>(true);
    QFuture<void> future = QtConcurrent::run(
        [this, toCopyScanfieldImages, path, referenceCurveSubfolderName, productSubfolderName]()
        {
            // check files on removable device
            std::vector<QString> products;
            QString fullProductFolderName = path + "/" + productSubfolderName;
            QDirIterator productJsonIterator{fullProductFolderName, {"*.json"}, QDir::Files};

            while (productJsonIterator.hasNext())
            {
                productJsonIterator.next();
                products.push_back(productJsonIterator.fileInfo().baseName());
            }

            if (products.size() == 0)
            {
                NotificationSystem::instance()->warning(
                    tr("Cannot locate .json file in \"%1\". Import failed!").arg(fullProductFolderName));
                setCopyInProgress(false);
                *toCopyScanfieldImages = false;
                return;
            }

            if (products.size() > 1)
            {
                NotificationSystem::instance()->warning(
                    tr("Multiple .json files found in \"%1\". Import failed!").arg(fullProductFolderName));
                setCopyInProgress(false);
                *toCopyScanfieldImages = false;
                return;
            }

            const auto productUuidString = products.front();
            const QFileInfo sourceReferenceCurveFileName(path + "/" + referenceCurveSubfolderName + "/" +
                                                         productUuidString + ".ref");

            if (!sourceReferenceCurveFileName.exists())
            {
                NotificationSystem::instance()->error(tr("Cannot locate .ref file in \"%1\". Import failed!")
                                                          .arg(sourceReferenceCurveFileName.absoluteFilePath()));
                setCopyInProgress(false);
                *toCopyScanfieldImages = false;
                return;
            }

            // Checks existence of a product in the system with the same uuid as on removable device
            const auto uuid(productUuidString);
            std::optional<int> type;
            if (auto productInController = findProduct(uuid),
                productInProductModel = findProduct(uuid, m_productModel->products());
                productInController != nullptr || productInProductModel != nullptr)
            {
                if (productInController)
                {
                    type = productInController->type();
                }
                else if (productInProductModel)
                {
                    type = productInProductModel->type();
                }
                deleteProduct(uuid);
            }

            const QDir probableProductFileOnDisk(m_productModel->productStorageDirectory().absolutePath() + "/" +
                                                 productUuidString + ".json");
            if (probableProductFileOnDisk.exists())
            {
                NotificationSystem::instance()->error(tr("Cannot remove existing product file .json. Import failed!"));
                setCopyInProgress(false);
                *toCopyScanfieldImages = false;
                return;
            }

            auto productInMemory = importProduct(fullProductFolderName + "/" + productUuidString + ".json");
            if (productInMemory == nullptr)
            {
                NotificationSystem::instance()->warning(
                    tr("Cannot read product \"%1\" from removable device. Import failed!"));
                setCopyInProgress(false);
                *toCopyScanfieldImages = false;
                return;
            }
            if (!type.has_value())
            {
                type = productInMemory->type();
            }
            if (!isTypeValid(productInMemory, type.value()))
            {
                type = nextType();
            }

            productInMemory->setType(type.value());

            if (!productInMemory->save())
            {
                NotificationSystem::instance()->warning(
                    tr("Cannot save product \"%1\". Import failed!").arg(productSubfolderName));
                setCopyInProgress(false);
                *toCopyScanfieldImages = false;
                return;
            }

            const auto targetReferenceCurveFileName = productUuidString + ".ref";

            if (QDir dir(m_productModel->referenceCurveStorageDirectory()); dir.exists(targetReferenceCurveFileName))
            {
                dir.remove(targetReferenceCurveFileName);
            }

            const auto targetReferenceCurvePath =
                m_productModel->referenceCurveStorageDirectory() + targetReferenceCurveFileName;

            if (!QFile::copy(sourceReferenceCurveFileName.absoluteFilePath(), targetReferenceCurvePath))
            {
                NotificationSystem::instance()->warning(
                    tr("Cannot import product reference curve file \"%1\" on the disk.").arg(productSubfolderName));
                setCopyInProgress(false);
                *toCopyScanfieldImages = false;
                return;
            }

            *toCopyScanfieldImages = true;
            return;
      });

    auto watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished,[this, path, scanFieldImageFolderName, toCopyScanfieldImages]()
        {
            if (*toCopyScanfieldImages)
            {
                m_copyService->clear();
                const auto sourceScanFieldImageFolder(path + "/" + scanFieldImageFolderName);

                QDirIterator scanfieldDirIterator{sourceScanFieldImageFolder, QDir::AllDirs | QDir::NoDotAndDotDot,
                                                  QDirIterator::Subdirectories};
                while (scanfieldDirIterator.hasNext())
                {
                    scanfieldDirIterator.next();
                    auto folderName = scanfieldDirIterator.fileName();
                    QDir targetScanFieldImageFolder(m_scanfieldPath + folderName);

                    if (targetScanFieldImageFolder.exists())
                    {
                        targetScanFieldImageFolder.removeRecursively();
                    }
                    m_copyService->addElement(sourceScanFieldImageFolder + "/" + folderName,
                                              targetScanFieldImageFolder.path());
                }

                if (m_copyService->size() == 0)
                {
                    setCopyInProgress(false);
                    NotificationSystem::instance()->information(tr("Files are copied successfully."));
                }
                else
                {
                    m_copyService->performCopy();
                }
            }
        });
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);
    watcher->setFuture(future);
}

void ProductController::setCopyInProgress(bool isCopying)
{
    if (m_copyService != nullptr)
    {
        isCopying = isCopying || m_copyServiceInProgress;
    }
    if (m_copyServiceInProgress == isCopying)
    {
        return;
    }
    m_copyServiceInProgress = isCopying;
    emit copyInProgressChanged();
}


void ProductController::deleteProduct(const QUuid &id)
{
    if (!m_productModel)
    {
        return;
    }

    // first check in new products
    auto it = std::find_if(m_newProducts.begin(), m_newProducts.end(), hasUuid(id));
    if (it != m_newProducts.end())
    {
        // just delete the product, not yet stored to persistent storage
        const bool wasEmpty = m_changes.empty();
        const int row = rowOf(id);
        beginRemoveRows(QModelIndex{}, row, row);
        auto p = *it;
        m_changes.erase(id);
        p->deleteLater();
        m_newProducts.erase(it);
        endRemoveRows();

        if (wasEmpty != m_changes.empty())
        {
            emit hasChangesChanged();
        }

        if (p == m_currentProduct)
        {
            m_currentProduct = nullptr;
            m_currentSeam = nullptr;
            emit currentSeamChanged();
            emit currentProductChanged();
        }

        // nothing else to do
        return;
    }

    if (auto p = m_productModel->findProduct(id))
    {
        UserLog::instance()->addChange(new ProductDeletedChangeEntry{p});
        QFile file(p->filePath());
        file.remove();
    }
    // model updates when product model reports removal
}

void ProductController::discardChanges(const QUuid& id)
{
    // unselect current product
    if (m_currentProduct && m_currentProduct->uuid() == id)
    {
        m_currentProduct = nullptr;
        m_currentSeam = nullptr;
        m_currentSeamInterval = nullptr;
        m_currentSeamSeries = nullptr;
        emit currentSeamIntervalChanged();
        emit currentSeamChanged();
        emit currentSeamSeriesChanged();
        emit currentProductChanged();
    }

    // remove from changes
    if (!m_changes.empty())
    {
        m_changes.erase(id);
        if (m_changes.empty())
        {
            emit hasChangesChanged();
        }
    }

    // delete product from modified products
    const auto row = rowOf(id);
    auto it = std::find_if(m_modifiedProducts.begin(), m_modifiedProducts.end(), hasUuid(id));
    if (it != m_modifiedProducts.end())
    {
        (*it)->deleteLater();
        m_modifiedProducts.erase(it);
        emit dataChanged(index(row, 0), index(row, 0));
    } else
    {
        // delete product from new products
        auto it = std::find_if(m_newProducts.begin(), m_newProducts.end(), hasUuid(id));
        if (it != m_newProducts.end())
        {
            // begin remove rows
            beginRemoveRows({}, row, row);
            (*it)->deleteLater();
            m_newProducts.erase(it);
            endRemoveRows();
        }
    }
}

void ProductController::discardCopyWhenUnmodified()
{
    if (!m_currentProduct)
    {
        return;
    }

    const auto & id = m_currentProduct->uuid();
    const auto foundIt = m_changes.find(id);
    if (foundIt != m_changes.end())
    {
        // The product was changed, so we must keep the modified copy.
        return;
    }
    // unselect current product
    m_currentProduct = nullptr;
    m_currentSeam = nullptr;
    m_currentSeamInterval = nullptr;
    m_currentSeamSeries = nullptr;
    emit currentSeamIntervalChanged();
    emit currentSeamChanged();
    emit currentSeamSeriesChanged();
    emit currentProductChanged();

    const auto row = rowOf(id);
    const auto removeIt = std::find_if(m_modifiedProducts.begin(), m_modifiedProducts.end(), hasUuid(id));
    if (removeIt != m_modifiedProducts.end())
    {
        (*removeIt)->deleteLater();
        m_modifiedProducts.erase(removeIt);
        emit dataChanged(index(row, 0), index(row, 0));
    }
}

static const QUuid s_videoRecorderEnabledType{QByteArrayLiteral("96599c45-4e20-4aaa-826d-25463670dd09")};
static const QString s_videoRecorderEnabled{QStringLiteral("IsEnabled")};

QVariant ProductController::isVideoRecorderEnabled(const QUuid &id) const
{
    auto p = findProduct(id);
    if (!p)
    {
        return {};
    }
    auto hwParams = p->hardwareParameters();
    if (!hwParams)
    {
        return {};
    }
    if (auto parameter = hwParams->findByNameAndTypeId(s_videoRecorderEnabled, s_videoRecorderEnabledType))
    {
        return parameter->value();
    }
    return {};
}

void ProductController::setVideoRecorderEnabled(const QUuid &id, const QVariant &value)
{
    auto p = findProduct(id);
    if (!p)
    {
        return;
    }
    auto hwParams = p->hardwareParameters();
    if (!hwParams)
    {
        if (!value.isValid())
        {
            // no hardware parameters and we would remove -> nothing to do
            return;
        }
        // create hardware parameters
        p->setHardwareParameters(new ParameterSet{QUuid::createUuid(), p});
        hwParams = p->hardwareParameters();
    }

    if (auto parameter = hwParams->findByNameAndTypeId(s_videoRecorderEnabled, s_videoRecorderEnabledType))
    {
        if (value.isValid())
        {
            parameter->setValue(value);
        } else
        {
            hwParams->removeParameter(parameter);
        }
    } else
    {
        // parameter not found
        if (!value.isValid())
        {
            // nothing to do
            return;
        }
        // create a parameter
        auto newParameter = new Parameter{QUuid::createUuid(), hwParams};
        newParameter->setTypeId(s_videoRecorderEnabledType);
        newParameter->setType(Parameter::DataType::Boolean);
        newParameter->setName(s_videoRecorderEnabled);
        newParameter->setValue(value);
        hwParams->addParameter(newParameter);
    }
    markAsChanged();
}

bool ProductController::isTypeValid(precitec::storage::Product *p, int type) const
{
    auto checkProducts = [p, type] (auto products)
    {
        auto it = std::find_if(products.begin(), products.end(), [type] (auto p) { return p->type() == type; });
        if (it != products.end())
        {
            if ((*it)->uuid() != p->uuid())
            {
                return true;
            }
        }
        return false;
    };
    if (m_productModel)
    {
        if (checkProducts(m_productModel->products()))
        {
            return false;
        }
    }
    if (checkProducts(m_modifiedProducts) || checkProducts(m_newProducts))
    {
        return false;
    }

    return true;
}

int ProductController::currentProductIndex()
{
    if (!m_currentProduct)
    {
        return -1;
    }
    return rowOf(m_currentProduct->uuid());
}

precitec::storage::Product *ProductController::importFromXml(const QString &path)
{
    auto product = Product::fromXml(path, this);
    if (!product)
    {
        return nullptr;
    }
    if (findProduct(product->uuid()))
    {
        NotificationSystem::instance()->information(tr("The product was already imported"));
        delete product;
        return nullptr;
    }
    if (!isTypeValid(product, product->type()))
    {
        product->setType(nextType());
        NotificationSystem::instance()->information(tr("Type of imported product adjusted"));
    }
    product->setReferenceCurveStorageDir(m_productModel->referenceCurveStorageDirectory());
    product->setFilePath(m_productModel->productStorageDirectory().absoluteFilePath(product->uuid().toString(QUuid::WithoutBraces) + QStringLiteral(".json")));

    const int start = m_productModel->rowCount();

    beginInsertRows(QModelIndex(), start + m_newProducts.size(), start + m_newProducts.size());
    m_newProducts.push_back(product);
    endInsertRows();

    addChange(product->uuid());

    return product;
}

QStringList ProductController::productImportList(const QString &basePath)
{
    QStringList products;
    QDirIterator it{basePath, QDirIterator::Subdirectories};
    while (it.hasNext())
    {
        it.next();
        const auto &fileInfo = it.fileInfo();
        if (fileInfo.isDir())
        {
            continue;
        }
        if (fileInfo.suffix().compare(QLatin1String("xml"), Qt::CaseInsensitive) != 0)
        {
            continue;
        }
        if (fileInfo.baseName().startsWith(QLatin1String("Produkt_")))
        {
           products << fileInfo.absoluteFilePath();
        }
    }
    return products;
}

QStringList ProductController::separatedProductImportList(const QString &path)
{
    return QDir(path).entryList();
}

void ProductController::checkProductSize(precitec::storage::Product* product)
{
    QFileInfo file{product->filePath()};
    const std::size_t sizeInBytes = file.size();
    if (sizeInBytes > s_maxProductFileSizeInBytes)
    {
        NotificationSystem::instance()->warning(tr("Product \"%1\" is larger than %2 MiB. Please consider decreasing size to improve system performance.").arg(product->name()).arg(s_maxProductFileSize));
    }

    const std::size_t direcotrySizeInBytes = gui::components::removableDevices::utils::directorySize(file.absolutePath());
    if (direcotrySizeInBytes > s_maxDirectorySizeInBytes)
    {
        NotificationSystem::instance()->warning(tr("All products occupy more than %1 MiB of storage. Please consider removing unused products to improve system performance.").arg(s_maxDirectorySize));
    }
}

void ProductController::manualTrigger(const QUuid &id,
    const QString& type,
    const QString& name,
    const QString& filePath)
{
    std::map<std::string, std::string> dataMap{
        {std::string{"uuid"}, id.toString(QUuid::WithoutBraces).toStdString()},
        {std::string{"type"}, type.toStdString()},
        {std::string{"name"}, name.toStdString()},
        {std::string{"filePath"}, filePath.toStdString()},
    };
    m_schedulerEventsProxy->schedulerEventFunction(SchedulerEvents::ProductManualTrigger, dataMap);
};

void ProductController::setSchedulerEventsProxy(SchedulerEventsProxy proxy)
{
    if (m_schedulerEventsProxy == proxy)
    {
        return;
    }
    m_schedulerEventsProxy = proxy;

    emit schedulerEventsProxyChanged();
}


Poco::UUID convertQUuidToPoco(const QUuid& qUuid) {
    // Convert QUuid to string
    QString uuidString = qUuid.toString();

    // Remove braces if present
    uuidString = uuidString.remove("{").remove("}");

    // Convert QString to std::string
    std::string stdUuidString = uuidString.toStdString();

    // Create Poco::UUID from std::string
    Poco::UUID pocoUuid(stdUuidString);

    return pocoUuid;
}

}
}
