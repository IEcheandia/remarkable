#include "resultsFilterModel.h"
#include "resultsModel.h"

namespace precitec
{
namespace gui
{

ResultsFilterModel::ResultsFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &ResultsFilterModel::seamSeriesAvailableChanged, this, &ResultsFilterModel::invalidate);
}

ResultsFilterModel::~ResultsFilterModel() = default;

bool ResultsFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto component = sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 1).value<ResultsModel::ResultsComponent>();

    if (!m_seamSeriesAvailable)
    {
        if (component == ResultsModel::ResultsComponent::Series)
        {
            return false;
        }
    }
    return true;
}

void ResultsFilterModel::setSeamSeriesAvailable(bool set)
{
    if (m_seamSeriesAvailable == set)
    {
        return;
    }
    m_seamSeriesAvailable = set;
    emit seamSeriesAvailableChanged();
}

}
}

