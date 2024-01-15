#pragma once

#include "abstractWizardFilterModel.h"

namespace precitec
{
namespace gui
{

/**
 * The LinkedSeamWizardFilterModel is intended to filter the WizardModel::WizardComponent depending on the available hardware for a selected LinkedSeam.
 **/
class LinkedSeamWizardFilterModel : public AbstractWizardFilterModel
{
    Q_OBJECT

public:
    LinkedSeamWizardFilterModel(QObject *parent = nullptr);
    ~LinkedSeamWizardFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

}
}
