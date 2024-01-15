#include "lwmResultFilterModel.h"
#include "resultSettingModel.h"

using precitec::storage::ResultSettingModel;

namespace precitec
{
namespace gui
{

LwmResultFilterModel::LwmResultFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &LwmResultFilterModel::sourceModelChanged, this, &LwmResultFilterModel::invalidateFilter);
    connect(this, &LwmResultFilterModel::modelReset, this, &LwmResultFilterModel::invalidateFilter);

    sort(0);
}

LwmResultFilterModel::~LwmResultFilterModel() = default;

bool LwmResultFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (auto resultSettingModel = qobject_cast<ResultSettingModel*>(sourceModel()))
    {
        const auto index = resultSettingModel->index(source_row, 0, source_parent);
        const auto enumType = index.data(Qt::DisplayRole).toInt();

        return resultSettingModel->isLwmType(enumType);
    }

    return false;
}

bool LwmResultFilterModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    const auto leftData = sourceModel()->data(source_left, Qt::DisplayRole).toInt();
    const auto rightData = sourceModel()->data(source_right, Qt::DisplayRole).toInt();

    return leftData < rightData;
}

}
}



