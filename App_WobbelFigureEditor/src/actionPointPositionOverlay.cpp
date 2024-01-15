#include "actionPointPositionOverlay.h"
#include <QPointF>

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

ActionPointPositionOverlay::ActionPointPositionOverlay(
        WobbleFigureEditor& figureEditor,
        RTC6::function::OverlayFunction &figure,
        std::pair<double, double> oldPosition,
        std::pair<double, double> newPosition,
        int laserPointID,
        int figureScale)
    : m_figureEditor{figureEditor}
    , m_figure{figure}
    , m_oldPosition{oldPosition}
    , m_newPosition{newPosition}
    , m_laserPointID{laserPointID}
    , m_figureScale{figureScale}
{
}

void ActionPointPositionOverlay::undo()
{
    updatePosition(m_oldPosition);
}

void ActionPointPositionOverlay::redo()
{
    updatePosition(m_newPosition);
}

void ActionPointPositionOverlay::updatePosition(const std::pair<double, double> &position)
{
    m_figure.functionValues.at(m_laserPointID) = position;
    WobbleFigure* figure = m_figureEditor.figure();
    auto laserPoint = figure->searchPoint(m_laserPointID);
    laserPoint->setEditable(true);
    laserPoint->setCenter(QPointF(position.first * m_figureScale, -(position.second * m_figureScale)));
    emit m_figureEditor.nodePositionUpdated();
}

}
}
}
}
