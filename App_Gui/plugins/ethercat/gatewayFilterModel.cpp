#include "gatewayFilterModel.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

GatewayFilterModel::GatewayFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &GatewayFilterModel::signalTypeChanged, this, &GatewayFilterModel::invalidateFilter);
}

GatewayFilterModel::~GatewayFilterModel() = default;

bool GatewayFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    return sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 2).value<ViConfigService::SignalType>() == m_signalType;
}

void GatewayFilterModel::setSignalType(ViConfigService::SignalType type)
{
    if (m_signalType == type)
    {
        return;
    }
    m_signalType = type;
    emit signalTypeChanged();
}

}
}
}
}
