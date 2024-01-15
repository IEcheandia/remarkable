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

class ActionPointPropertieSeamVelocity : public precitec::interface::ActionInterface
{
public:
    ActionPointPropertieSeamVelocity(
            WobbleFigureEditor& figureEditor,
            RTC6::seamFigure::SeamFigure& figure,
            int laserPointID,
            double oldVelocity,
            double newVelocity);
    void undo() override;
    void redo() override;
    ~ActionPointPropertieSeamVelocity() = default;

private:
    void updateVelocity(double velocity);

private:
    WobbleFigureEditor& m_figureEditor;
    RTC6::seamFigure::SeamFigure& m_figure;
    int m_laserPointID;
    double m_oldVelocity;
    double m_newVelocity;
};


}
}
}
}
