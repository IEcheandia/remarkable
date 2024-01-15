#pragma once
#include "../../Interfaces/include/event/actioninterface.h"
#include <QObject>
#include <QList>

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

class CommandManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool undoIsPossible READ isUndoPossible NOTIFY undoIsPossibleChanged)
    Q_PROPERTY(bool redoIsPossible READ isRedoPossible NOTIFY redoIsPossibleChanged)
public:
    explicit CommandManager(QObject* parent = nullptr);
    ~CommandManager() override;

    void addAction(precitec::interface::ActionInterface_sp actionSp);
    Q_INVOKABLE void clearStack();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();

    bool isUndoPossible() const;
    bool isRedoPossible() const;

signals:
    void executedUndo();
    void executedRedo();
    void undoIsPossibleChanged();
    void redoIsPossibleChanged();
    void stackCleared();

private:
    void emitPossibleChanges();

private:
    QList<precitec::interface::ActionInterface_sp> m_actionList;
    int m_currentIndex{-1};
};


} // namepsace wobbleFigureEditor
} // namepsace components
} // namepsace scantracker
} // namepsace precitec
