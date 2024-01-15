import QtQuick 2.12
import QtQuick.Controls 2.3

import precitec.gui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication

ToolButton {
    property var drawer: null
    property var pdfFile: OnlineHelp.HasNoPdf

    onClicked: {
        if (drawer != null)
        {
            if (!drawer.opened)
            {
                switch(pdfFile)
                {
                    case OnlineHelp.HasNoPdf:
                        break;
                    case OnlineHelp.SimulationSelectProductPdf:
                        drawer.file = "WM-TC_BASIS_Simulation_Select-Product";
                        break;
                    case OnlineHelp.SimulationAssemblyPdf:
                        drawer.file = "WM-TC_BASIS_Simulation_Select-Assembly";
                        break;
                    case OnlineHelp.SimulationViewerPdf:
                        drawer.file = "WM-TC_BASIS_Simulation_Unterkapitel";
                        break;
                    default:
                        drawer.file = "";
                }

                drawer.open();
            }
            else
            {
                drawer.close();
            }
        }
    }

    onPdfFileChanged: {
        if (drawer != null)
        {
            drawer.close();
        }
    }
}
