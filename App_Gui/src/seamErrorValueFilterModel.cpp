#include "seamErrorValueFilterModel.h"
#include "seam.h"
#include "seamSeries.h"
#include "parameter.h"
#include "parameterSet.h"
#include "product.h"
#include "graphFunctions.h"
#include "graphModel.h"
#include "subGraphModel.h"

using precitec::storage::Seam;
using precitec::storage::GraphModel;
using precitec::storage::SubGraphModel;
using precitec::storage::graphFunctions::getGraphFromModel;

static std::map<int, QString> s_inspectionControlKeys {
    {20102, QStringLiteral("Inspect manager processing mode")}
};

namespace precitec
{
namespace gui
{

SeamErrorValueFilterModel::SeamErrorValueFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &SeamErrorValueFilterModel::currentSeamChanged, this, &SeamErrorValueFilterModel::updateFilterKeys);
    connect(this, &SeamErrorValueFilterModel::graphModelChanged, this, &SeamErrorValueFilterModel::updateFilterKeys);
    connect(this, &SeamErrorValueFilterModel::subGraphModelChanged, this, &SeamErrorValueFilterModel::updateFilterKeys);
    connect(this, &SeamErrorValueFilterModel::filterKeysChanged, this, &SeamErrorValueFilterModel::invalidate);
    connect(this, &SeamErrorValueFilterModel::sortOrderChanged, this, &SeamErrorValueFilterModel::invalidate);
    connect(this, &SeamErrorValueFilterModel::searchTextChanged, this, &SeamErrorValueFilterModel::invalidate);
    setSortRole(Qt::UserRole + 1);
    sort(0, Qt::AscendingOrder);
}

SeamErrorValueFilterModel::~SeamErrorValueFilterModel() = default;

bool SeamErrorValueFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.isValid())
    {
        return false;
    }
    const auto variant = sourceIndex.data(Qt::DisplayRole);
    if (!variant.isValid())
    {
        return false;
    }
    if (std::find(m_filterKeys.begin(), m_filterKeys.end(), variant.toInt()) != m_filterKeys.end()
        || std::find_if(s_inspectionControlKeys.begin(), s_inspectionControlKeys.end(), [&variant] (auto inspectKey) { return inspectKey.first == variant.toInt(); }) != s_inspectionControlKeys.end())
    {
        if (m_searchText.isEmpty())
        {
            return true;
        }

        if (sourceIndex.data(Qt::UserRole + 1).toString().contains(m_searchText, Qt::CaseInsensitive))
        {
            return true;
        }

        auto ok = false;
        const auto number = m_searchText.toInt(&ok);
        if (!ok)
        {
            return false;
        }
        return sourceIndex.data(Qt::DisplayRole).toInt() == number;
    }

    return false;
}

void SeamErrorValueFilterModel::setSearchText(const QString& searchText)
{
    if (m_searchText == searchText)
    {
        return;
    }
    m_searchText = searchText;
    emit searchTextChanged();
}

int SeamErrorValueFilterModel::findIndex(int value)
{
    for (int i = 0; i < sourceModel()->rowCount({}); i++)
    {
        if (index(i, 0).data().toInt() == value)
        {
            return i;
        }
    }
    return -1;
}

void SeamErrorValueFilterModel::setSortOrder(Qt::SortOrder sortOrder)
{
    sort(0, sortOrder);
    emit sortOrderChanged();
}

bool SeamErrorValueFilterModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    QVariant leftData = sourceModel()->data(source_left, sortRole());
    QVariant rightData = sourceModel()->data(source_right, sortRole());

    switch (sortRole())
    {
        case Qt::DisplayRole:
             return leftData.toInt() <= rightData.toInt();
        case Qt::UserRole + 1:
             return leftData.toString().compare(rightData.toString(), Qt::CaseInsensitive) <= 0;
        case Qt::UserRole + 7:
             return leftData.toInt() <= rightData.toInt();
        default:
             return true;
    }
}

void SeamErrorValueFilterModel::setCurrentSeam(Seam *seam)
{
    if (m_currentSeam == seam)
    {
        return;
    }

    if (m_currentSeam)
    {
        disconnect(m_currentSeam, &Seam::graphChanged, this, &SeamErrorValueFilterModel::updateFilterKeys);
        disconnect(m_currentSeam, &Seam::subGraphChanged, this, &SeamErrorValueFilterModel::updateFilterKeys);
        disconnect(m_destroyConnection);
    }

    m_currentSeam = seam;

    if (m_currentSeam)
    {
        m_destroyConnection = connect(m_currentSeam, &QObject::destroyed, this, std::bind(&SeamErrorValueFilterModel::setCurrentSeam, this, nullptr));
        connect(m_currentSeam, &Seam::graphChanged, this, &SeamErrorValueFilterModel::updateFilterKeys);
        connect(m_currentSeam, &Seam::subGraphChanged, this, &SeamErrorValueFilterModel::updateFilterKeys);
    } else
    {
        m_destroyConnection = {};
    }

    emit currentSeamChanged();
}

void SeamErrorValueFilterModel::updateFilterKeys()
{
    m_filterKeys.clear();

    if (!m_currentSeam || !m_graphModel || !m_subGraphModel)
    {
        emit filterKeysChanged();
        return;
    }

    const auto& graph = getGraph();
    for (const auto& instanceFilter : graph.instanceFilters)
    {
        for (const auto& attribute : instanceFilter.attributes)
        {
            if (QString::fromStdString(attribute.name).compare(QLatin1String("ResultType")) == 0)
            {
                const auto filterKey = attribute.value.convert<int>();
                if (std::none_of(m_filterKeys.cbegin(), m_filterKeys.cend(), [&filterKey] (auto key) { return filterKey == key; }))
                {
                    m_filterKeys.push_back(filterKey);
                }
            }
        }
    }

    emit filterKeysChanged();
}

void SeamErrorValueFilterModel::setGraphModel(GraphModel* graphModel)
{
    if (m_graphModel == graphModel)
    {
        return;
    }
    disconnect(m_graphModelDestroyedConnection);
    m_graphModel = graphModel;
    if (m_graphModel)
    {
        m_graphModelDestroyedConnection = connect(m_graphModel, &QObject::destroyed, this, std::bind(&SeamErrorValueFilterModel::setGraphModel, this, nullptr));
    } else
    {
        m_graphModelDestroyedConnection = {};
    }
    emit graphModelChanged();
}

void SeamErrorValueFilterModel::setSubGraphModel(SubGraphModel* subGraphModel)
{
    if (m_subGraphModel == subGraphModel)
    {
        return;
    }
    disconnect(m_subGraphModelDestroyedConnection);
    m_subGraphModel = subGraphModel;
    if (m_subGraphModel)
    {
        m_subGraphModelDestroyedConnection = connect(m_subGraphModel, &QObject::destroyed, this, std::bind(&SeamErrorValueFilterModel::setSubGraphModel, this, nullptr));
    } else
    {
        m_subGraphModelDestroyedConnection = {};
    }
    emit subGraphModelChanged();
}

fliplib::GraphContainer SeamErrorValueFilterModel::getGraph() const
{
    return getGraphFromModel(m_currentSeam, m_graphModel, m_subGraphModel);
}

}
}

