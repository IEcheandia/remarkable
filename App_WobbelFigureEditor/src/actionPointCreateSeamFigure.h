#pragma once
#include "../../Interfaces/include/event/actioninterface.h"
#include "editorDataTypes.h"
#include "LaserPoint.h"
#include "WobbleFigureEditor.h"
#include "FileModel.h"
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

class ActionPointCreateSeamFigure : public precitec::interface::ActionInterface
{
public:
    ActionPointCreateSeamFigure(
            WobbleFigureEditor& figureEditor,
            FileModel& fileModel,
            RTC6::seamFigure::SeamFigure oldFigure,
            RTC6::seamFigure::SeamFigure newFigure);
    void undo() override;
    void redo() override;
    ~ActionPointCreateSeamFigure() = default;

private:
    void updateFigure(RTC6::seamFigure::SeamFigure figure);

private:
    WobbleFigureEditor& m_figureEditor;
    FileModel& m_fileModel;
    RTC6::seamFigure::SeamFigure m_oldFigure;
    RTC6::seamFigure::SeamFigure m_newFigure;
};


}
}
}
}
