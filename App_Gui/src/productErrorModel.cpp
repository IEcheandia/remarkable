#include "productErrorModel.h"
#include "product.h"
#include "productError.h"

using precitec::storage::Product;
using precitec::storage::ProductError;

namespace precitec
{
namespace gui
{

ProductErrorModel::ProductErrorModel(QObject *parent)
    : OverlyingErrorModel(parent)
{
}

ProductErrorModel::~ProductErrorModel() = default;

QVariant ProductErrorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !currentProduct())
    {
        return {};
    }
    const auto &errors = currentProduct()->overlyingErrors();
    if (index.row() > int(errors.size()))
    {
        return {};
    }
    auto error = errors.at(index.row());
    if (role == Qt::DisplayRole)
    {
         return error->name();
    }
    if (role == Qt::UserRole)
    {
         return QVariant::fromValue(error);
    }
    if (role == Qt::UserRole + 1)
    {
         return nameFromId(error->variantId());
    }
    if (role == Qt::UserRole + 2)
    {
         return isTypeError(error->variantId());
    }

    return {};
}

int ProductErrorModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !currentProduct())
    {
        return 0;
    }
    return currentProduct()->overlyingErrors().size();
}

QHash<int, QByteArray> ProductErrorModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("error")},
        {Qt::UserRole + 1, QByteArrayLiteral("type")},
        {Qt::UserRole + 2, QByteArrayLiteral("isTypeError")}
    };
}

void ProductErrorModel::setCurrentProduct(Product *product)
{
    if (m_currentProduct == product)
    {
        return;
    }
    beginResetModel();
    m_currentProduct = product;
    disconnect(m_destroyConnection);
    m_destroyConnection = QMetaObject::Connection{};
    if (m_currentProduct)
    {
        m_destroyConnection = connect(m_currentProduct, &QObject::destroyed, this, std::bind(&ProductErrorModel::setCurrentProduct, this, nullptr));
    } else
    {
        m_destroyConnection = {};
    }
    endResetModel();
    emit currentProductChanged();
}

ProductError *ProductErrorModel::createError(ErrorType errorType)
{
    if (!m_currentProduct)
    {
        return nullptr;
    }
    beginInsertRows({}, rowCount(), rowCount());

    auto error = m_currentProduct->addOverlyingError(variantId(errorType));
    error->setName(name(errorType));

    if (attributeModel())
    {
        error->initFromAttributes(attributeModel());
    }
    endInsertRows();

    return error;
}

void ProductErrorModel::removeError(ProductError *error)
{
    if (!error || !m_currentProduct)
    {
        return;
    }
    const auto &errors = m_currentProduct->overlyingErrors();
    auto it = std::find(errors.begin(), errors.end(), error);
    if (it == errors.end())
    {
        return;
    }
    auto index = std::distance(errors.begin(), it);
    beginRemoveRows({}, index, index);
    m_currentProduct->removeOverlyingError(index);
    endRemoveRows();
}

}
}


