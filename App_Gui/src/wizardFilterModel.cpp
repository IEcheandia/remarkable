#include "wizardFilterModel.h"
#include "wizardModel.h"

namespace precitec
{
namespace gui
{

WizardFilterModel::WizardFilterModel(QObject *parent)
    : AbstractWizardFilterModel(parent)
{
}

WizardFilterModel::~WizardFilterModel() = default;

bool WizardFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto component = sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>();
    if (!isYAxisAvailable())
    {
        if (component == WizardModel::WizardComponent::Axis)
        {
            return false;
        }
    }
    if (!isSensorGrabberAvailable())
    {
        if (component == WizardModel::WizardComponent::Camera ||
            component == WizardModel::WizardComponent::ToolCenterPoint)
        {
            return false;
        }
    }
    if (cameraInterfaceType() == CameraInterfaceType::GigE && isScheimpflugCameraAvailable())
    { 
        //Scheimpflug GigE Camera: the calibration is done only once with the chessboard
        if (component == WizardModel::WizardComponent::CameraCalibration)
        {
            return false;
        }
        if (component == WizardModel::WizardComponent::CameraChessboardCalibration && !showProductionSetup())
        {
            return false;
        }
    }
    else
    {
        if (component == WizardModel::WizardComponent::CameraChessboardCalibration )
        {
            return false;
        }
    }
    if (!isIDMAvailable())
    {
        if (component == WizardModel::WizardComponent::IDM)
        {
            return false;
        }
    }
    if (!isLWMAvailable())
    {
        if (component == WizardModel::WizardComponent::LWM)
        {
            return false;
        }
    }
    if (!isExternalLWMAvailable())
    {
        if (component == WizardModel::WizardComponent::ExternalLWM)
        {
            return false;
        }
    }
    // TODO: remove once FreeRDP is working
    else
    {
        if (component == WizardModel::WizardComponent::ExternalLWM)
        {
            return false;
        }
    }
    if (!isIDMAvailable() || !isNewsonScannerAvailable())
    {
        if (component == WizardModel::WizardComponent::IDMCalibration ||
            component == WizardModel::WizardComponent::ToolCenterPointOCT)
        {
            return false;
        }
    }
    if (!(isIDMAvailable() || isCoaxCameraAvailable()) || !isScanlabScannerAvailable())
    {
        if (component == WizardModel::WizardComponent::ScanfieldCalibration)
        {
            return false;
        }
    }
    if (!isLineLaserAvailable())
    {
        if (component == WizardModel::WizardComponent::CameraCalibration)
        {
            return false;
        }
    }
    if (!isLEDAvailable() || !ledCalibration())
    {
        if (component == WizardModel::WizardComponent::LEDCalibration)
        {
            return false;
        }
    }
    if (!isLaserControlAvailable())
    {
        if (component == WizardModel::WizardComponent::LaserControl ||
            component == WizardModel::WizardComponent::LaserControlDelay)
        {
            return false;
        }
    }
    if (!scanTracker())
    {
        if (component == WizardModel::WizardComponent::ScanTracker)
        {
            return false;
        }
    }
    if (!zCollimator())
    {
        if (component == WizardModel::WizardComponent::FocusPosition)
        {
            return false;
        }
    }
    if (!isScanlabScannerAvailable())
    {
        if (component == WizardModel::WizardComponent::FigureEditor || component == WizardModel::WizardComponent::ScanLabScanner)
        {
            return false;
        }
    }
    if (sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 1).toBool())
    {
        return false;
    }
    if (sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 3).toBool())
    {
        return false;
    }
    if (sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 4).toBool())
    {
        return false;
    }
    return true;
}

}
}
