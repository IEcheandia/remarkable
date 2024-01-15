#pragma once

#include "abstractWizardFilterModel.h"

namespace precitec
{
namespace gui
{

/**
 * The WizardFilterModel is intended to filter the WizardModel::WizardComponent depending on the available hardware.
 **/
class WizardFilterModel : public AbstractWizardFilterModel
{
    Q_OBJECT

public:
    WizardFilterModel(QObject *parent = nullptr);
    ~WizardFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

}
}
