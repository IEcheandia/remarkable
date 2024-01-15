#include "actionPointPositionSeamFigure.h"
#include <QPointF>

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

ActionPointPositionSeamFigure::ActionPointPositionSeamFigure(
    WobbleFigureEditor& figureEditor,
    RTC6::seamFigure::SeamFigure& figure,
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

void ActionPointPositionSeamFigure::undo()
{
    updatePosition(m_oldPosition);
}

void ActionPointPositionSeamFigure::redo()
{
    updatePosition(m_newPosition);
}

void ActionPointPositionSeamFigure::updatePosition(const std::pair<double, double>& position)
{
    m_figure.figure.at(m_laserPointID).endPosition = position;
    WobbleFigure* figure = m_figureEditor.figure();
    auto laserPoint = figure->searchPoint(m_laserPointID);
    laserPoint->setEditable(true);
    laserPoint->setCenter(QPointF(position.first * m_figureScale, -(position.second * m_figureScale)));
    emit m_figureEditor.nodePositionUpdated();
    emit m_figureEditor.seamPointsChanged();
}

}
}
}
}
