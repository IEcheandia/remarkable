#pragma once
#include "../../Interfaces/include/event/actioninterface.h"
#include "editorDataTypes.h"
#include "LaserPoint.h"
#include "WobbleFigureEditor.h"
#include <stdlib.h>
#include <utility>

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

class WobbleFigure;

class ActionPointPositionSeamFigure : public precitec::interface::ActionInterface
{
public:
    ActionPointPositionSeamFigure(
            WobbleFigureEditor& figureEditor,
            RTC6::seamFigure::SeamFigure& figure,
            std::pair<double, double> oldPosition,
            std::pair<double, double> newPosition,
            int laserPointID,
            int figureScale);
    void undo() override;
    void redo() override;
    ~ActionPointPositionSeamFigure() = default;

private:
    void updatePosition(std::pair<double, double> const& position);

private:
    WobbleFigureEditor& m_figureEditor;
    RTC6::seamFigure::SeamFigure& m_figure;
    std::pair<double, double> m_oldPosition;
    std::pair<double, double> m_newPosition;
    int m_laserPointID;
    int m_figureScale;
};


}
}
}
}
