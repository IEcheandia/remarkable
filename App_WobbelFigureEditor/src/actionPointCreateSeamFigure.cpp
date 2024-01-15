#include "actionPointCreateSeamFigure.h"

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

ActionPointCreateSeamFigure::ActionPointCreateSeamFigure(
        WobbleFigureEditor& figureEditor,
        FileModel& fileModel,
        RTC6::seamFigure::SeamFigure oldFigure,
        RTC6::seamFigure::SeamFigure newFigure)
    : m_figureEditor{figureEditor}
    , m_fileModel{fileModel}
    , m_oldFigure{oldFigure}
    , m_newFigure{newFigure}
{
}

void ActionPointCreateSeamFigure::undo()
{
    updateFigure(m_oldFigure);
}

void ActionPointCreateSeamFigure::redo()
{
    updateFigure(m_newFigure);
}

void ActionPointCreateSeamFigure::updateFigure(RTC6::seamFigure::SeamFigure figure)
{
    m_fileModel.setSeamFigure(figure);
}

}
}
}
}
