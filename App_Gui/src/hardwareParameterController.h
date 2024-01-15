#pragma once

#include "liveModeController.h"

#include <QPointer>

namespace precitec
{

namespace storage
{
class Seam;
}

namespace gui
{
class AbstractHardwareParameterModel;
class HardwareParameterFilterModel;

/**
 * Controller to update live product when hardware parameters change.
 **/
class HardwareParameterController : public LiveModeController
{
    Q_OBJECT
    /**
     * The model providing the hardware Parameter.
     **/
    Q_PROPERTY(precitec::gui::AbstractHardwareParameterModel *model READ model WRITE setModel NOTIFY modelChanged)
    /**
     * Optional model to filter the hardware Parameters to be written to the default product
     **/
    Q_PROPERTY(precitec::gui::HardwareParameterFilterModel *filterModel READ filterModel WRITE setFilterModel NOTIFY filterModelChanged)

public:
    explicit HardwareParameterController(QObject *parent = nullptr);
    ~HardwareParameterController() override;

    AbstractHardwareParameterModel *model() const;
    HardwareParameterFilterModel *filterModel() const;

    void setModel(AbstractHardwareParameterModel *model);
    void setFilterModel(HardwareParameterFilterModel *filterModel);

Q_SIGNALS:
    void modelChanged();
    void filterModelChanged();

private:
    void updateHardwareParameters();
    void updateLiveProduct();
    precitec::storage::Seam *defaultSeam() const;
    QPointer<AbstractHardwareParameterModel> m_model;
    QPointer<HardwareParameterFilterModel> m_filterModel;
};

}
}

