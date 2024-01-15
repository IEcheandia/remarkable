#pragma once

#include <QObject>

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{
namespace velocityLimits
{

Q_NAMESPACE

enum VelocityLimits {
    Default = -1,               //Use system setting
    MinValue = 1,
    MarkMaxValue = 9000,
    JumpMaxValue = 10000
};
Q_ENUM_NS(VelocityLimits)

}
}
}
}
}
