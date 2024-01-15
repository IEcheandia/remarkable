#include "errorGroupFilterModel.h"
#include "errorGroupModel.h"
#include "simpleError.h"

using precitec::storage::SimpleError;

namespace precitec
{
namespace gui
{

ErrorGroupFilterModel::ErrorGroupFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &ErrorGroupFilterModel::filterGroupChanged, this, &ErrorGroupFilterModel::invalidateFilter);
    connect(this, &ErrorGroupFilterModel::sourceModelChanged, this, &ErrorGroupFilterModel::invalidateFilter);
    connect(this, &ErrorGroupFilterModel::modelReset, this, &ErrorGroupFilterModel::invalidateFilter);
}

ErrorGroupFilterModel::~ErrorGroupFilterModel() = default;

void ErrorGroupFilterModel::setFilterGroup(int group)
{
    if (m_filterGroup == group)
    {
        return;
    }
    m_filterGroup = group;
    emit filterGroupChanged();
}

void ErrorGroupFilterModel::setInterval(bool set)
{
    if (m_interval == set)
    {
        return;
    }
    m_interval = set;
    emit intervalChanged();
}

bool ErrorGroupFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (m_filterGroup == -1)
    {
        return false;
    }
    const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);

    if (m_interval && sourceIndex.data(Qt::UserRole + 5).value<SimpleError::BoundaryType>() == SimpleError::BoundaryType::Reference)
    {
        return false;
    }

    const auto group = sourceIndex.data(Qt::UserRole + 4).value<ErrorGroupModel::ErrorGroup>();
    return group == ErrorGroupModel::ErrorGroup(m_filterGroup);
}

}
}



