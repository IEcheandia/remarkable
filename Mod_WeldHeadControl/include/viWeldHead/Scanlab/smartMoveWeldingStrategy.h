#pragma once
#include "AbstractWeldingStrategy.h"


namespace precitec
{
namespace hardware
{
class SmartMoveLowLevel;
namespace smartMove
{

class WeldingStrategy : public AbstractWeldingStrategy
{
public:
    WeldingStrategy(precitec::hardware::SmartMoveLowLevel& networkInterface);
    void stop_Mark() override;
    bool done_Mark() override;
    int setStatusSignalHead1Axis(precitec::Axis axis, precitec::StatusSignals value) override;

private:
    precitec::hardware::SmartMoveLowLevel& m_networkInterface;
};

}
}
}
