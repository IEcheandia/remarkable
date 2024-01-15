#include "actionPointPropertieWobbleRingPower.h"
namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

ActionPointPropertieWobbleRingPower::ActionPointPropertieWobbleRingPower(
        WobbleFigureEditor& figureEditor,
        RTC6::wobbleFigure::Figure& figure,
        int laserPointID,
        double oldRingPower,
        double newRingPower)
    : m_figureEditor{figureEditor}
    , m_figure{figure}
    , m_laserPointID{laserPointID}
    , m_oldRingPower{oldRingPower}
    , m_newRingPower{newRingPower}
{
}

void ActionPointPropertieWobbleRingPower::undo()
{
    updateRingPower(m_oldRingPower);
}

void ActionPointPropertieWobbleRingPower::redo()
{
    updateRingPower(m_newRingPower);
}

void ActionPointPropertieWobbleRingPower::updateRingPower(double ringPower)
{
    m_figure.figure.at(m_laserPointID).ringPower = ringPower;
    WobbleFigure* figure = m_figureEditor.figure();
    auto laserPoint = figure->searchPoint(m_laserPointID);
    if(ringPower < 0)
        laserPoint->setRingPower(ringPower);
    else
        laserPoint->setRingPower(ringPower*100);
    emit m_figureEditor.nodePropertiesUpdated();
}

}
}
}
}
