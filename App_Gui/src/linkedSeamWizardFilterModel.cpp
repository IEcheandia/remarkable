#include "linkedSeamWizardFilterModel.h"
#include "wizardModel.h"

#include "guiConfiguration.h"

namespace precitec
{
namespace gui
{

LinkedSeamWizardFilterModel::LinkedSeamWizardFilterModel(QObject *parent)
    : AbstractWizardFilterModel(parent)
{
}

LinkedSeamWizardFilterModel::~LinkedSeamWizardFilterModel() = default;

bool LinkedSeamWizardFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto component = sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>();
    if (component != WizardModel::WizardComponent::SeamAssemblyImage)
    {
        return false;
    }
    if (!sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 1).toBool())
    {
        return false;
    }
    return true;
}

}
}
