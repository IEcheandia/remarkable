#pragma once

#include <QObject>

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

Q_NAMESPACE

enum class FileType
{
    Seam,
    Wobble,
    Overlay,
    Basic,
    Dxf,
    None
};
Q_ENUM_NS(FileType)

}
}
}
}
