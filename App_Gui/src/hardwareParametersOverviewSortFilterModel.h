#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

class HardwareParametersOverviewSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit HardwareParametersOverviewSortFilterModel(QObject *parent = nullptr);
    ~HardwareParametersOverviewSortFilterModel() override;

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

};

}
}

Q_DECLARE_METATYPE(precitec::gui::HardwareParametersOverviewSortFilterModel*)
