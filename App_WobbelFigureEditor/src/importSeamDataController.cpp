#include "importSeamDataController.h"
#include "seam.h"
#include "parameterSet.h"
#include "parameter.h"

#include "figureEditorSettings.h"

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

ImportSeamDataController::ImportSeamDataController(QObject* parent)
    : AbstractDataExchangeController(parent)
{
    connect(this, &AbstractDataExchangeController::productModelIndexChanged, this, &ImportSeamDataController::reset);
    connect(this, &AbstractDataExchangeController::seamChanged, this, &ImportSeamDataController::importVelocityFromSeam);
}

ImportSeamDataController::~ImportSeamDataController() = default;

void ImportSeamDataController::reset()
{
    m_velocity = 0.0;
    emit velocityChanged();
}

double ImportSeamDataController::umPerSecondToMMPerSecond(double velocityInUmPerSeconds)
{
    return velocityInUmPerSeconds * 0.001;
}

void ImportSeamDataController::importVelocityFromSeam()
{
    if (!seam())
    {
        return;
    }

    m_velocity = umPerSecondToMMPerSecond(seam()->velocity());
    emit velocityChanged();
}

void ImportSeamDataController::importData()
{
    FigureEditorSettings::instance()->setScannerSpeed(m_velocity);
}

}
}
}
}

