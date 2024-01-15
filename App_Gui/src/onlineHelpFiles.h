#pragma once


#include <QtGlobal>
#include <QtCore/qobjectdefs.h>

namespace precitec
{
namespace gui
{
namespace onlineHelp
{
Q_NAMESPACE

/**
 * The Online Help Pdf's exported to QtQuick as precitec.gui.App.
 **/
enum class OnlineHelpFile
{
    HasNoPdf,
    SimulationSelectProductPdf,
    SimulationAssemblyPdf,
    SimulationViewerPdf
};
Q_ENUM_NS(OnlineHelpFile)

}
}
}

