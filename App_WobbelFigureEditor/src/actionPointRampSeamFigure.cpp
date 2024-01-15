#include "actionPointRampSeamFigure.h"

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

ActionPointRampSeamFigure::ActionPointRampSeamFigure(
        WobbleFigureEditor& figureEditor,
        RTC6::seamFigure::SeamFigure& figure,
        std::vector<RTC6::seamFigure::Ramp> oldRamps,
        std::vector<RTC6::seamFigure::Ramp> newRamps)
    : m_figureEditor{figureEditor}
    , m_figure{figure}
    , m_oldRamps{oldRamps}
    , m_newRamps{newRamps}
{
}

void ActionPointRampSeamFigure::undo()
{
    updateRamps(m_oldRamps);
}

void ActionPointRampSeamFigure::redo()
{
    updateRamps(m_newRamps);
}

void ActionPointRampSeamFigure::updateRamps(std::vector<RTC6::seamFigure::Ramp> ramps)
{
    m_figure.ramps = ramps;
    emit m_figureEditor.rampsChanged();
}

}
}
}
}
