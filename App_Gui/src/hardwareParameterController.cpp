#include "hardwareParameterController.h"
#include "abstractHardwareParameterModel.h"
#include "hardwareParameterFilterModel.h"

#include "copyMode.h"
#include "parameterSet.h"
#include "product.h"
#include "productModel.h"
#include "seamSeries.h"
#include "seam.h"

namespace precitec
{
using storage::CopyMode;

namespace gui
{

HardwareParameterController::HardwareParameterController(QObject *parent)
    : LiveModeController(parent)
{
    connect(this, &HardwareParameterController::liveModeChanged, this, &HardwareParameterController::updateLiveProduct);
}

HardwareParameterController::~HardwareParameterController() = default;

void HardwareParameterController::setModel(AbstractHardwareParameterModel *model)
{
    if (m_model == model)
    {
        return;
    }
    if (m_model)
    {
        disconnect(m_model, &AbstractHardwareParameterModel::parameterChanged, this, &HardwareParameterController::updateHardwareParameters);
    }
    m_model = model;
    if (m_model)
    {
        connect(m_model, &AbstractHardwareParameterModel::parameterChanged, this, &HardwareParameterController::updateHardwareParameters);
    }
    emit modelChanged();
}

void HardwareParameterController::setFilterModel(HardwareParameterFilterModel *filterModel)
{
    if (m_filterModel == filterModel)
    {
        return;
    }
    m_filterModel = filterModel;
    emit filterModelChanged();
}

AbstractHardwareParameterModel *HardwareParameterController::model() const
{
    return m_model.data();
}

HardwareParameterFilterModel *HardwareParameterController::filterModel() const
{
    return m_filterModel.data();
}

void HardwareParameterController::updateHardwareParameters()
{
    if (!m_model)
    {
        return;
    }
    updateLiveProduct();
}

void HardwareParameterController::updateLiveProduct()
{
    if (!productModel())
    {
        return;
    }
    auto seam = defaultSeam();
    if (!seam || !m_model)
    {
        return;
    }

    const auto parameterSet = m_model->getParameterSet();
    if (liveMode() && parameterSet)
    {
        const auto copyMode = CopyMode::WithDifferentIds;
        auto ps = parameterSet->duplicate(copyMode, seam);
        if (m_filterModel)
        {
            m_filterModel->filterPrameterSet(ps);
            m_model->updateLedSendData(ps);
            m_model->updateGenerateScanTracker2DFigure(ps);
        }
        seam->setHardwareParameters(ps);
    } else
    {
        seam->setHardwareParameters(nullptr);
    }

    if (liveMode() && !isUpdating())
    {
        stopLiveMode();
    }

    productModel()->defaultProduct()->save();

    if (!liveMode())
    {
        return;
    }
    startDelayedLiveMode();
}

precitec::storage::Seam *HardwareParameterController::defaultSeam() const
{
    auto defaultProduct = productModel()->defaultProduct();
    if (!defaultProduct)
    {
        return nullptr;
    }
    if (defaultProduct->seamSeries().empty())
    {
        return nullptr;
    }
    auto seamSeries = defaultProduct->seamSeries().front();
    if (seamSeries->seams().empty())
    {
        return nullptr;
    }
    return seamSeries->seams().front();
}

}
}
