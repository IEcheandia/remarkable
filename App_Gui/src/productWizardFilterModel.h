#pragma once

#include "abstractWizardFilterModel.h"

namespace precitec
{
namespace gui
{

/**
 * The ProductWizardFilterModel is intended to filter the WizardModel::WizardComponent depending on the available hardware for a selected product.
 **/
class ProductWizardFilterModel : public AbstractWizardFilterModel
{
    Q_OBJECT

public:
    ProductWizardFilterModel(QObject *parent = nullptr);
    ~ProductWizardFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

}
}
