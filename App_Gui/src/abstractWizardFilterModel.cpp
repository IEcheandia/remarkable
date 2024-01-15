#include "abstractWizardFilterModel.h"
#include "wizardModel.h"

namespace precitec
{
namespace gui
{

AbstractWizardFilterModel::AbstractWizardFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &AbstractWizardFilterModel::yAxisAvailableChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::sensorGrabberAvailableChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::idmAvailableChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::ledAvailableChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::laserControlAvailableChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::ledCalibrationChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::scanTrackerChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::zCollimatorChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::lwmAvailableChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::externalLwmAvailableChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::lineLaser1AvailableChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::lineLaser2AvailableChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::lineLaser3AvailableChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::newsonScannerAvailableChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::scanlabScannerAvailableChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::cameraInterfaceTypeChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::scanTracker2DAvailableChanged, this, &AbstractWizardFilterModel::invalidate);
    connect(this, &AbstractWizardFilterModel::showProductionSetupChanged, this, &AbstractWizardFilterModel::invalidate);

}

AbstractWizardFilterModel::~AbstractWizardFilterModel() = default;

void AbstractWizardFilterModel::setYAxisAvailable(bool set)
{
    if (m_yAxisAvailable == set)
    {
        return;
    }
    m_yAxisAvailable = set;
    emit yAxisAvailableChanged();
}

void AbstractWizardFilterModel::setSensorGrabberAvailable(bool set)
{
    if (m_sensorGrabberAvailable == set)
    {
        return;
    }
    m_sensorGrabberAvailable = set;
    emit sensorGrabberAvailableChanged();
}

void AbstractWizardFilterModel::setIDMAvailable(bool set)
{
    if (m_idmAvailable == set)
    {
        return;
    }
    m_idmAvailable = set;
    emit idmAvailableChanged();
}

void AbstractWizardFilterModel::setCoaxCameraAvailable(bool set)
{
    if (m_coaxCameraAvailable == set)
    {
        return;
    }
    m_coaxCameraAvailable = set;
    emit coaxCameraAvailableChanged();
}

void AbstractWizardFilterModel::setScheimpflugCameraAvailable (bool set)
{
    if (m_scheimpflugCameraAvailable == set)
    {
        return;
    }
    m_scheimpflugCameraAvailable = set;
    emit scheimpflugCameraAvailableChanged();
}


void AbstractWizardFilterModel::setLEDCameraAvailable(bool set)
{
    if (m_ledCameraAvailable == set)
    {
        return;
    }
    m_ledCameraAvailable = set;
    emit ledCameraAvailableChanged();
}

void AbstractWizardFilterModel::setLEDAvailable(bool set)
{
    if (m_ledAvailable == set)
    {
        return;
    }
    m_ledAvailable = set;
    emit ledAvailableChanged();
}

void AbstractWizardFilterModel::setLaserControlAvailable(bool set)
{
    if (m_laserControlAvailable == set)
    {
        return;
    }
    m_laserControlAvailable = set;
    emit laserControlAvailableChanged();
}

void AbstractWizardFilterModel::setLEDCalibration(bool set)
{
    if (m_ledCalibration == set)
    {
        return;
    }
    m_ledCalibration = set;
    emit ledCalibrationChanged();
}

void AbstractWizardFilterModel::setScanTracker(bool set)
{
    if (m_scanTracker == set)
    {
        return;
    }
    m_scanTracker = set;
    emit scanTrackerChanged();
}

void AbstractWizardFilterModel::setZCollimator(bool set)
{
    if (m_zCollimator == set)
    {
        return;
    }
    m_zCollimator = set;
    emit zCollimatorChanged();
}

void AbstractWizardFilterModel::setLWMAvailable(bool set)
{
    if (m_lwmAvailable == set)
    {
        return;
    }
    m_lwmAvailable = set;
    emit lwmAvailableChanged();
}

void AbstractWizardFilterModel::setExternalLWMAvailable(bool set)
{
    if (m_externalLwmAvailable == set)
    {
        return;
    }
    m_externalLwmAvailable = set;
    emit externalLwmAvailableChanged();
}

void AbstractWizardFilterModel::setLineLaser1Available(bool set)
{
    if (m_lineLaser1Available == set)
    {
        return;
    }
    m_lineLaser1Available = set;
    emit lineLaser1AvailableChanged();
}

void AbstractWizardFilterModel::setLineLaser2Available(bool set)
{
    if (m_lineLaser2Available == set)
    {
        return;
    }
    m_lineLaser2Available = set;
    emit lineLaser2AvailableChanged();
}

void AbstractWizardFilterModel::setLineLaser3Available(bool set)
{
    if (m_lineLaser3Available == set)
    {
        return;
    }
    m_lineLaser3Available = set;
    emit lineLaser3AvailableChanged();
}

void AbstractWizardFilterModel::setNewsonScannerAvailable(bool set)
{
    if (m_newsonScannerAvailable == set)
    {
        return;
    }
    m_newsonScannerAvailable = set;
    emit newsonScannerAvailableChanged();
}

void AbstractWizardFilterModel::setScanlabScannerAvailable(bool set)
{
    if (m_scanlabScannerAvailable == set)
    {
        return;
    }
    m_scanlabScannerAvailable = set;
    emit scanlabScannerAvailableChanged();
}

void AbstractWizardFilterModel::setCameraInterfaceType(CameraInterfaceType type)
{
    if (m_cameraInterfaceType == type)
    {
        return;
    }
    m_cameraInterfaceType = type;
    emit cameraInterfaceTypeChanged();
}


void AbstractWizardFilterModel::setScanTracker2DAvailable(bool set)
{
    if (m_scanTracker2DAvailable == set)
    {
        return;
    }
    m_scanTracker2DAvailable = set;
    emit scanTracker2DAvailableChanged();
}

void AbstractWizardFilterModel::setShowProductionSetup (bool set)
{
    if (m_showProductionSetup == set)
    {
        return;
    }
    m_showProductionSetup = set;
    emit showProductionSetupChanged();
}

}
}
