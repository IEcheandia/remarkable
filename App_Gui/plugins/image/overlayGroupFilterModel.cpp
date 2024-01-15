#include "overlayGroupFilterModel.h"


namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{

OverlayGroupFilterModel::OverlayGroupFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

OverlayGroupFilterModel::~OverlayGroupFilterModel() = default;

bool OverlayGroupFilterModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const
{
    return sourceColumn == 0 && !sourceParent.isValid();
}

bool OverlayGroupFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    return sourceModel()->data(index, Qt::UserRole).toBool();
}

void OverlayGroupFilterModel::swapEnabled(int row)
{
    const QModelIndex sourceIndex = mapToSource(index(row, 0));
    if (!sourceIndex.isValid())
    {
        return;
    }
    sourceModel()->setData(sourceIndex, !sourceModel()->data(sourceIndex, Qt::UserRole + 1).toBool(), Qt::UserRole + 1);
}

}
}
}
}
