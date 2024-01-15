#pragma once

#include "abstractHardwareParameterModel.h"

namespace precitec
{

namespace storage
{
class ParameterSet;
class Product;
}

namespace gui
{

/**
 * The HardwareParameterProductModel provides a few dedicated hardware parameters and their
 * values in the hardware ParameterSet of a Product.
 *
 * The model provides the following roles:
 * @li display: user visible name of the hardware parameter
 * @li key: the HardwareParameterModel::Key identifying the parameter
 * @li enabled: whether the hardware Parameter is available in the hardware ParameterSet
 * @li attribute: the Attribute for this hardware Parameter
 * @li parameter: the Parameter in the hardware ParameterSet, is @c null if enabled is @c false
 **/
class HardwareParameterProductModel : public AbstractHardwareParameterModel
{
    Q_OBJECT
    /**
     * The Product for which the HardwareParameter values should be taken.
     **/
    Q_PROPERTY(precitec::storage::Product *product READ product WRITE setProduct NOTIFY productChanged)
public:
    explicit HardwareParameterProductModel(QObject *parent = nullptr);
    ~HardwareParameterProductModel();

    precitec::storage::Product *product() const
    {
        return m_product;
    }
    void setProduct(precitec::storage::Product *product);

    /**
     * Gets the ParameterSet for the Product. If there is none it will be created.
     * Assumes there is a product.
     **/
    precitec::storage::ParameterSet *getParameterSet() const override;

Q_SIGNALS:
    void productChanged();

protected:
    bool isValid() const override
    {
        return m_product != nullptr;
    }
    precitec::storage::ParameterSet *getParameterSetDirect() const override;

private:
    precitec::storage::Product *m_product = nullptr;
    QMetaObject::Connection m_productDestroyed;
};

}
}

