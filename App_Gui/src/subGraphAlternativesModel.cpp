#include "subGraphAlternativesModel.h"

namespace precitec
{
namespace gui
{

SubGraphAlternativesModel::SubGraphAlternativesModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &SubGraphAlternativesModel::selectedIndexChanged, this,
        [this]
        {
            if (m_subGraphModel)
            {
                m_sinks = m_subGraphModel->sinkBridges(m_selectedIndex);
            }
            else
            {
                m_sinks = {};
            }
            invalidate();
        }
    );
    connect(this, &SubGraphAlternativesModel::subGraphModelChanged, this, &SubGraphAlternativesModel::selectedIndexChanged);
}

SubGraphAlternativesModel::~SubGraphAlternativesModel() = default;

void SubGraphAlternativesModel::setSubGraphModel(precitec::storage::SubGraphModel *subGraphModel)
{
    if (m_subGraphModel == subGraphModel)
    {
        return;
    }
    m_subGraphModel = subGraphModel;
    disconnect(m_subGraphModelDestroyed);
    disconnect(m_subGraphModelDataChanged);
    if (m_subGraphModel)
    {
        m_subGraphModelDestroyed = connect(m_subGraphModel, &QObject::destroyed, this, std::bind(&SubGraphAlternativesModel::setSubGraphModel, this, nullptr));
        m_subGraphModelDataChanged = connect(m_subGraphModel, &storage::SubGraphModel::dataChanged, this,
            [this] (const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
            {
                Q_UNUSED(topLeft)
                Q_UNUSED(bottomRight)
                if (roles.empty() || std::find(roles.begin(), roles.end(), Qt::UserRole + 2) != roles.end())
                {
                    invalidate();
                }
            });
        setSourceModel(m_subGraphModel);
    }
    else
    {
        m_subGraphModelDestroyed = {};
        m_subGraphModelDataChanged = {};
    }

    emit subGraphModelChanged();
}

bool SubGraphAlternativesModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (source_parent.isValid())
    {
        return false;
    }
    if (!m_subGraphModel)
    {
        return false;
    }
    if (!m_selectedIndex.isValid())
    {
        return false;
    }
    if (m_sinks.empty())
    {
        return false;
    }

    const auto index = m_subGraphModel->index(source_row, 0);
    if (m_selectedIndex == index)
    {
        return false;
    }

    const auto &sourceBridges = m_subGraphModel->sourceBridges(index);
    for (const auto &sourceBridge : sourceBridges)
    {
        if (!m_subGraphModel->matchingBridge(sourceBridge))
        {
            return false;
        }
    }
    const auto &sinkBridges = m_subGraphModel->sinkBridges(index);
    for (const auto &sinkBridge : m_sinks)
    {
        if (std::none_of(sinkBridges.begin(), sinkBridges.end(), [&sinkBridge] (const auto &candidate) { return sinkBridge.dataType == candidate.dataType && sinkBridge.name == candidate.name && sinkBridge.instanceFilter != candidate.instanceFilter;}))
        {
            if (m_subGraphModel->isSinkBridgeUsed(sinkBridge))
            {
                return false;
            }
        }
    }

    return true;
}

void SubGraphAlternativesModel::setSelectedIndex(const QModelIndex &index)
{
    if (m_selectedIndex == index)
    {
        return;
    }
    m_selectedIndex = index;
    emit selectedIndexChanged();
}

}
}
