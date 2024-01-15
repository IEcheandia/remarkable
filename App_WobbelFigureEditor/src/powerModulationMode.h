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
namespace powerModulationMode
{

Q_NAMESPACE

enum class PowerModulationMode {
    Core = 1,
    Ring = 2,
    CoreAndRing = 8
};
Q_ENUM_NS(PowerModulationMode)

}
}
}
}
}
