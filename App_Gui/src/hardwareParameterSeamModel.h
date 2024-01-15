#pragma once

#include "abstractHardwareParameterModel.h"

namespace precitec
{

namespace storage
{
class ParameterSet;
class Seam;
}

namespace gui
{

/**
 * The HardwareParameterModel provides a few dedicated hardware parameters and their
 * values in the hardware ParameterSet of a Seam.
 *
 * The model provides the following roles:
 * @li display: user visible name of the hardware parameter
 * @li key: the HardwareParameterModel::Key identifying the parameter
 * @li enabled: whether the hardware Parameter is available in the hardware ParameterSet
 * @li attribute: the Attribute for this hardware Parameter
 * @li parameter: the Parameter in the hardware ParameterSet, is @c null if enabled is @c false
 * @li milliFromMicro: the value is in um and should be presented in mm
 **/
class HardwareParameterSeamModel : public AbstractHardwareParameterModel
{
    Q_OBJECT
    /**
     * The Seam for which the HardwareParameter values should be taken.
     **/
    Q_PROPERTY(precitec::storage::Seam *seam READ seam WRITE setSeam NOTIFY seamChanged)
public:
    explicit HardwareParameterSeamModel(QObject *parent = nullptr);
    ~HardwareParameterSeamModel();

    precitec::storage::Seam *seam() const
    {
        return m_seam;
    }
    void setSeam(precitec::storage::Seam *seam);

    /**
     * Gets the ParameterSet for the Seam. If there is none it will be created.
     * Assumes there is a seam.
     **/
    precitec::storage::ParameterSet *getParameterSet() const override;

Q_SIGNALS:
    void seamChanged();

protected:
    bool isValid() const override
    {
        return m_seam != nullptr;
    }
    precitec::storage::ParameterSet *getParameterSetDirect() const override;

private:
    precitec::storage::Seam *m_seam = nullptr;
    QMetaObject::Connection m_seamDestroyed;
};

}
}
