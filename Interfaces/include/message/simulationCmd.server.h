#pragma once

#include "message/simulationCmd.interface.h"

namespace precitec
{
namespace interface
{

template <>
class TSimulationCmd<MsgServer> : public TSimulationCmd<AbstractInterface>
{
public:
    TSimulationCmd() {};
};

}
}
