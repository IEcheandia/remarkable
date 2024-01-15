#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{

/**
 * Filter model to restrict the visibility of the results.
 * Only results with plotter number @link{plotterNumber} are allowed
 * If @link{nioFilter} is set, then all NIOs are also filtered
 **/

class PlotterFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(int plotterNumber READ plotterNumber WRITE setPlotterNumber NOTIFY plotterNumberChanged)

    Q_PROPERTY(bool nioFilter READ nioFilter WRITE setNioFilter NOTIFY nioFilterChanged)

public:
    explicit PlotterFilterModel(QObject* parent = nullptr);
    ~PlotterFilterModel() override;

    int plotterNumber() const
    {
        return m_plotterNumber;
    }
    void setPlotterNumber(int number);

    bool nioFilter() const
    {
        return m_nioFilter;
    }
    void setNioFilter(bool nioFilter);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

Q_SIGNALS:
    void plotterNumberChanged();
    void nioFilterChanged();

private:
    int m_plotterNumber = -1;
    bool m_nioFilter = false;
};

}
}

