#pragma once

#include <QSortFilterProxyModel>
#include <QPointer>

namespace precitec
{
namespace gui
{

class IntervalErrorModel;

/**
 * Sorts the Errors on Interval
 **/
class IntervalErrorFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit IntervalErrorFilterModel(QObject *parent = nullptr);
    ~IntervalErrorFilterModel() override;

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::IntervalErrorFilterModel*)

