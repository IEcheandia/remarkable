#include "FilterConnectorSortModel.h"
#include "FilterConnector.h"

using namespace precitec::gui::components::grapheditor;

FilterConnectorSortModel::FilterConnectorSortModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &FilterConnectorSortModel::searchTextChanged, this, &FilterConnectorSortModel::invalidate);
}

FilterConnectorSortModel::~FilterConnectorSortModel() = default;

QString FilterConnectorSortModel::getSearchText() const
{
    return m_searchText;
}

void FilterConnectorSortModel::setSearchText(const QString& searchText)
{
    if (m_searchText == searchText)
    {
        return;
    }
    m_searchText = searchText;
    emit searchTextChanged();
}

bool FilterConnectorSortModel::searchOnlyFree() const
{
    return m_searchFreeOnes;
}

void FilterConnectorSortModel::setSearchOnlyFree(bool onlyFreeOnes)
{
    if (m_searchFreeOnes != onlyFreeOnes)
    {
        m_searchFreeOnes = onlyFreeOnes;
        emit searchOnlyFreeChanged();
    }
}

bool FilterConnectorSortModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (m_searchText.isEmpty())
    {
        return true;
    }
    const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.isValid())
    {
        return false;
    }
    const auto connectorModel = sourceIndex.data(Qt::UserRole + 1);
    if (connectorModel.canConvert<FilterConnector*>())
    {
        const auto connector = connectorModel.value<FilterConnector*>();
        //Check if type is In (1) or Out (2) or InOut (3)
        if (m_searchText == "_In")
        {
            if (static_cast<int>(connector->getType()) == 1)    //1 = In
            {
                if (m_searchFreeOnes)
                {
                     if (connector->getInEdgeItems().size() == 0)
                     {
                         qDebug() << connector->getInEdgeItems().size();
                         return true;
                     }
                     else
                     {
                         return false;
                     }
                }
                else
                {
                    return true;
                }
            }
        }
        else
        {
            if (static_cast<int>(connector->getType()) == 2)      //2 = Out
            {
                return true;
            }
        }
    }

    return false;
}

