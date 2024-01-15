#include "ResultSettingFilterModel.h"
#include "errorSettingModel.h"
#include "resultSettingModel.h"

using precitec::storage::ResultSetting;
using precitec::storage::ErrorSettingModel;

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{

ResultSettingFilterModel::ResultSettingFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    m_sortOnType = ResultSetting::Type::Name;
    m_sortAsc = true;
    connect(this, &ResultSettingFilterModel::sortOnTypeChanged, this, &ResultSettingFilterModel::sortingChanged);
    connect(this, &ResultSettingFilterModel::sortAscChanged, this, &ResultSettingFilterModel::invalidate);
    connect(this, &ResultSettingFilterModel::resultModelChanged, this, &ResultSettingFilterModel::invalidate);
    setSortRole(Qt::UserRole + 1);
    sort(0);
}

ResultSettingFilterModel::~ResultSettingFilterModel() = default;

precitec::storage::ResultSettingModel* ResultSettingFilterModel::resultModel() const
{
    return m_resultModel;
}

void ResultSettingFilterModel::setResultModel(precitec::storage::ResultSettingModel* newModel)
{
    if (m_resultModel == newModel)
    {
        return;
    }
    m_resultModel = newModel;
    emit resultModelChanged();
}

bool ResultSettingFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    return true;
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

QModelIndex ResultSettingFilterModel::indexForResultType(int enumType) const
{
    if (!m_resultModel)
    {
        return {};
    }
    return mapFromSource(m_resultModel->indexForResultType(enumType));
}

void ResultSettingFilterModel::ensureItemExists(int enumType)
{
    if (!m_resultModel)
    {
        return;
    }
    m_resultModel->ensureItemExists(enumType);
}

}
}
}
}

