#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace storage
{

class GraphSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    GraphSortModel(QObject *parent = nullptr);
    ~GraphSortModel() override;

protected:
    bool lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const override;
};

}
}
