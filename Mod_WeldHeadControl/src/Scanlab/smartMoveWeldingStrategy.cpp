#include "viWeldHead/Scanlab/smartMoveWeldingStrategy.h"
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

WeldingStrategy::WeldingStrategy(precitec::hardware::SmartMoveLowLevel& networkInterface)
    : AbstractWeldingStrategy()
    , m_networkInterface(networkInterface)
{
}

void WeldingStrategy::stop_Mark()
{
    GlobalCommandGenerator generator;
    generator.addProcessJob(precitec::smartMove::JobRepeats::Stop);

    SmartMoveGlobalInterpreter interpreter;
    interpreter.translateGlobalCommands(generator.generatedGlobalCommands());

    const auto& globalCommands = interpreter.currentCommandSeries();

    if (globalCommands.size() != 1)
    {
        wmLog(eError, "Number of commands is wrong\n");
        return;
    }

    if (!static_cast<bool>(m_networkInterface.processJob(globalCommands.front())))
    {
        wmLog(eError, "stop_Mark failed!\n");
    }
}

bool WeldingStrategy::done_Mark()
{
    return m_networkInterface.printReady();
}

int WeldingStrategy::setStatusSignalHead1Axis(precitec::Axis axis, precitec::StatusSignals value)
{
    // TODO: implement
    return 0;
}

}
}
}
