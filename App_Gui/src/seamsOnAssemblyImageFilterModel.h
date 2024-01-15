#pragma once
#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

/**
 * Filter model for SeamsOnAssemblyImageModel. Removes the entries with a QPointF of @c -1x-1.
 **/
class SeamsOnAssemblyImageFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SeamsOnAssemblyImageFilterModel(QObject *parent = nullptr);
    ~SeamsOnAssemblyImageFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

}
}
