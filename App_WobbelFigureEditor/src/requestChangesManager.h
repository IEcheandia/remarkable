#pragma once
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

class RequestChangesManager : public QObject
{
    Q_OBJECT
public:
    explicit RequestChangesManager(QObject* parent = nullptr);
    ~RequestChangesManager() override;

    enum ActionType
    {
        None = 0,
        NewFigure,
        OpenFigure,
        ImportFigure,
        SimulationStart
    };

    Q_INVOKABLE void createNewFigure();
    Q_INVOKABLE void openNewFigure();
    Q_INVOKABLE void importNewFigure();
    Q_INVOKABLE void startSimulation();
    Q_INVOKABLE void startAction();
    Q_INVOKABLE void resetAction();
    Q_INVOKABLE bool requestIsInAction();

signals:
    void haveToAskAboutSavingFigure();
    void createNewFigureAllowed();
    void openNewFigureAllowed();
    void importNewFigureAllowed();
    void simulationStartAllowed();
    void resetedAction();

private:
    ActionType m_actionType{ActionType::None};
};

} // namepsace wobbleFigureEditor
} // namepsace components
} // namepsace scantracker
} // namepsace precitec
