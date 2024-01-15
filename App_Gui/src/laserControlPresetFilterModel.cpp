#include "laserControlPresetFilterModel.h"

namespace precitec
{
namespace gui
{

LaserControlPresetFilterModel::LaserControlPresetFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

LaserControlPresetFilterModel::~LaserControlPresetFilterModel() = default;

bool LaserControlPresetFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    // AbstractLaserControlModel isCurrent role
    return sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 4).toBool();
}

}
}



