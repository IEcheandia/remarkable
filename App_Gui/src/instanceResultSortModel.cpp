#include "instanceResultSortModel.h"
#include "productInstanceModel.h"

#include <QTimer>

using precitec::storage::ProductInstanceModel;

namespace precitec
{
namespace gui
{

InstanceResultSortModel::InstanceResultSortModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_timer(new QTimer{this})
{
    // timer to delay dataChanged invalidate calls to the end of the event loop
    m_timer->setInterval(0);
    m_timer->setSingleShot(true);

    connect(m_timer, &QTimer::timeout, this, &InstanceResultSortModel::invalidate);

    setSortRole(Qt::DisplayRole);
    sort(0, Qt::DescendingOrder);

    connect(this, &InstanceResultSortModel::dataChanged, this,  [this] (const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
    {
        Q_UNUSED(topLeft)
        Q_UNUSED(bottomRight)

        // Qt::UserRole - date
        // Qt::UserRole + 1 - state
        if ((roles.contains(Qt::UserRole + 1) && m_filterType != FilterType::All) || (roles.contains(Qt::UserRole) && m_sortOnDate))
        {
            m_timer->start();
        }
    });
    connect(this, &InstanceResultSortModel::filterTypeChanged, this, &InstanceResultSortModel::invalidate);
    connect(this, &InstanceResultSortModel::includeLinkedSeamsChanged, this, &InstanceResultSortModel::invalidate);
    connect(this, &InstanceResultSortModel::sortOnDateChanged, this, [this] {
        // Qt::DisplayRole - serial number
        // Qt::UserRole - date
        setSortRole(m_sortOnDate ? Qt::DisplayRole : Qt::UserRole);
    });
}

InstanceResultSortModel::~InstanceResultSortModel() = default;

bool InstanceResultSortModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_UNUSED(source_parent)

    // Qt::UserRole + 1 - state
    // Qt::UserRole + 3 - linkedSeam
    const auto& state = sourceModel()->index(source_row, 0).data(Qt::UserRole + 1).value<ProductInstanceModel::State>();
    const auto& linkedSeam = sourceModel()->index(source_row, 0).data(Qt::UserRole + 3).toBool();

    if (!m_includeLinkedSeams && linkedSeam)
    {
        return false;
    }

    switch (m_filterType)
    {
        case FilterType::RemoveNIO:
            return state != ProductInstanceModel::State::Nio;
        case FilterType::OnlyNIO:
            return state == ProductInstanceModel::State::Nio;
        default:
            return true;
    }
}

bool InstanceResultSortModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    const auto& leftData = sourceModel()->data(source_left, sortRole());
    const auto& rightData = sourceModel()->data(source_right, sortRole());

    switch (sortRole())
    {
        // Qt::DisplayRole - serial number
        case Qt::DisplayRole:
        {
            const auto& left = leftData.toInt();
            const auto& right = rightData.toInt();
            if (left == right)
            {
                // Qt::UserRole + 4 - visualSeamNumber
                return source_left.data(Qt::UserRole + 4).toInt() <= source_right.data(Qt::UserRole + 4).toInt();
            }
            return left <= right;
        }
        // Qt::UserRole - date
        case Qt::UserRole:
            return leftData.toDateTime() <= rightData.toDateTime();
        default:
            return true;
    }
}

void InstanceResultSortModel::setSortOrder(Qt::SortOrder order)
{
    sort(0, order);
    emit sortOrderChanged();
}

void InstanceResultSortModel::setFilterType(FilterType type)
{
    if (m_filterType == type)
    {
        return;
    }
    m_filterType = type;
    emit filterTypeChanged();
}

void InstanceResultSortModel::setSortOnDate(bool sortOnDate)
{
    if (m_sortOnDate == sortOnDate)
    {
        return;
    }
    m_sortOnDate = sortOnDate;
    emit sortOnDateChanged();
}

void InstanceResultSortModel::setIncludeLinkedSeams(bool includeLinkedSeams)
{
    if (m_includeLinkedSeams == includeLinkedSeams)
    {
        return;
    }
    m_includeLinkedSeams = includeLinkedSeams;
    emit includeLinkedSeamsChanged();
}

}
}

