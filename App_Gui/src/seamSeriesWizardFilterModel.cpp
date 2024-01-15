#include "seamSeriesWizardFilterModel.h"
#include "wizardModel.h"

namespace precitec
{
namespace gui
{

SeamSeriesWizardFilterModel::SeamSeriesWizardFilterModel(QObject *parent)
    : AbstractWizardFilterModel(parent)
{
}

SeamSeriesWizardFilterModel::~SeamSeriesWizardFilterModel() = default;

bool SeamSeriesWizardFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto component = sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>();
    if (!isLaserControlAvailable())
    {
        if (component == WizardModel::WizardComponent::SeamSeriesLaserControl)
        {
            return false;
        }
    }
    if (!scanTracker())
    {
        if (component == WizardModel::WizardComponent::SeamSeriesScanTracker)
        {
            return false;
        }
    }
    if (!isLWMAvailable())
    {
        if (component == WizardModel::WizardComponent::SeamSeriesLaserWeldingMonitor)
        {
            return false;
        }
    }
    if (!isScanlabScannerAvailable())
    {
        if (component == WizardModel::WizardComponent::SeamSeriesAcquireScanField ||
            component == WizardModel::WizardComponent::SeamSeriesScanLabScanner ||
            component == WizardModel::WizardComponent::SeamSeriesScanTracker2D)
        {
            return false;
        }
    }
    else
    {
        if (isScanTracker2DAvailable())
        {
            if (component == WizardModel::WizardComponent::SeamSeriesAcquireScanField ||
                component == WizardModel::WizardComponent::SeamSeriesScanLabScanner)
            {
                return false;
            }
        }
        else
        {
            if (component == WizardModel::WizardComponent::SeamSeriesScanTracker2D)
            {
                return false;
            }
        }
    }
    if (!isIDMAvailable())
    {
        if (component == WizardModel::WizardComponent::SeamSeriesIDM)
        {
            return false;
        }
    }
    if (!zCollimator())
    {
        if (component == WizardModel::WizardComponent::SeamSeriesZCollimator)
        {
            return false;
        }
    }
    if (!sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 4).toBool())
    {
        return false;
    }
    return true;
}

}
}
