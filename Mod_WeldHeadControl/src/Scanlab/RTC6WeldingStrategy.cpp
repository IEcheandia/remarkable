#include "viWeldHead/Scanlab/RTC6WeldingStrategy.h"

#include "rtc6.h"

#include <cstdint>

using precitec::StatusSignals;

namespace RTC6
{

void WeldingStrategy::stop_Mark()
{
    stop_execution();
    reset_error(0x20); // clear RTC6_BUSY
    write_da_1(0);
    if (m_enableDebugMode)
    {
        stop_trigger();
    }
}

bool WeldingStrategy::done_Mark()
{
    UINT oStatus = 0;
    UINT oPos = 0;
    get_status(&oStatus, &oPos);
    if (!(oStatus & 1))
    {
        return true;
    }
    return false;
}

int WeldingStrategy::setStatusSignalHead1Axis(precitec::Axis axis, precitec::StatusSignals value)
{
    auto command = 0;
    switch (value)
    {
        case StatusSignals::XY2_100_Statusword:
            command = 0x0500;
            break;
        case StatusSignals::CurrentPosition:
            command = 0x0501;
            break;
        case StatusSignals::TargetPosition:
            command = 0x0502;
            break;
        case StatusSignals::PositionDifference:
            command = 0x0503;
            break;
        case StatusSignals::CurrentCurrent:
            command = 0x0504;
            break;
        case StatusSignals::RelativeGalvanometerscanner:
            command = 0x0505;
            break;
        case StatusSignals::CurrentSpeed:
            command = 0x0506;
            break;
        case StatusSignals::CurrentPositionZAxis:
            command = 0x0512;
            break;
        case StatusSignals::TemperatureGalvanometerscanner:
            command = 0x0514;
            break;
        case StatusSignals::TemperatureServocard:
            command = 0x0515;
            break;
        default:
            return command;
    }

    control_command(1, static_cast<std::uint32_t>(axis), command);
    return command;
}
}