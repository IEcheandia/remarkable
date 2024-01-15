#include "productModel.h"
#include "product.h"
#include "seamSeries.h"
#include "referenceCurve.h"

#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMutexLocker>

namespace precitec
{
namespace storage
{

ProductModel::ProductModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_jsonWatcher(new QFileSystemWatcher(this))
{
    connect(m_jsonWatcher, &QFileSystemWatcher::fileChanged, this, &ProductModel::updateProduct);
    connect(m_jsonWatcher, &QFileSystemWatcher::fileChanged, this, &ProductModel::updateReferenceCurves);
    connect(m_jsonWatcher, &QFileSystemWatcher::fileChanged, this, &ProductModel::updateScanfieldImages);
    connect(m_jsonWatcher, &QFileSystemWatcher::directoryChanged, this, &ProductModel::checkNewProducts);
    connect(m_jsonWatcher, &QFileSystemWatcher::directoryChanged, this, &ProductModel::updateReferenceCurves);
    connect(m_jsonWatcher, &QFileSystemWatcher::directoryChanged, this, &ProductModel::updateScanfieldImages);
    connect(this, &ProductModel::referenceCurveStorageDirectoryChanged, this, &ProductModel::updateReferenceCurves);
    connect(this, &ProductModel::scanfieldImageStorageDirectoryChanged, this, &ProductModel::updateScanfieldImages);
}

ProductModel::~ProductModel() = default;

int ProductModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_products.size();
}

QVariant ProductModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return m_products.at(index.row())->name();
    case Qt::UserRole:
        return m_products.at(index.row())->uuid();
    case Qt::UserRole+1:
        return QVariant::fromValue(m_products.at(index.row()));
    default:
        return QVariant();
    }
}

void ProductModel::loadProducts(const QDir &productStorageDirectory)
{
    QMutexLocker lock{&m_storageMutex};
    m_productStorageDirectory = productStorageDirectory;
    beginResetModel();
    qDeleteAll(m_products);
    m_products.clear();

    clearJsonWatcher();

    const auto files = productStorageDirectory.entryInfoList(QStringList{QStringLiteral("*.json")}, QDir::Files | QDir::Readable);
    for (const auto &file : files)
    {
        auto product = Product::fromJson(file.absoluteFilePath(), this);
        if (product)
        {
            product->setReferenceCurveStorageDir(m_referenceCurveStorageDirectory);
            m_products.push_back(product);
            m_jsonWatcher->addPath(product->filePath());
        }
    }
    m_jsonWatcher->addPath(m_productStorageDirectory.absolutePath());

    std::sort(m_products.begin(), m_products.end(), [] (auto product1, auto product2) {
        return product1->type() < product2->type();
    });

    endResetModel();
}

void ProductModel::clearJsonWatcher()
{
    const auto directories = m_jsonWatcher->directories();
    if (!directories.isEmpty())
    {
        m_jsonWatcher->removePaths(directories);
    }
    const auto files = m_jsonWatcher->files();
    if (!files.empty())
    {
        m_jsonWatcher->removePaths(files);
    }
}

QHash<int, QByteArray> ProductModel::roleNames() const
{
    return QHash<int, QByteArray>{
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("uuid")},
        {Qt::UserRole+1, QByteArrayLiteral("product")}
    };
}

QUuid ProductModel::uuid(const QModelIndex &index) const
{
    return data(index, Qt::UserRole).toUuid();
}

Product *ProductModel::findProduct(const QUuid &uuid) const
{
    auto it = std::find_if(m_products.begin(), m_products.end(), [uuid] (auto product) { return product->uuid() == uuid; });
    if (it != m_products.end())
    {
        return *it;
    }
    return nullptr;
}

Product *ProductModel::defaultProduct() const
{
    auto it = std::find_if(m_products.begin(), m_products.end(), [] (auto product) { return product->isDefaultProduct(); });
    if (it != m_products.end())
    {
        return *it;
    }
    return nullptr;
}

ParameterSet *ProductModel::findFilterParameterSet(const QUuid &uuid) const
{
    for (auto p: m_products)
    {
        if (auto ps = p->filterParameterSet(uuid))
        {
            return ps;
        }
    }
    return nullptr;
}

void ProductModel::ensureAllFilterParameterSetsLoaded(const QUuid& referenceParameterSet)
{
    for (auto p : m_products)
    {
        if (p->containsFilterParameterSet(referenceParameterSet))
        {
            p->ensureAllFilterParameterSetsLoaded();
        }
    }
}

void ProductModel::updateProduct(const QString &path)
{
    QMutexLocker lock{&m_storageMutex};
    auto it = std::find_if(m_products.begin(), m_products.end(), [path] (auto p) { return p->filePath() == path; });
    if (it == m_products.end())
    {
        // not found, ignore
        return;
    }
    if (auto product = Product::fromJson(path, this))
    {
        const auto i = index(std::distance(m_products.begin(), it));
        (*it)->deleteLater();
        *it = product;
        product->setReferenceCurveStorageDir(m_referenceCurveStorageDirectory);
        m_jsonWatcher->addPath(path);
        emit dataChanged(i, i);
    }
}

void ProductModel::reloadProduct(const QUuid &uuid)
{
    if (auto p = findProduct(uuid))
    {
        updateProduct(p->filePath());
    }
}

void ProductModel::checkNewProducts()
{
    QMutexLocker lock{&m_storageMutex};
    // look through files
    const auto files = m_productStorageDirectory.entryInfoList(QStringList{QStringLiteral("*.json")}, QDir::Files | QDir::Readable);
    for (const auto &file : files)
    {
        // do we have a product with this file?
        const auto path = file.absoluteFilePath();
        auto it = std::find_if(m_products.begin(), m_products.end(), [path] (auto p) { return p->filePath() == path; });
        if (it == m_products.end())
        {
            // This is a new product
            auto product = Product::fromJson(path, this);
            if (product)
            {
                beginInsertRows({}, m_products.size(), m_products.size());
                product->setReferenceCurveStorageDir(m_referenceCurveStorageDirectory);
                m_products.push_back(product);
                endInsertRows();
                // adding the file to the QFileSystemWatcher is racy. In case the file got deleted again we won't detect updates and assume it exists.
                m_jsonWatcher->addPath(product->filePath());
            }
        }
    }

    // check for removed files
    for (std::size_t i = 0; i < m_products.size(); i++)
    {
        auto product = m_products.at(i);
        if (std::none_of(files.begin(), files.end(), [product] (const QFileInfo &file) { return product->filePath() == file.absoluteFilePath(); }))
        {
            beginRemoveRows({}, i, i);
            auto it = m_products.begin();
            std::advance(it, i);
            m_products.erase(it);
            product->deleteLater();
            m_jsonWatcher->removePath(product->filePath());
            endRemoveRows();
        }
    }
}

AbstractMeasureTask *ProductModel::findMeasureTask(const QUuid& id) const
{
    for (auto product : m_products)
    {
        if (auto measureTask = product->findMeasureTask(id))
        {
            return measureTask;
        }
    }
    return nullptr;
}

void ProductModel::setReferenceStorageDirectory(const QString& dir)
{
    if (m_referenceCurveStorageDirectory == dir)
    {
        return;
    }
    m_referenceCurveStorageDirectory = dir;
    for (auto product : m_products)
    {
        product->setReferenceCurveStorageDir(m_referenceCurveStorageDirectory);
    }
    emit referenceCurveStorageDirectoryChanged();
}

void ProductModel::setScanfieldImageStorageDirectory(const QString& dir)
{
    if (m_scanfieldImageStorageDirectory == dir)
    {
        return;
    }
    m_scanfieldImageStorageDirectory = dir;
    emit scanfieldImageStorageDirectoryChanged();
}

void ProductModel::updateReferenceCurves()
{
    if (m_referenceCurveStorageDirectory.isEmpty() || !m_cleanupEnabled)
    {
        return;
    }

    QDir dir{m_referenceCurveStorageDirectory};
    if (!dir.exists())
    {
        return;
    }

    if (m_products.empty())
    {
        return;
    }

    std::vector<ReferenceCurve*> curves;

    for (auto product : m_products)
    {
        const auto& refCurves = product->allReferenceCurves();
        curves.reserve(curves.size() + refCurves.size());
        for (auto curve : refCurves)
        {
            curves.push_back(curve);
        }
    }

    const auto& files = dir.entryInfoList(QStringList{QStringLiteral("*.ref")}, QDir::Files | QDir::Readable);
    for (const auto& file : files)
    {
        const auto& fileName = file.baseName();
        if (std::none_of(m_products.begin(), m_products.end(), [&fileName] (auto product) { return product->uuid().toString(QUuid::WithoutBraces) == fileName; })
            && std::none_of(curves.begin(), curves.end(), [&fileName] (auto curve) { return curve->uuid().toString(QUuid::WithoutBraces) == fileName; })
        )
        {
            dir.remove(file.fileName());
        }
    }
}

void ProductModel::updateScanfieldImages()
{
    if (m_scanfieldImageStorageDirectory.isEmpty() || !m_cleanupEnabled)
    {
        return;
    }

    QDir dir{m_scanfieldImageStorageDirectory};
    if (!dir.exists())
    {
        return;
    }

    if (m_products.empty())
    {
        return;
    }

    std::vector<SeamSeries*> seamSeries;

    for (auto product : m_products)
    {
        const auto& series = product->seamSeries();
        seamSeries.reserve(series.size() + seamSeries.size());
        for (auto s : series)
        {
            seamSeries.push_back(s);
        }
    }

    const auto& directories = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);

    for (const auto& directory : directories)
    {
        const auto& dirName = directory.baseName();
        if (std::none_of(seamSeries.begin(), seamSeries.end(), [&dirName] (auto series) { return series->uuid().toString(QUuid::WithoutBraces) == dirName; }))
        {
            QDir{directory.absoluteFilePath()}.removeRecursively();
        }
    }
}

void ProductModel::setCleanupEnabled(bool cleanupEnabled)
{
    if (m_cleanupEnabled == cleanupEnabled)
    {
        return;
    }
    m_cleanupEnabled = cleanupEnabled;
    emit cleanupEnabledChanged();
}

}
}
