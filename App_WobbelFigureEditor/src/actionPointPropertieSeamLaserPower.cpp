#include "actionPointPropertieSeamLaserPower.h"
namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

ActionPointPropertieSeamLaserPower::ActionPointPropertieSeamLaserPower(
        WobbleFigureEditor& figureEditor,
        RTC6::seamFigure::SeamFigure &figure,
        int laserPointID,
        double oldLaserPower,
        double newLaserPower)
    : m_figureEditor{figureEditor}
    , m_figure{figure}
    , m_laserPointID{laserPointID}
    , m_oldLaserPower{oldLaserPower}
    , m_newLaserPower{newLaserPower}
{
}

void ActionPointPropertieSeamLaserPower::undo()
{
    updateLaserPower(m_oldLaserPower);
}

void ActionPointPropertieSeamLaserPower::redo()
{
    updateLaserPower(m_newLaserPower);
}

void ActionPointPropertieSeamLaserPower::updateLaserPower(double laserPower)
{
    m_figure.figure.at(m_laserPointID).power = laserPower;
    WobbleFigure* figure = m_figureEditor.figure();
    auto laserPoint = figure->searchPoint(m_laserPointID);
    if(laserPower < 0)
        laserPoint->setLaserPower(laserPower);
    else
        laserPoint->setLaserPower(laserPower*100);
    emit m_figureEditor.nodePropertiesUpdated();
}

}
}
}
}
