#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{


/**
 * @brief Filter model to filter on all available items in a OverlayGroupModel.
 **/
class OverlayGroupFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit OverlayGroupFilterModel(QObject *parent = nullptr);
    ~OverlayGroupFilterModel() override;

    /**
     * Swaps the enabled stated for the item at @p row.
     **/
    Q_INVOKABLE void swapEnabled(int row);

protected:
    bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

}
}
}
}
