#pragma once
#include "AbstractWeldingStrategy.h"

namespace RTC6
{
class WeldingStrategy : public precitec::hardware::AbstractWeldingStrategy
{
public:
    void stop_Mark() override;
    bool done_Mark() override;
    int setStatusSignalHead1Axis(precitec::Axis axis, precitec::StatusSignals value) override;

private:
    bool m_enableDebugMode = false;
};
}