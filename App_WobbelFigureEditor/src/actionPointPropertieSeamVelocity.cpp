#include "actionPointPropertieSeamVelocity.h"
namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

ActionPointPropertieSeamVelocity::ActionPointPropertieSeamVelocity(
        WobbleFigureEditor& figureEditor,
        RTC6::seamFigure::SeamFigure &figure,
        int laserPointID,
        double oldVelocity,
        double newVelocity)
    : m_figureEditor{figureEditor}
    , m_figure{figure}
    , m_laserPointID{laserPointID}
    , m_oldVelocity{oldVelocity}
    , m_newVelocity{newVelocity}
{
}

void ActionPointPropertieSeamVelocity::undo()
{
    updateVelocity(m_oldVelocity);
}

void ActionPointPropertieSeamVelocity::redo()
{
    updateVelocity(m_newVelocity);
}

void ActionPointPropertieSeamVelocity::updateVelocity(double velocity)
{
    m_figure.figure.at(m_laserPointID).velocity = velocity;
    WobbleFigure* figure = m_figureEditor.figure();
    auto laserPoint = figure->searchPoint(m_laserPointID);
    laserPoint->setVelocity(velocity);
    emit m_figureEditor.nodePropertiesUpdated();
}

}
}
}
}
