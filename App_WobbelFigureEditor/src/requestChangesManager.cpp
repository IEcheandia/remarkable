#include "requestChangesManager.h"
#include <QDebug>

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

RequestChangesManager::RequestChangesManager(QObject* parent)
    : QObject(parent)
{
}

void RequestChangesManager::createNewFigure()
{
    m_actionType = ActionType::NewFigure;
    emit haveToAskAboutSavingFigure();
}

void RequestChangesManager::openNewFigure()
{
    m_actionType = ActionType::OpenFigure;
    emit haveToAskAboutSavingFigure();
}

void RequestChangesManager::importNewFigure()
{
    m_actionType = ActionType::ImportFigure;
    emit haveToAskAboutSavingFigure();
}

void RequestChangesManager::startSimulation()
{
    m_actionType = ActionType::SimulationStart;
    emit haveToAskAboutSavingFigure();
}

void RequestChangesManager::startAction()
{
    switch (m_actionType)
    {
    case ActionType::NewFigure:
        emit createNewFigureAllowed();
        break;
    case ActionType::OpenFigure:
        emit openNewFigureAllowed();
        break;
    case ActionType::ImportFigure:
        emit importNewFigureAllowed();
        break;
    case ActionType::SimulationStart:
        emit simulationStartAllowed();
        break;
    case ActionType::None:
        break;
    }
    resetAction();
}

void RequestChangesManager::resetAction()
{
    m_actionType = ActionType::None;
    emit resetedAction();
}

bool RequestChangesManager::requestIsInAction()
{
    return (m_actionType != ActionType::None);
}

RequestChangesManager::~RequestChangesManager() = default;

} // namepsace wobbleFigureEditor
} // namepsace components
} // namepsace scantracker
} // namepsace precitec
