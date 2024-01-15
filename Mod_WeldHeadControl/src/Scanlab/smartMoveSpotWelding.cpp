#include "viWeldHead/Scanlab/smartMoveSpotWelding.h"
#include "viWeldHead/Scanlab/smartMoveControl.h"
#include "viWeldHead/Scanlab/smartMoveLowLevel.h"
#include "viWeldHead/Scanlab/globalCommandGenerator.h"
#include "viWeldHead/Scanlab/smartMoveGlobalInterpreter.h"

#include "module/moduleLogger.h"

namespace precitec
{
namespace hardware
{
namespace smartMove
{

SpotWelding::SpotWelding(precitec::hardware::SmartMoveControl& control)
    : AbstractSpotWelding()
    , m_control(control)
{
}

SpotWelding::~SpotWelding()
{
    // TODO: abort execution
}

void SpotWelding::start_Mark() const
{
    // send align

    GlobalCommandGenerator generator{};
    generator.addSystemOperationMode(SystemOperationModes::Service, true);
    generator.addInputMode(InputModes::Alignment, true);
    // we are already on position, maybe we need to jump again...
    generator.addLaserPower(m_laserPowerCenterInPercent * 0.01, m_laserPowerRingInPercent * 0.01);
    generator.addLaserOnForAlignment(std::make_pair(sysTsFromSec(m_durationInSec), sysTsFromSec(5 * 60)), true);

    SmartMoveGlobalInterpreter interpreter{};
    interpreter.translateGlobalCommands(generator.generatedGlobalCommands());

    const auto& commands{interpreter.currentCommandSeries()};
    for (const auto& command: commands)
    {
        if (!m_control.networkInterface().sendCommand(command, "Alignment command empty"))
        {
            wmLog(eError, "Send command %s for spot welding failed\n", command);
            break;
        }

    }
}

void SpotWelding::define_Mark(double weldingDurationInSec, double laserPowerCenterInPercent, double laserPowerRingInPercent)
{
    AbstractSpotWelding::define_Mark(weldingDurationInSec, laserPowerCenterInPercent, laserPowerRingInPercent);
}

int SpotWelding::sysTsFromSec(double sec) const
{
    return sec / m_control.sysTs();
}

}
}
}

