#include "previewController.h"

#include "FileModel.h"
#include "laserPointController.h"

#include <QDebug>

using precitec::scantracker::components::wobbleFigureEditor::FileModel;

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

PreviewController::PreviewController(QObject* parent)
    : QObject(parent)
{
}

PreviewController::~PreviewController() = default;

void PreviewController::setFileModel(precitec::scantracker::components::wobbleFigureEditor::FileModel* model)
{
    if (m_fileModel == model)
    {
        return;
    }

    disconnect(m_fileModelDestroyedConnection);
    m_fileModel = model;

    if (m_fileModel)
    {
        m_fileModelDestroyedConnection = connect(m_fileModel, &QObject::destroyed, this, std::bind(&PreviewController::setFileModel, this, nullptr));
    }
    else
    {
        m_fileModelDestroyedConnection = {};
    }
    emit fileModelChanged();
}

void PreviewController::setLaserPointController(precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* newLaserPointController)
{
    if (m_laserPointController == newLaserPointController)
    {
        return;
    }

    disconnect(m_laserPointControllerDestroyedConnection);
    m_laserPointController = newLaserPointController;

    if (m_laserPointController)
    {
        m_laserPointControllerDestroyedConnection = connect(m_laserPointController, &QObject::destroyed, this, std::bind(&PreviewController::setLaserPointController, this, nullptr));
    }
    else
    {
        m_laserPointControllerDestroyedConnection = {};
    }
    emit laserPointControllerChanged();
}

template <typename T, typename U>
void PreviewController::preview(const QModelIndex &index, T load, U draw)
{
    if (!m_fileModel || !m_laserPointController)
    {
        return;
    }
    draw(m_laserPointController, load(m_fileModel, index.data(Qt::UserRole + 4).toString()));
    m_laserPointController->setPointsAreModifiable(false);
}

void PreviewController::previewSeamFigure(const QModelIndex &index)
{
    preview(index, std::mem_fn(&FileModel::loadSeamFigure), std::mem_fn(&LaserPointController::drawSeamFigure));
}

void PreviewController::previewWobbleFigure(const QModelIndex &index)
{
    preview(index, std::mem_fn(&FileModel::loadWobbleFigure), std::mem_fn(&LaserPointController::drawWobbleFigure));
}

void PreviewController::previewBasicFigure(const QModelIndex& index)
{
    preview(index, std::mem_fn(&FileModel::loadBasicFigure), std::mem_fn(&LaserPointController::drawBasicFigure));
}

}
}
}
}
