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

class ActionPointPropertieWobbleLaserPower : public precitec::interface::ActionInterface
{
public:
    ActionPointPropertieWobbleLaserPower(
            WobbleFigureEditor& figureEditor,
            RTC6::wobbleFigure::Figure& figure,
            int laserPointID,
            double oldLaserPower,
            double newLaserPower);
    void undo() override;
    void redo() override;
    ~ActionPointPropertieWobbleLaserPower() = default;

private:
    void updateLaserPower(double laserPower);

private:
    WobbleFigureEditor& m_figureEditor;
    RTC6::wobbleFigure::Figure& m_figure;
    int m_laserPointID;
    double m_oldLaserPower;
    double m_newLaserPower;
};


}
}
}
}
