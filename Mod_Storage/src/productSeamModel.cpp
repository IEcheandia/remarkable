#include "productSeamModel.h"
#include "product.h"
#include "seam.h"
#include "seamSeries.h"
#include "linkedSeam.h"

namespace precitec
{
namespace storage
{

ProductSeamModel::ProductSeamModel(QObject* parent)
    : QAbstractListModel(parent)
{
    connect(this, &ProductSeamModel::productChanged, this, &ProductSeamModel::initSeams);
}

ProductSeamModel::~ProductSeamModel() = default;

QVariant ProductSeamModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return QVariant::fromValue(m_seams.at(index.row()));
    case Qt::UserRole:
        return QVariant::fromValue(m_seams.at(index.row())->seamSeries());
    case Qt::UserRole + 1:
        return m_seams.at(index.row())->seamSeries()->name() + QStringLiteral(" (") + QString::number(m_seams.at(index.row())->seamSeries()->visualNumber()) + QStringLiteral(")");
    case Qt::UserRole + 2:
        return m_seams.at(index.row())->metaObject()->inherits(&LinkedSeam::staticMetaObject);
    default:
        break;
    }

    return {};
}

QHash<int, QByteArray> ProductSeamModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("seam")},
        {Qt::UserRole, QByteArrayLiteral("seamSeries")},
        {Qt::UserRole + 1, QByteArrayLiteral("seamSeriesName")},
        {Qt::UserRole +2, QByteArrayLiteral("linkedSeam")}
    };
}

int ProductSeamModel::rowCount(const QModelIndex &parent) const
{
    if (!m_product || parent.isValid())
    {
        return 0;
    }

    return m_seams.size();
}

void ProductSeamModel::setProduct(precitec::storage::Product* product)
{
    if (m_product == product)
    {
        return;
    }
    disconnect(m_productDestroyedConnection);
    disconnect(m_seamChangedConnection);
    m_product = product;
    if (m_product)
    {
        m_productDestroyedConnection = connect(product, &QObject::destroyed, this, std::bind(&ProductSeamModel::setProduct, this, nullptr));
        m_seamChangedConnection = connect(product, &Product::seamsChanged, this, &ProductSeamModel::initSeams);
    }
    emit productChanged();
}

void ProductSeamModel::initSeams()
{
    beginResetModel();
    m_seams.clear();
    if (m_product)
    {
        const auto seamSeries = m_product->seamSeries();
        for (auto series : seamSeries)
        {
            const auto &seams = series->seams();
            m_seams.reserve(m_seams.size() + seams.size());
            std::copy(seams.begin(), seams.end(), std::back_inserter(m_seams));
        }
    }
    endResetModel();
}

}
}
