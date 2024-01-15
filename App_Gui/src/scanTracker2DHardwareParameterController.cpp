#include "scanTracker2DHardwareParameterController.h"
#include "abstractHardwareParameterModel.h"
#include "hardwareParameters.h"
#include "parameter.h"

namespace precitec
{
namespace gui
{

ScanTracker2DHardwareParameterController::ScanTracker2DHardwareParameterController(QObject* parent)
    : HardwareParameterController(parent)
{
    connect(this, &HardwareParameterController::modelChanged, this, &ScanTracker2DHardwareParameterController::initMode);
}

ScanTracker2DHardwareParameterController::~ScanTracker2DHardwareParameterController() = default;

void ScanTracker2DHardwareParameterController::setMode(Mode mode)
{
    if (m_mode == mode)
    {
        return;
    }
    m_mode = mode;
    emit modeChanged();
}

void ScanTracker2DHardwareParameterController::initMode()
{
    auto* parameterModel = this->model();
    if (!parameterModel)
    {
        return;
    }
    auto parameter = parameterModel->data(parameterModel->indexForKey(HardwareParameters::Key::ScanTracker2DCustomFigure), Qt::UserRole + 3).value<storage::Parameter*>();
    if (parameter)
    {
        if (parameter->value().toBool())
        {
            setMode(Mode::CustomFigure);
        }
        else
        {
            setMode(Mode::BasicFigure);
        }
        initFileId();
    }
    else
    {
        setMode(Mode::NotSet);
    }

    connect(this, &ScanTracker2DHardwareParameterController::modeChanged, this, &ScanTracker2DHardwareParameterController::updateModeParameter);
}

void ScanTracker2DHardwareParameterController::updateModeParameter()
{
    auto* parameterModel = this->model();
    if (!parameterModel)
    {
        return;
    }
    resetFileId();

    switch (m_mode)
    {
    case Mode::NotSet:
        parameterModel->setEnable(HardwareParameters::Key::ScanTracker2DCustomFigure, false);
        break;
    case Mode::BasicFigure:
        if (!parameterModel->data(parameterModel->indexForKey(HardwareParameters::Key::ScanTracker2DCustomFigure), Qt::UserRole + 1).toBool())
        {
            parameterModel->setEnable(HardwareParameters::Key::ScanTracker2DCustomFigure, true);
        }
        parameterModel->updateHardwareParameter(HardwareParameters::Key::ScanTracker2DCustomFigure, false);
        break;
    case Mode::CustomFigure:
        if (!parameterModel->data(parameterModel->indexForKey(HardwareParameters::Key::ScanTracker2DCustomFigure), Qt::UserRole + 1).toBool())
        {
            parameterModel->setEnable(HardwareParameters::Key::ScanTracker2DCustomFigure, true);
        }
        parameterModel->updateHardwareParameter(HardwareParameters::Key::ScanTracker2DCustomFigure, true);
        break;
    }
}

void ScanTracker2DHardwareParameterController::initFileId()
{
    auto* parameterModel = this->model();
    if (!parameterModel)
    {
        return;
    }
    auto parameter = parameterModel->data(parameterModel->indexForKey(HardwareParameters::Key::ScannerFileNumber), Qt::UserRole + 3).value<storage::Parameter*>();
    if (parameter)
    {
        setFileIdInternal(parameter->value().toInt());
    }
    else
    {
        setFileIdInternal(-1);
    }
}

void ScanTracker2DHardwareParameterController::resetFileId()
{
    if (auto* parameterModel = this->model())
    {
        parameterModel->setEnable(HardwareParameters::Key::ScannerFileNumber, false);
    }
    setFileIdInternal(-1);
}

void ScanTracker2DHardwareParameterController::setFileIdInternal(int fileId)
{
    if (m_fileId == fileId)
    {
        return;
    }
    m_fileId = fileId;
    emit fileIdChanged();
}

void ScanTracker2DHardwareParameterController::setFileId(int fileId)
{
    setFileIdInternal(fileId);
    if (auto* parameterModel = this->model())
    {
        if (m_fileId == -1)
        {
            parameterModel->setEnable(HardwareParameters::Key::ScannerFileNumber, false);
        }
        else
        {
            if (!parameterModel->data(parameterModel->indexForKey(HardwareParameters::Key::ScannerFileNumber), Qt::UserRole + 1).toBool())
            {
                parameterModel->setEnable(HardwareParameters::Key::ScannerFileNumber, true);
            }
            parameterModel->updateHardwareParameter(HardwareParameters::Key::ScannerFileNumber, m_fileId);
        }
    }
}

}
}
