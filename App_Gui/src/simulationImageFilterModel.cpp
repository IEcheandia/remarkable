#include "simulationImageFilterModel.h"
#include "simulationController.h"
#include "simulationImageModel.h"

#include "seam.h"
#include "seamSeries.h"

namespace precitec
{
namespace gui
{

SimulationImageFilterModel::SimulationImageFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &SimulationImageFilterModel::seamChanged, this, &SimulationImageFilterModel::updateFilter);
}

SimulationImageFilterModel::~SimulationImageFilterModel() = default;

bool SimulationImageFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const auto index = sourceModel()->index(source_row, 0, source_parent);
    if (index.data(Qt::UserRole).value<quint32>() != m_seamSeries)
    {
        return false;
    }
    if (index.data(Qt::UserRole + 1).value<quint32>() != m_seam)
    {
        return false;
    }
    return true;
}

void SimulationImageFilterModel::setSimulationController(SimulationController* controller)
{
    if (m_controller == controller)
    {
        return;
    }
    m_controller = controller;
    setSourceModel(m_controller ? m_controller->imageModel() : nullptr);
    if (m_controller)
    {
        connect(m_controller, &SimulationController::imageChanged, this, &SimulationImageFilterModel::updateImage);
    }
    emit simulationControllerChanged();
}

void SimulationImageFilterModel::updateImage()
{
    if (!m_controller)
    {
        return;
    }
    bool changed = false;
    const auto index = m_controller->imageModel()->index(m_controller->frameIndex(), 0);
    if (!index.isValid())
    {
        setCurrentFrameIndex(-1);
        return;
    }
    setCurrentFrameIndex(mapFromSource(index).row());
    const auto seamSeries = index.data(Qt::UserRole).value<quint32>();
    const auto seam = index.data(Qt::UserRole + 1).value<quint32>();
    if (seamSeries != m_seamSeries)
    {
        changed = true;
        m_seamSeries = seamSeries;
    }
    if (seam != m_seam)
    {
        changed = true;
        m_seam = seam;
    }
    if (changed)
    {
        emit seamChanged();
    }
}

void SimulationImageFilterModel::updateFilter()
{
    invalidateFilter();

    auto newIndex = -1;

    if (m_controller)
    {
        newIndex = mapFromSource(m_controller->imageModel()->index(m_controller->frameIndex(), 0)).row();
    }

    setCurrentFrameIndex(newIndex);
}

void SimulationImageFilterModel::setCurrentFrameIndex(int index)
{
    if (m_currentFrameIndex == index)
    {
        return;
    }
    m_currentFrameIndex = index;
    emit currentFrameIndexChanged();
}

}
}
