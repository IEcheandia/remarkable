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

Q_NAMESPACE

enum PowerLimits {
    Default = -1,               //Use system setting
    MinValue = 0,
    DigitalMaxValue = 63,
    AnalogMaxValue = 100
};
Q_ENUM_NS(PowerLimits)

}
}
}
}
