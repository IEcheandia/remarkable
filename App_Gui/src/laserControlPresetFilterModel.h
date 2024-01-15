#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

/**
 * @brief Filter proxy model to filter on DataSets representing a reference curve
 **/
class LaserControlPresetFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    LaserControlPresetFilterModel(QObject *parent = nullptr);
    ~LaserControlPresetFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::LaserControlPresetFilterModel*)


