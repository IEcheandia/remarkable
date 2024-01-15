#include "productInstanceSeamSortModel.h"
#include "productInstanceSeamModel.h"
#include "seamSeries.h"
#include "seamSeriesMetaData.h"

#include <QDir>

namespace precitec
{
namespace storage
{

ProductInstanceSeamSortModel::ProductInstanceSeamSortModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    sort(0);
    connect(this, &ProductInstanceSeamSortModel::productInstanceSeamModelChanged, this, &ProductInstanceSeamSortModel::init);
}

ProductInstanceSeamSortModel::~ProductInstanceSeamSortModel() = default;

void ProductInstanceSeamSortModel::setProductInstanceSeamModel(precitec::storage::ProductInstanceSeamModel* model)
{
    if (m_sourceModel == model)
    {
        return;
    }
    m_sourceModel = model;
    setSourceModel(m_sourceModel);

    for (const auto &connection : m_sourceModelConnections)
    {
        disconnect(connection);
    }
    m_sourceModelConnections.clear();
    m_sourceModelConnections.reserve(3);

    if (m_sourceModel)
    {
        m_sourceModelConnections.emplace_back(connect(m_sourceModel, &QObject::destroyed, this, std::bind(&ProductInstanceSeamSortModel::setProductInstanceSeamModel, this, nullptr)));
        m_sourceModelConnections.emplace_back(connect(m_sourceModel, &ProductInstanceSeamModel::modelReset, this, &ProductInstanceSeamSortModel::init));
        m_sourceModelConnections.emplace_back(connect(m_sourceModel, &ProductInstanceSeamModel::metaDataLoadFinished, this, &ProductInstanceSeamSortModel::invalidate));
    }
    emit productInstanceSeamModelChanged();
}

bool ProductInstanceSeamSortModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    const auto &left = source_left.data(Qt::UserRole + 4).toUuid();
    const auto &right = source_right.data(Qt::UserRole + 4).toUuid();
    const auto leftIt = std::find(m_uuids.begin(), m_uuids.end(), left);
    const auto rightIt = std::find(m_uuids.begin(), m_uuids.end(), right);
    if (leftIt == rightIt)
    {
        return source_left.data(Qt::DisplayRole).toInt() < source_right.data(Qt::DisplayRole).toInt();
    }
    return std::distance(m_uuids.begin(), leftIt) < std::distance(m_uuids.begin(), rightIt);
}

void ProductInstanceSeamSortModel::init()
{
    m_uuids.clear();
    if (!m_sourceModel || !m_sourceModel->seamSeries())
    {
        return;
    }

    const auto metaData = SeamSeriesMetaData::parse(m_sourceModel->productInstance(), m_sourceModel->seamSeries()->number());
    if (metaData.isSeamsValid())
    {
        m_uuids.reserve(metaData.seams().size());
        std::transform(metaData.seams().begin(), metaData.seams().end(), std::back_inserter(m_uuids), [] (const auto& metaData) { return metaData.uuid(); });
        invalidate();
    }
}

}
}
