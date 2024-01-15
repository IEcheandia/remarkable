#include "commandManager.h"

using namespace precitec::scantracker::components::wobbleFigureEditor;

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

CommandManager::CommandManager(QObject *parent)
    : QObject(parent)
{
}

CommandManager::~CommandManager() = default;

void CommandManager::addAction(precitec::interface::ActionInterface_sp actionSp)
{
    if(isRedoPossible())
    {
        if(m_currentIndex < 0)
        {
            clearStack();
        }
        else
        {
            m_actionList = m_actionList.mid(0, m_currentIndex+1);
        }
    }
    m_actionList.append(std::move(actionSp));
    m_currentIndex = m_actionList.size() -1;
    emitPossibleChanges();
}

void CommandManager::clearStack()
{
    m_actionList.clear();
    m_currentIndex = -1;
    emit undoIsPossibleChanged();
    emit redoIsPossibleChanged();
}

void CommandManager::undo()
{
    if(!isUndoPossible())
    {
        return;
    }
    auto action =  m_actionList.at(m_currentIndex);
    action->undo();
    if(m_currentIndex > -1)
        m_currentIndex--;
    emit executedUndo();
    emitPossibleChanges();
}

void CommandManager::redo()
{
    if(!isRedoPossible())
    {
        return;
    }
    if(m_currentIndex < m_actionList.size()-1)
        m_currentIndex++;
    auto action =  m_actionList.at(m_currentIndex);
    action->redo();
    emit executedRedo();
    emitPossibleChanges();
}

bool CommandManager::isUndoPossible() const
{
    return m_currentIndex > -1;
}

bool CommandManager::isRedoPossible() const
{
    return (!m_actionList.empty() &&
            m_currentIndex < m_actionList.size()-1);
}

void CommandManager::emitPossibleChanges()
{
    emit undoIsPossibleChanged();
    emit redoIsPossibleChanged();
}


} // namepsace wobbleFigureEditor
} // namepsace components
} // namepsace scantracker
} // namepsace precitec
