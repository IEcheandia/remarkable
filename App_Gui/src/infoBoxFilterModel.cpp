#include "infoBoxFilterModel.h"

#include <QRect>

namespace precitec
{
namespace gui
{

InfoBoxFilterModel::InfoBoxFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &InfoBoxFilterModel::positionAdded, this, &InfoBoxFilterModel::invalidateFilter);
    connect(this, &InfoBoxFilterModel::multipleSelectionChanged, this, &InfoBoxFilterModel::invalidateFilter);
    connect(this, &InfoBoxFilterModel::sourceModelChanged, this, &InfoBoxFilterModel::invalidateFilter);
    connect(this, &InfoBoxFilterModel::modelReset, this, &InfoBoxFilterModel::invalidateFilter);
}

InfoBoxFilterModel::~InfoBoxFilterModel() = default;

void InfoBoxFilterModel::addPosition(const QPointF& position)
{
    if (!m_multipleSelection)
    {
        m_positions.clear();
    }

    auto it = std::find_if(m_positions.begin(), m_positions.end(), [position] (auto pos)
    {
       return qFuzzyCompare(pos.x(), position.x()) && qFuzzyCompare(pos.y(), position.y());
    });

    if (it == m_positions.end())
    {
        m_positions.push_back(position);
        emit positionAdded();
    }
}

void InfoBoxFilterModel::setMultipleSelection(bool set)
{
    if (m_multipleSelection == set)
    {
        return;
    }
    m_multipleSelection = set;

    if (!m_multipleSelection && !m_positions.empty())
    {
        const auto lastElement = m_positions.back();
        m_positions.clear();
        m_positions.push_back(lastElement);
    }

    emit multipleSelectionChanged();
}

bool InfoBoxFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    auto index = sourceModel()->index(source_row, 0, source_parent);
    const auto bounds = index.data(Qt::UserRole + 1).toRectF();

    if (m_positions.empty())
    {
        return true;
    }

    if (!bounds.isValid())
    {
       return false;
    }

    auto result = false;
    for (auto position : m_positions)
    {
        result = bounds.contains(position);
        if (result)
        {
            break;
        }
    }
    return result;
}

void InfoBoxFilterModel::clear()
{
    m_positions.clear();
    emit positionAdded();
}

}
}



