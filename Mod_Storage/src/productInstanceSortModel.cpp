#include "productInstanceSortModel.h"
#include "productInstanceModel.h"

namespace precitec
{
namespace storage
{

ProductInstanceSortModel::ProductInstanceSortModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_from(QDate::currentDate())
    , m_to(QDate::currentDate())
{
    connect(this, &ProductInstanceSortModel::filterTypeChanged, this, &ProductInstanceSortModel::invalidate);
    connect(this, &ProductInstanceSortModel::invalidateFilter, this, &ProductInstanceSortModel::invalidate);
    connect(this, &ProductInstanceSortModel::filterOnDateChanged, this, &ProductInstanceSortModel::invalidate);
    connect(this, &ProductInstanceSortModel::fromChanged, this, &ProductInstanceSortModel::invalidate);
    connect(this, &ProductInstanceSortModel::toChanged, this, &ProductInstanceSortModel::invalidate);

    connect(this, &ProductInstanceSortModel::rowsInserted, this, &ProductInstanceSortModel::rowCountChanged);
    connect(this, &ProductInstanceSortModel::rowsRemoved, this, &ProductInstanceSortModel::rowCountChanged);
}

ProductInstanceSortModel::~ProductInstanceSortModel() = default;

bool ProductInstanceSortModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto state = sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 2).value<ProductInstanceModel::State>();

    switch (m_filterType)
    {
        case FilterType::RemoveNIO:
            if (state == ProductInstanceModel::State::Nio)
            {
                return false;
            }
            break;
        case FilterType::OnlyNIO:
            if (state != ProductInstanceModel::State::Nio)
            {
                return false;
            }
            break;
        case FilterType::All:
            break;
    }

    if (!m_filter.isEmpty())
    {
        // Serial Number (DisplayRole) and Part Number (User Role + 11)
        const auto index = sourceModel()->index(source_row, 0, source_parent);
        if (!index.data(Qt::DisplayRole).toString().contains(m_filter, Qt::CaseInsensitive) &&
            !index.data(Qt::UserRole + 11).toString().contains(m_filter, Qt::CaseInsensitive))
        {
            return false;
        }
    }
    if (m_filterOnDate)
    {
        const auto reference = sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 1).toDateTime().date();
        if (reference < m_from)
        {
            return false;
        }
        if (reference > m_to)
        {
            return false;
        }
    }
    return true;
}

void ProductInstanceSortModel::setSortOrder(Qt::SortOrder order)
{
    sort(0, order);
    emit sortOrderChanged();
}

void ProductInstanceSortModel::setFilterType(FilterType type)
{
    if(m_filterType == type)
    {
        return;
    }
    m_filterType = type;
    emit filterTypeChanged();
}

void ProductInstanceSortModel::setFilterOnDate(bool set)
{
    if (m_filterOnDate == set)
    {
        return;
    }
    m_filterOnDate = set;
    emit filterOnDateChanged();
}

void ProductInstanceSortModel::setFrom(const QDate& date)
{
    if (m_from == date)
    {
        return;
    }
    m_from = date;
    emit fromChanged();
}

void ProductInstanceSortModel::setTo(const QDate& date)
{
    if (m_to == date)
    {
        return;
    }
    m_to = date;
    emit toChanged();
}

void ProductInstanceSortModel::forceSort()
{
    sort(0, sortOrder());
}

}
}
