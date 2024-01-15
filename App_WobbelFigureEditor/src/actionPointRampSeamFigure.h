#pragma once
#include "../../Interfaces/include/event/actioninterface.h"
#include "editorDataTypes.h"
#include "LaserPoint.h"
#include "WobbleFigureEditor.h"
#include <stdlib.h>
#include <utility>

using namespace precitec::scantracker::components::wobbleFigureEditor;

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

class ActionPointRampSeamFigure : public precitec::interface::ActionInterface
{
public:
    ActionPointRampSeamFigure(
            WobbleFigureEditor& figureEditor,
            RTC6::seamFigure::SeamFigure& figure,
            std::vector<RTC6::seamFigure::Ramp> oldRamps,
            std::vector<RTC6::seamFigure::Ramp> newRamps);
    void undo() override;
    void redo() override;
    ~ActionPointRampSeamFigure() = default;

private:
    void updateRamps(std::vector<RTC6::seamFigure::Ramp> ramps);

private:
    WobbleFigureEditor& m_figureEditor;
    RTC6::seamFigure::SeamFigure& m_figure;
    std::vector<RTC6::seamFigure::Ramp> m_oldRamps;
    std::vector<RTC6::seamFigure::Ramp> m_newRamps;
};


}
}
}
}
