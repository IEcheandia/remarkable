#include "wizardModel.h"

#include <QMetaEnum>

namespace precitec
{
namespace gui
{

WizardModel::WizardModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

WizardModel::~WizardModel() = default;

QHash<int, QByteArray> WizardModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::DecorationRole, QByteArrayLiteral("icon")},
        {Qt::UserRole + 1, QByteArrayLiteral("subitem")},
        {Qt::UserRole + 2, QByteArrayLiteral("component")},
        {Qt::UserRole + 3, QByteArrayLiteral("productItem")},
        {Qt::UserRole + 4, QByteArrayLiteral("seamSeriesItem")}
    };
}

int WizardModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return QMetaEnum::fromType<WizardComponent>().keyCount();
}

QVariant WizardModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto component = static_cast<WizardComponent>(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        return name(component);
    case Qt::DecorationRole:
        return iconName(component);
    case Qt::UserRole + 1:
        return subItem(component);
    case Qt::UserRole + 2:
        return QVariant::fromValue(component);
    case Qt::UserRole + 3:
        return productItem(component);
    case Qt::UserRole + 4:
        return seamSeriesItem(component);
    default:
        return {};
    }
}

QString WizardModel::name(WizardComponent component) const
{
    switch (component)
    {
    case WizardComponent::Axis:
        return tr("Hardware");
    case WizardComponent::Camera:
        return tr("HW-ROI");
    case WizardComponent::ToolCenterPoint:
        return tr("Tool Center Point");
    case WizardComponent::ScanTracker:
    case WizardComponent::ProductScanTracker:
    case WizardComponent::SeamSeriesScanTracker:
    case WizardComponent::SeamScanTracker:
        return tr("Scan Tracker");
    case WizardComponent::FigureEditor:
        return tr("Figure Editor");
    case WizardComponent::ProductStructure:
        return tr("Product Structure");
    case WizardComponent::SeamAssemblyImage:
        return tr("Assembly Image");
    case WizardComponent::ProductCamera:
    case WizardComponent::SeamCamera:
        return tr("Sensor");
    case WizardComponent::SeamAxis:
        return tr("Axis");
    case WizardComponent::SeamDetection:
        return tr("Detection");
    case WizardComponent::ProductError:
    case WizardComponent::SeamSeriesError:
    case WizardComponent::SeamError:
        return tr("Errors");
    case WizardComponent::SeamReferenceCurves:
        return tr("Reference Curves");
    case WizardComponent::LaserControl:
    case WizardComponent::ProductLaserControl:
    case WizardComponent::SeamSeriesLaserControl:
    case WizardComponent::SeamLaserControl:
        return tr("Laser Control");
    case WizardComponent::IDM:
    case WizardComponent::ProductIDM:
    case WizardComponent::SeamSeriesIDM:
    case WizardComponent::SeamIDM:
        return tr("IDM");
    case WizardComponent::IDMCalibration:
        return tr("Calibration IDM");
    case WizardComponent::ScanfieldCalibration:
        return tr("Calibration Scanfield");
    case WizardComponent::CameraCalibration:
    case WizardComponent::CameraChessboardCalibration:
        return tr("Calibration Camera");
    case WizardComponent::ToolCenterPointOCT:
        return tr("IDM Tool Center Point");
    case WizardComponent::LEDCalibration:
        return tr("Calibration LED");
    case WizardComponent::FocusPosition:
        return tr("Focus Position");
    case WizardComponent::LaserControlDelay:
        return tr("Laser Control Delay");
    case WizardComponent::LWM:
    case WizardComponent::ProductLaserWeldingMonitor:
    case WizardComponent::SeamSeriesLaserWeldingMonitor:
    case WizardComponent::SeamLaserWeldingMonitor:
    case WizardComponent::ExternalLWM:
    case WizardComponent::SeamExternalLWM:
        return tr("Laser Welding Monitor");
    case WizardComponent::ScanLabScanner:
    case WizardComponent::ProductScanLabScanner:
    case WizardComponent::SeamSeriesScanLabScanner:
    case WizardComponent::SeamScanLabScanner:
        return tr("Scanner");
    case WizardComponent::ProductColorMaps:
        return tr("Color Maps");
    case WizardComponent::SeamSeriesAcquireScanField:
        return tr("Acquire Scan Field Image");
    case WizardComponent::ProductHardwareParametersOverview:
        return tr("Hardware Parameters Overview");
    case WizardComponent::ProductDetectionOverview:
        return tr("Detection Overview");
    case WizardComponent::SeamIntervalError:
        return tr("Interval Errors");
    case WizardComponent::ProductZCollimator:
    case WizardComponent::SeamSeriesZCollimator:
    case WizardComponent::SeamZCollimator:
        return tr("Z Collimator");
    case WizardComponent::ProductScanTracker2D:
    case WizardComponent::SeamSeriesScanTracker2D:
    case WizardComponent::SeamScanTracker2D:
        return tr("ScanTracker 2D");
    default:
        return {};
    }
}

QString WizardModel::iconName(WizardComponent component) const
{
    switch (component)
    {
    case WizardComponent::Axis:
        return QStringLiteral("wizard-hardware");
    case WizardComponent::Camera:
        return QStringLiteral("wizard-roi");
    case WizardComponent::ToolCenterPoint:
        return QStringLiteral("crosshairs");
    case WizardComponent::ScanTracker:
    case WizardComponent::ProductScanTracker:
    case WizardComponent::SeamSeriesScanTracker:
    case WizardComponent::SeamScanTracker:
    case WizardComponent::SeamSeriesAcquireScanField:
    case WizardComponent::ProductScanTracker2D:
    case WizardComponent::SeamSeriesScanTracker2D:
    case WizardComponent::SeamScanTracker2D:
        return QStringLiteral("wizard-scantracker");
    case WizardComponent::FigureEditor:
        return QStringLiteral("wizard-figure-editor");
    case WizardComponent::ProductStructure:
        return QStringLiteral("wizard-product-structure");
    case WizardComponent::SeamAssemblyImage:
        return QStringLiteral("view-assembly-image");
    case WizardComponent::SeamAxis:
        return QStringLiteral("wizard-axis");
    case WizardComponent::ProductCamera:
    case WizardComponent::SeamCamera:
        return QStringLiteral("wizard-sensor");
    case WizardComponent::SeamDetection:
    case WizardComponent::ProductDetectionOverview:
        return QStringLiteral("wizard-detection");
    case WizardComponent::ProductError:
    case WizardComponent::SeamSeriesError:
    case WizardComponent::SeamError:
    case WizardComponent::SeamIntervalError:
        return QStringLiteral("wizard-sumerror");
    case WizardComponent::SeamReferenceCurves:
        return QStringLiteral("wizard-reference");
    case WizardComponent::LaserControl:
    case WizardComponent::ProductLaserControl:
    case WizardComponent::SeamSeriesLaserControl:
    case WizardComponent::SeamLaserControl:
    case WizardComponent::LaserControlDelay:
        return QStringLiteral("wizard-lasercontrol");
    case WizardComponent::ToolCenterPointOCT:
        return QStringLiteral("crosshairs");
    case WizardComponent::IDMCalibration:
    case WizardComponent::CameraCalibration:
    case WizardComponent::CameraChessboardCalibration:
    case WizardComponent::LEDCalibration:
    case WizardComponent::ScanfieldCalibration:
        return QStringLiteral("wizard-calibration");
    case WizardComponent::FocusPosition:
    case WizardComponent::ProductZCollimator:
    case WizardComponent::SeamSeriesZCollimator:
    case WizardComponent::SeamZCollimator:
        return QStringLiteral("menu-icon_optics-set");
    case WizardComponent::LWM:
    case WizardComponent::ProductLaserWeldingMonitor:
    case WizardComponent::SeamSeriesLaserWeldingMonitor:
    case WizardComponent::SeamLaserWeldingMonitor:
    case WizardComponent::ExternalLWM:
    case WizardComponent::SeamExternalLWM:
        return QStringLiteral("wizard-lwm");
    case WizardComponent::ScanLabScanner:
    case WizardComponent::ProductScanLabScanner:
    case WizardComponent::SeamSeriesScanLabScanner:
    case WizardComponent::SeamScanLabScanner:
        return QStringLiteral("wizard-scanner");
    case WizardComponent::IDM:
    case WizardComponent::ProductIDM:
    case WizardComponent::SeamSeriesIDM:
    case WizardComponent::SeamIDM:
        return QStringLiteral("menu-icon_settings");
    case WizardComponent::ProductColorMaps:
        return QStringLiteral("view-plot");
    case WizardComponent::ProductHardwareParametersOverview:
        return QStringLiteral("wizard-hardware");
    default:
        return QString();
    }
}

bool WizardModel::subItem(WizardComponent component) const
{
    switch (component)
    {
    case WizardComponent::SeamAssemblyImage:
    case WizardComponent::SeamCamera:
    case WizardComponent::SeamAxis:
    case WizardComponent::SeamDetection:
    case WizardComponent::SeamError:
    case WizardComponent::SeamReferenceCurves:
    case WizardComponent::SeamLaserControl:
    case WizardComponent::SeamScanTracker:
    case WizardComponent::SeamLaserWeldingMonitor:
    case WizardComponent::SeamExternalLWM:
    case WizardComponent::SeamScanLabScanner:
    case WizardComponent::SeamIDM:
    case WizardComponent::SeamIntervalError:
    case WizardComponent::SeamZCollimator:
    case WizardComponent::SeamScanTracker2D:
        return true;
    default:
        return false;
    }
}

bool WizardModel::productItem(WizardComponent component) const
{
    switch (component)
    {
    case WizardComponent::ProductColorMaps:
    case WizardComponent::ProductHardwareParametersOverview:
    case WizardComponent::ProductDetectionOverview:
    case WizardComponent::ProductCamera:
    case WizardComponent::ProductError:
    case WizardComponent::ProductLaserControl:
    case WizardComponent::ProductScanTracker:
    case WizardComponent::ProductLaserWeldingMonitor:
    case WizardComponent::ProductScanLabScanner:
    case WizardComponent::ProductIDM:
    case WizardComponent::ProductZCollimator:
    case WizardComponent::ProductScanTracker2D:
        return true;
    default:
        return false;
    }
}

bool WizardModel::seamSeriesItem(WizardComponent component) const
{
    switch (component)
    {
    case WizardComponent::SeamSeriesAcquireScanField:
    case WizardComponent::SeamSeriesError:
    case WizardComponent::SeamSeriesLaserControl:
    case WizardComponent::SeamSeriesScanTracker:
    case WizardComponent::SeamSeriesLaserWeldingMonitor:
    case WizardComponent::SeamSeriesScanLabScanner:
    case WizardComponent::SeamSeriesIDM:
    case WizardComponent::SeamSeriesZCollimator:
    case WizardComponent::SeamSeriesScanTracker2D:
        return true;
    default:
        return false;
    }
}

QModelIndex WizardModel::indexForComponent(WizardComponent component) const
{
    return index(int(component), 0);
}

}
}
