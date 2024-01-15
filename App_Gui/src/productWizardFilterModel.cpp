#include "productWizardFilterModel.h"
#include "guiConfiguration.h"
#include "wizardModel.h"

namespace precitec
{
namespace gui
{

ProductWizardFilterModel::ProductWizardFilterModel(QObject *parent)
    : AbstractWizardFilterModel(parent)
{
}

ProductWizardFilterModel::~ProductWizardFilterModel() = default;

bool ProductWizardFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto component = sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>();
    if (!isSensorGrabberAvailable())
    {
        if (component == WizardModel::WizardComponent::ProductCamera)
        {
            return false;
        }
    }
    if (!isLaserControlAvailable())
    {
        if (component == WizardModel::WizardComponent::ProductLaserControl)
        {
            return false;
        }
    }
    if (!scanTracker())
    {
        if (component == WizardModel::WizardComponent::ProductScanTracker)
        {
            return false;
        }
    }
    if (!isLWMAvailable())
    {
        if (component == WizardModel::WizardComponent::ProductLaserWeldingMonitor)
        {
            return false;
        }
    }
    if (!isScanlabScannerAvailable())
    {
        if (component == WizardModel::WizardComponent::ProductScanLabScanner ||
            component == WizardModel::WizardComponent::ProductScanTracker2D)
        {
            return false;
        }
    }
    else
    {
        if (isScanTracker2DAvailable())
        {
            if (component == WizardModel::WizardComponent::ProductScanLabScanner)
            {
                return false;
            }
        }
        else
        {
            if (component == WizardModel::WizardComponent::ProductScanTracker2D)
            {
                return false;
            }
        }
    }
    if (!isIDMAvailable())
    {
        if (component == WizardModel::WizardComponent::ProductIDM)
        {
            return false;
        }
    }
    if (!zCollimator())
    {
        if (component == WizardModel::WizardComponent::ProductZCollimator)
        {
            return false;
        }
    }
    if (!sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 3).toBool())
    {
        return false;
    }
    return true;
}

}
}
