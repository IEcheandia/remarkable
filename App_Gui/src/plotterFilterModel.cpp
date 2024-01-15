#include "plotterFilterModel.h"

#include <precitec/multicolorSet.h>
#include <precitec/dataSet.h>

using precitec::gui::components::plotter::MulticolorSet;
using precitec::gui::components::plotter::DataSet;

namespace precitec
{
namespace gui
{

PlotterFilterModel::PlotterFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortCaseSensitivity(Qt::CaseInsensitive);
    sort(0, Qt::DescendingOrder);

    connect(this, &PlotterFilterModel::plotterNumberChanged, this, &PlotterFilterModel::invalidate);
    connect(this, &PlotterFilterModel::nioFilterChanged, this, &PlotterFilterModel::invalidate);
}

PlotterFilterModel::~PlotterFilterModel() = default;

bool PlotterFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const auto index = sourceModel()->index(source_row, 0, source_parent);
    const auto& data = index.data(Qt::UserRole + 2);
    const auto isNio = index.data(Qt::UserRole + 10).toBool();

    if (m_nioFilter && isNio)
    {
        return false;
    }

    if (data.canConvert<MulticolorSet*>())
    {
        const auto multicolorSet = data.value<MulticolorSet*>();
        if (!multicolorSet)
        {
            return false;
        }
    }

    if (data.canConvert<std::vector<MulticolorSet*>>())
    {
        const auto& multicolorSets = data.value<std::vector<MulticolorSet*>>();
        auto allNull = true;
        for (auto ms : multicolorSets)
        {
            if (ms)
            {
                allNull = false;
                break;
            }
        }
        if (allNull)
        {
            return false;
        }
    }

    if (data.canConvert<DataSet*>())
    {
        const auto dataSet = data.value<DataSet*>();
        if (!dataSet)
        {
            return false;
        }
    }

    if (data.canConvert<std::vector<DataSet*>>())
    {
        const auto& dataSets = data.value<std::vector<DataSet*>>();
        auto allNull = true;
        for (auto ds : dataSets)
        {
            if (ds)
            {
                allNull = false;
                break;
            }
        }
        if (allNull)
        {
            return false;
        }
    }

    if (m_plotterNumber != -1)
    {
        const int plotterNumber = index.data(Qt::UserRole + 3).toInt();
        if (plotterNumber != -1 && plotterNumber != m_plotterNumber)
        {
            return false;
        }
    }

    return index.data(Qt::UserRole + 6).toBool();
}

void PlotterFilterModel::setPlotterNumber(int number)
{
    if (m_plotterNumber == number)
    {
        return;
    }
    m_plotterNumber = number;
    emit plotterNumberChanged();
}

void PlotterFilterModel::setNioFilter(bool nioFilter)
{
    if (m_nioFilter == nioFilter)
    {
        return;
    }
    m_nioFilter = nioFilter;
    emit nioFilterChanged();
}

}
}

