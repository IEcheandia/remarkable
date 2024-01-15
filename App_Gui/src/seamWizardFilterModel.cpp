#include "seamWizardFilterModel.h"
#include "wizardModel.h"

#include "guiConfiguration.h"

namespace precitec
{
namespace gui
{

SeamWizardFilterModel::SeamWizardFilterModel(QObject *parent)
    : AbstractWizardFilterModel(parent)
{
    connect(GuiConfiguration::instance(), &GuiConfiguration::seamIntervalsOnProductStructureChanged, this, &SeamWizardFilterModel::invalidate);
}

SeamWizardFilterModel::~SeamWizardFilterModel() = default;

bool SeamWizardFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto component = sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>();
    if (!isYAxisAvailable())
    {
        if (component == WizardModel::WizardComponent::SeamAxis)
        {
            return false;
        }
    }
    if (!isSensorGrabberAvailable())
    {
        if (component == WizardModel::WizardComponent::SeamCamera)
        {
            return false;
        }
    }
    if (!isLaserControlAvailable())
    {
        if (component == WizardModel::WizardComponent::SeamLaserControl)
        {
            return false;
        }
    }
    if (!scanTracker())
    {
        if (component == WizardModel::WizardComponent::SeamScanTracker)
        {
            return false;
        }
    }
    if (!isLWMAvailable())
    {
        if (component == WizardModel::WizardComponent::SeamLaserWeldingMonitor)
        {
            return false;
        }
    }
    if (!isExternalLWMAvailable())
    {
        if (component == WizardModel::WizardComponent::SeamExternalLWM)
        {
            return false;
        }
    }
    if (!isScanlabScannerAvailable())
    {
        if (component == WizardModel::WizardComponent::SeamScanLabScanner ||
            component == WizardModel::WizardComponent::SeamScanTracker2D)
        {
            return false;
        }
    }
    else
    {
        if (isScanTracker2DAvailable())
        {
            if (component == WizardModel::WizardComponent::SeamScanLabScanner)
            {
                return false;
            }
        }
        else
        {
            if (component == WizardModel::WizardComponent::SeamScanTracker2D)
            {
                return false;
            }
        }
    }
    if (!isIDMAvailable())
    {
        if (component == WizardModel::WizardComponent::SeamIDM)
        {
            return false;
        }
    }
    if (!GuiConfiguration::instance()->seamIntervalsOnProductStructure())
    {
        if (component == WizardModel::WizardComponent::SeamIntervalError)
        {
            return false;
        }
    }
    if (!zCollimator())
    {
        if (component == WizardModel::WizardComponent::SeamZCollimator)
        {
            return false;
        }
    }
    if (!sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 1).toBool())
    {
        return false;
    }
    return true;
}

}
}
