#pragma once

#include "abstractHardwareParameterModel.h"

namespace precitec
{

namespace storage
{
class ParameterSet;
class SeamSeries;
}

namespace gui
{

/**
 * The HardwareParameterSeriesModel provides a few dedicated hardware parameters and their
 * values in the hardware ParameterSet of a Product.
 *
 * The model provides the following roles:
 * @li display: user visible name of the hardware parameter
 * @li key: the HardwareParameterModel::Key identifying the parameter
 * @li enabled: whether the hardware Parameter is available in the hardware ParameterSet
 * @li attribute: the Attribute for this hardware Parameter
 * @li parameter: the Parameter in the hardware ParameterSet, is @c null if enabled is @c false
 **/
class HardwareParameterSeriesModel : public AbstractHardwareParameterModel
{
    Q_OBJECT
    /**
     * The Product for which the HardwareParameter values should be taken.
     **/
    Q_PROPERTY(precitec::storage::SeamSeries* seamSeries READ seamSeries WRITE setSeamSeries NOTIFY seamSeriesChanged)
public:
    explicit HardwareParameterSeriesModel(QObject *parent = nullptr);
    ~HardwareParameterSeriesModel();

    precitec::storage::SeamSeries* seamSeries() const
    {
        return m_seamSeries;
    }
    void setSeamSeries(precitec::storage::SeamSeries* seamSeries);

    /**
     * Gets the ParameterSet for the Product. If there is none it will be created.
     * Assumes there is a product.
     **/
    precitec::storage::ParameterSet *getParameterSet() const override;

Q_SIGNALS:
    void seamSeriesChanged();

protected:
    bool isValid() const override
    {
        return m_seamSeries != nullptr;
    }
    precitec::storage::ParameterSet *getParameterSetDirect() const override;

private:
    precitec::storage::SeamSeries *m_seamSeries = nullptr;
    QMetaObject::Connection m_seamSeriesDestroyed;
};

}
}


