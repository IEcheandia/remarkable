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

class ActionPointPropertieSeamRingPower : public precitec::interface::ActionInterface
{
public:
    ActionPointPropertieSeamRingPower(
            WobbleFigureEditor& figureEditor,
            RTC6::seamFigure::SeamFigure& figure,
            int laserPointID,
            double oldRingPower,
            double newRingPower);
    void undo() override;
    void redo() override;
    ~ActionPointPropertieSeamRingPower() = default;

private:
    void updateRingPower(double ringPower);

private:
    WobbleFigureEditor& m_figureEditor;
    RTC6::seamFigure::SeamFigure& m_figure;
    int m_laserPointID;
    double m_oldRingPower;
    double m_newRingPower;
};


}
}
}
}
