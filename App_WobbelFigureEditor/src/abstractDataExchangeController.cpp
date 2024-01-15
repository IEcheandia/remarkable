#include "abstractDataExchangeController.h"

#include "productModel.h"
#include "product.h"
#include "seam.h"

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

AbstractDataExchangeController::AbstractDataExchangeController(QObject* parent)
    : QObject(parent)
{
    connect(this, &AbstractDataExchangeController::productModelIndexChanged, this, &AbstractDataExchangeController::setProductFromProductModel);
    connect(this, &AbstractDataExchangeController::productChanged, this, &AbstractDataExchangeController::setSeamListFromProduct);
    connect(this, &AbstractDataExchangeController::seamListIndexChanged, this, &AbstractDataExchangeController::setSeamFromList);
}

AbstractDataExchangeController::~AbstractDataExchangeController() = default;

void AbstractDataExchangeController::setProductModel(precitec::storage::ProductModel* productModel)
{
    if (m_productModel == productModel)
    {
        return;
    }

    disconnect(m_productModelDestroyedConnection);
    m_productModel = productModel;

    if (m_productModel)
    {
        m_productModelDestroyedConnection = connect(m_productModel, &QObject::destroyed, this, std::bind(&AbstractDataExchangeController::setProductModel, this, nullptr));
    }
    else
    {
        m_productModelDestroyedConnection = {};
    }

    emit productModelChanged();
}

void AbstractDataExchangeController::setProductModelIndex(const QModelIndex& modelIndex)
{
    if (m_productModelIndex == modelIndex)
    {
        return;
    }

    m_productModelIndex = modelIndex;
    emit productModelIndexChanged();
}

void AbstractDataExchangeController::setProduct(precitec::storage::Product* newProduct)
{
    if (m_product == newProduct)
    {
        return;
    }

    disconnect(m_productDestroyedConnection);
    m_product = newProduct;

    if (m_product)
    {
        m_productDestroyedConnection = connect(m_product, &QObject::destroyed, this, std::bind(&AbstractDataExchangeController::setProductModel, this, nullptr));
    }
    else
    {
        m_productDestroyedConnection = {};
    }

    emit productChanged();
}

void AbstractDataExchangeController::setSeams(const QVariantList& seams)
{
    if (m_seams == seams)
    {
        return;
    }

    m_seams = seams;
    emit seamsChanged();
}

void AbstractDataExchangeController::setSeamListIndex(int newIndex)
{
    if (m_seamListIndex == newIndex)
    {
        return;
    }

    m_seamListIndex = newIndex;
    emit seamListIndexChanged();
}

void AbstractDataExchangeController::setSeam(precitec::storage::Seam* newSeam)
{
    if (m_seam == newSeam)
    {
        return;
    }

    disconnect(m_seamDestroyedConnection);
    m_seam = newSeam;

    if (m_seam)
    {
        m_seamDestroyedConnection = connect(m_seam, &QObject::destroyed, this, std::bind(&AbstractDataExchangeController::setSeam, this, nullptr));
    }
    else
    {
        m_seamDestroyedConnection = {};
    }
    emit seamChanged();
}

void AbstractDataExchangeController::setProductFromProductModel()
{
    if (!m_productModel || !m_productModelIndex.isValid())
    {
        return;
    }

    const auto& productVariant = m_productModel->data(m_productModelIndex, Qt::UserRole + 1);

    if (!productVariant.isValid() || productVariant.isNull() || !productVariant.canConvert<precitec::storage::Product*>())
    {
        return;
    }

    setProduct(productVariant.value<precitec::storage::Product*>());
}

void AbstractDataExchangeController::setSeamListFromProduct()
{
    if (!m_product)
    {
        return;
    }

    setSeams(m_product->allSeams());
}

void AbstractDataExchangeController::setSeamFromList()
{
    if (m_seams.isEmpty() || m_seamListIndex < 0 || m_seamListIndex >= m_seams.size())
    {
        return;
    }

    const auto& variant = m_seams.at(seamListIndex());

    if (!variant.isValid() || variant.isNull() || !variant.canConvert<precitec::storage::Seam*>())
    {
        return;
    }

    setSeam(variant.value<precitec::storage::Seam*>());
}

}
}
}
}
