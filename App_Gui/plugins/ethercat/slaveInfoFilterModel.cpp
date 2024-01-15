#include "slaveInfoFilterModel.h"

#include "event/ethercatDefines.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

SlaveInfoFilterModel::SlaveInfoFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

SlaveInfoFilterModel::~SlaveInfoFilterModel() = default;

bool SlaveInfoFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto index = sourceModel()->index(source_row, 0, source_parent);
    const auto vendorId = index.data(Qt::UserRole).value<quint32>();
    const auto productCode = index.data(Qt::UserRole + 1).value<quint32>();

    if (vendorId == VENDORID_BECKHOFF)
    {
        if (productCode == PRODUCTCODE_EL1018)
        {
            return true;
        }
        if (productCode == PRODUCTCODE_EL3102)
        {
            return true;
        }
        if (productCode == PRODUCTCODE_EL3702)
        {
            return true;
        }
        if (productCode == PRODUCTCODE_EL2008)
        {
            return true;
        }
        if (productCode == PRODUCTCODE_EL4102)
        {
            return true;
        }
        if (productCode == PRODUCTCODE_EL4132)
        {
            return true;
        }
    }
    if (vendorId == VENDORID_KUNBUS && productCode == PRODUCTCODE_KUNBUS_GW)
    {
        return true;
    }
    if (vendorId == VENDORID_HMS && productCode == PRODUCTCODE_ANYBUS_GW)
    {
        return true;
    }
    if (vendorId == VENDORID_HILSCHER && productCode == PRODUCTCODE_FIELDBUS)
    {
        return true;
    }

    return false;
}

}
}
}
}
