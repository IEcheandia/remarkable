#pragma once

#include "abstractWizardFilterModel.h"

namespace precitec
{
namespace gui
{

/**
 * The SeamSeriesWizardFilterModel is intended to filter the WizardModel::WizardComponent depending on the available hardware for a selected seam series.
 **/
class SeamSeriesWizardFilterModel : public AbstractWizardFilterModel
{
    Q_OBJECT

public:
    SeamSeriesWizardFilterModel(QObject *parent = nullptr);
    ~SeamSeriesWizardFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

}
}
