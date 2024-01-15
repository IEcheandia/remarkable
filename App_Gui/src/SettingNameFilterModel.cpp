#include "SettingNameFilterModel.h"

namespace precitec
{
namespace gui
{

SettingNameFilterModel::SettingNameFilterModel(QObject* parent)
: QSortFilterProxyModel(parent)
{
    connect(this, &SettingNameFilterModel::searchTextChanged, this, &SettingNameFilterModel::invalidate);
}

SettingNameFilterModel::~SettingNameFilterModel() = default;

void SettingNameFilterModel::setSearchText(const QString& text)
{
    if (m_searchText != text)
    {
        m_searchText = text;
        emit searchTextChanged();
    }
}

QString SettingNameFilterModel::searchText() const
{
    return m_searchText;
}

bool SettingNameFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (m_searchText.isNull())
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

}
}
