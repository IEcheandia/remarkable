#pragma once

#include "abstractLaserControlTaskModel.h"

class LaserControlProductModelTest;

namespace precitec
{

namespace storage
{

class ParameterSet;
class Product;
class LaserControlPreset;

}

namespace gui
{

class LaserControlProductModel : public AbstractLaserControlTaskModel
{
    Q_OBJECT

    /**
     * The Seam for which the HardwareParameter values should be taken
     **/
    Q_PROPERTY(precitec::storage::Product* product READ product WRITE setProduct NOTIFY productChanged)

public:
    explicit LaserControlProductModel(QObject* parent = nullptr);
    ~LaserControlProductModel() override;

    precitec::storage::Product* product() const
    {
        return m_product;
    }
    void setProduct(precitec::storage::Product *product);

Q_SIGNALS:
    void productChanged();

protected:
    void updateHardwareParameters() override;
    precitec::storage::ParameterSet *getParameterSet() const override;
    void init() override;

private:
    precitec::storage::Product* m_product = nullptr;
    QMetaObject::Connection m_productDestroyed;

    friend LaserControlProductModelTest;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::LaserControlProductModel*)
