#pragma once

#include "abstractWizardFilterModel.h"

namespace precitec
{
namespace gui
{

/**
 * The SeamWizardFilterModel is intended to filter the WizardModel::WizardComponent depending on the available hardware for a selected seam.
 **/
class SeamWizardFilterModel : public AbstractWizardFilterModel
{
    Q_OBJECT

public:
    SeamWizardFilterModel(QObject *parent = nullptr);
    ~SeamWizardFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

}
}
