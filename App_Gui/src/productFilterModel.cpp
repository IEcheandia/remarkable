#include "productFilterModel.h"
#include "product.h"

using precitec::storage::Product;

namespace precitec
{
namespace gui
{

ProductFilterModel::ProductFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &ProductFilterModel::includeDefaultProductChanged, this, &ProductFilterModel::invalidate);
    sort(0);
}

ProductFilterModel::~ProductFilterModel() = default;

bool ProductFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (auto p = sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 1).value<Product*>())
    {
        return !p->isDefaultProduct() || m_includeDefaultProduct;
    }
    return false;
}

bool ProductFilterModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    auto leftProduct = source_left.data(Qt::UserRole+1).value<Product*>();
    auto rightProduct = source_right.data(Qt::UserRole+1).value<Product*>();
    if (!leftProduct || !rightProduct)
    {
        return true;
    }
    return leftProduct->type() < rightProduct->type();
}

void ProductFilterModel::setIncludeDefaultProduct(bool set)
{
    if (m_includeDefaultProduct == set)
    {
        return;
    }
    m_includeDefaultProduct = set;
    emit includeDefaultProductChanged();
}

}
}
