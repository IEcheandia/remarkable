#pragma once
#include "common/definesScanlab.h"

namespace precitec
{
namespace hardware
{

class AbstractWeldingStrategy
{
public:
    virtual ~AbstractWeldingStrategy();
    virtual void stop_Mark() = 0;
    virtual bool done_Mark() = 0;
    virtual int setStatusSignalHead1Axis(precitec::Axis axis, precitec::StatusSignals value) = 0;
};

}
}
