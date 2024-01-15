#include "seamsOnAssemblyImageFilterModel.h"

#include <QPointF>

namespace precitec
{
namespace gui
{

SeamsOnAssemblyImageFilterModel::SeamsOnAssemblyImageFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

SeamsOnAssemblyImageFilterModel::~SeamsOnAssemblyImageFilterModel() = default;

bool SeamsOnAssemblyImageFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    return sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole).toPointF() != QPointF{-1, -1};
}

}
}
