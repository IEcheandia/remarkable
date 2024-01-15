#include "subGraphCategoryFilterModel.h"
#include "subGraphModel.h"

namespace precitec
{
namespace storage
{

SubGraphCategoryFilterModel::SubGraphCategoryFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &SubGraphCategoryFilterModel::categoryNameChanged, this, &SubGraphCategoryFilterModel::invalidate);
    connect(this, &SubGraphCategoryFilterModel::disabledFilterChanged, this, &SubGraphCategoryFilterModel::invalidate);
    sort(0);
    connect(this, &SubGraphCategoryFilterModel::searchTextChanged, this, &SubGraphCategoryFilterModel::invalidate);
    sort(0);
}

SubGraphCategoryFilterModel::~SubGraphCategoryFilterModel() = default;

void SubGraphCategoryFilterModel::setCategoryName(const QString &name)
{
    if (m_categoryName == name)
    {
        return;
    }
    m_categoryName = name;
    emit categoryNameChanged();
}

void SubGraphCategoryFilterModel::setDisabledFilter(bool disabledFilter)
{
    m_disabledFilter = disabledFilter;
    emit disabledFilterChanged();
}

QString SubGraphCategoryFilterModel::searchText() const
{
    return m_searchText;
}

void SubGraphCategoryFilterModel::setSearchText(const QString& text)
{
    if (m_searchText != text)
    {
        m_searchText = text;
        emit searchTextChanged();
    }
}

bool SubGraphCategoryFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (auto subGraphModel = qobject_cast<SubGraphModel*>(sourceModel()))
    {
        const auto index = subGraphModel->index(source_row, 0, source_parent);

        if (index.isValid())
        {
            if (subGraphModel->category(index).compare(m_categoryName, Qt::CaseInsensitive) == 0)
            {
                if(subGraphModel->isGraphEnabled(index) || m_disabledFilter)
                {
                    QString name = subGraphModel->data(index,Qt::DisplayRole).toString();
                    if(name.contains(m_searchText,Qt::CaseInsensitive)){
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

}
}
