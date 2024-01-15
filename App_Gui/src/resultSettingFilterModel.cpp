#include "resultSettingFilterModel.h"
#include "errorSettingModel.h"

using precitec::storage::ResultSetting;
using precitec::storage::ErrorSettingModel;

namespace precitec
{
namespace gui
{

ResultSettingFilterModel::ResultSettingFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    m_sortOnType = ResultSetting::Type::Name;
    m_sortAsc = true;
    connect(this, &ResultSettingFilterModel::sortOnTypeChanged, this, &ResultSettingFilterModel::sortingChanged);
    connect(this, &ResultSettingFilterModel::sortAscChanged, this, &ResultSettingFilterModel::invalidate);
    setSortRole(Qt::UserRole + 1);
    sort(0);
    connect(this, &ResultSettingFilterModel::searchTextChanged, this, &ResultSettingFilterModel::invalidate);
}

ResultSettingFilterModel::~ResultSettingFilterModel() = default;

bool ResultSettingFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
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
    if (sourceIndex.data(Qt::UserRole + 1).toString().contains(m_searchText, Qt::CaseInsensitive))
    {
        return true;
    }
    if (sourceIndex.data(Qt::DisplayRole).toString().contains(m_searchText, Qt::CaseInsensitive))
    {
        return true;
    }
    bool ok = false;
    const auto number = m_searchText.toInt(&ok);
    if (!ok)
    {
        return false;
    }
    return sourceIndex.data(Qt::DisplayRole).toInt() == number;
}

void ResultSettingFilterModel::sortingChanged()
{
    switch (m_sortOnType)
    {
        case ResultSetting::Type::Name:
            setSortRole(Qt::UserRole + 1);
            break;
        case ResultSetting::Type::EnumType:
            setSortRole(Qt::DisplayRole);
            break;
        case ResultSetting::Type::Visible:
            setSortRole(Qt::UserRole + 7);
            break;
        default:
            break;
    }
    invalidate();
}

bool ResultSettingFilterModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    QVariant leftData = sourceModel()->data(source_left, sortRole());
    QVariant rightData = sourceModel()->data(source_right, sortRole());
    switch (sortRole())
    {
        case Qt::DisplayRole:
             return m_sortAsc ? leftData.toInt() <= rightData.toInt() : leftData.toInt() >= rightData.toInt();
        case Qt::UserRole + 1:
             return m_sortAsc ? leftData.toString() <= rightData.toString() : leftData.toString() >= rightData.toString();
        case Qt::UserRole + 7:
             return m_sortAsc ? leftData.toInt() <= rightData.toInt() : leftData.toInt() >= rightData.toInt();
        default:
             return m_sortAsc ? leftData.toInt() <= rightData.toInt() : leftData.toInt() >= rightData.toInt();
             break;
    }
}

void ResultSettingFilterModel::setSearchText(const QString &searchText)
{
    if (m_searchText == searchText)
    {
        return;
    }
    m_searchText = searchText;
    emit searchTextChanged();
}

int ResultSettingFilterModel::findIndex(int value)
{
    for (int i = 0; i < sourceModel()->rowCount(); i++)
    {
        if (index(i, 0).data().toInt() == value)
        {
            return i;
        }
    }
    return -1;
}

void ResultSettingFilterModel::updateUsedFlags()
{
    connect(qobject_cast<ErrorSettingModel*>(this->sourceModel()), &ErrorSettingModel::updateUsedFlagChanged, this, &ResultSettingFilterModel::invalidate);
}

}
}
