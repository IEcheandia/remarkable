#include "hardwareParameterProductModel.h"
#include "attribute.h"
#include "attributeModel.h"
#include "product.h"
#include "parameter.h"
#include "parameterSet.h"

#include <QMetaEnum>

using precitec::storage::AttributeModel;
using precitec::storage::Product;
using precitec::storage::ParameterSet;

namespace precitec
{
namespace gui
{

HardwareParameterProductModel::HardwareParameterProductModel(QObject *parent)
    : AbstractHardwareParameterModel(parent)
{
    connect(this, &HardwareParameterProductModel::productChanged, this,
        [this]
        {
            dataChanged(index(0, 0), index(rowCount() - 1, 0), {});
        }
    );
}

HardwareParameterProductModel::~HardwareParameterProductModel() = default;

void HardwareParameterProductModel::setProduct(Product *product)
{
    if (m_product == product)
    {
        return;
    }
    m_product = product;
    disconnect(m_productDestroyed);
    if (m_product)
    {
        m_productDestroyed = connect(m_product, &Product::destroyed, this, std::bind(&HardwareParameterProductModel::setProduct, this, nullptr));
    } else
    {
        m_productDestroyed = QMetaObject::Connection{};
    }
    emit productChanged();
}

ParameterSet *HardwareParameterProductModel::getParameterSet() const
{
    if (!m_product)
    {
        return nullptr;
    }

    if (!m_product->hardwareParameters())
    {
        m_product->createHardwareParameters();
    }
    return m_product->hardwareParameters();
}

ParameterSet *HardwareParameterProductModel::getParameterSetDirect() const
{
    if (!m_product)
    {
        return nullptr;
    }
    return m_product->hardwareParameters();
}

}
}

