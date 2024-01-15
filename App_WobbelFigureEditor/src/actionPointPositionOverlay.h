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

class ActionPointPositionOverlay : public precitec::interface::ActionInterface
{
public:
    // Todo: this class is the same like seamFigure, only the figure is different
    ActionPointPositionOverlay(
            WobbleFigureEditor& figureEditor,
            RTC6::function::OverlayFunction& figure,
            std::pair<double, double> oldPosition,
            std::pair<double, double> newPosition,
            int laserPointID,
            int figureScale);
    void undo() override;
    void redo() override;
    ~ActionPointPositionOverlay() = default;

private:
    void updatePosition(std::pair<double, double> const& position);

private:
    WobbleFigureEditor& m_figureEditor;
    RTC6::function::OverlayFunction& m_figure;
    std::pair<double, double> m_oldPosition;
    std::pair<double, double> m_newPosition;
    int m_laserPointID;
    int m_figureScale;
};


}
}
}
}
