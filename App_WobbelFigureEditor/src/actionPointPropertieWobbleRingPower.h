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

class ActionPointPropertieWobbleRingPower : public precitec::interface::ActionInterface
{
public:
    ActionPointPropertieWobbleRingPower(
            WobbleFigureEditor& figureEditor,
            RTC6::wobbleFigure::Figure& figure,
            int laserPointID,
            double oldRingPower,
            double newRingPower);
    void undo() override;
    void redo() override;
    ~ActionPointPropertieWobbleRingPower() = default;

private:
    void updateRingPower(double ringPower);

private:
    WobbleFigureEditor& m_figureEditor;
    RTC6::wobbleFigure::Figure& m_figure;
    int m_laserPointID;
    double m_oldRingPower;
    double m_newRingPower;
};


}
}
}
}
