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

class ActionPointPropertieSeamLaserPower : public precitec::interface::ActionInterface
{
public:
    ActionPointPropertieSeamLaserPower(
            WobbleFigureEditor& figureEditor,
            RTC6::seamFigure::SeamFigure& figure,
            int laserPointID,
            double oldLaserPower,
            double newLaserPower);
    void undo() override;
    void redo() override;
    ~ActionPointPropertieSeamLaserPower() = default;

private:
    void updateLaserPower(double laserPower);

private:
    WobbleFigureEditor& m_figureEditor;
    RTC6::seamFigure::SeamFigure& m_figure;
    int m_laserPointID;
    double m_oldLaserPower;
    double m_newLaserPower;
};


}
}
}
}
