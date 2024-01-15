#include "errorTemplateFilterModel.h"
#include "errorSettingModel.h"
#include "resultSetting.h"
#include "event/resultType.h"

using precitec::storage::ErrorSettingModel;
using precitec::interface::ResultType;

namespace precitec
{
namespace gui
{

ErrorTemplateFilterModel::ErrorTemplateFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &ErrorTemplateFilterModel::qualityFaultCategory2Changed, this, &ErrorTemplateFilterModel::invalidate);
}

ErrorTemplateFilterModel::~ErrorTemplateFilterModel()
{
}

bool ErrorTemplateFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.isValid())
    {
        return false;
    }
    if (!m_errorSettingModel)
    {
        return true;
    }

    auto type = sourceIndex.data(Qt::DisplayRole).toInt();
    if (type >= 5000)
    {
        return true;
    }
    const auto items = m_errorSettingModel->errorItems();
    auto it = std::find_if(items.begin(), items.end(), [type] (auto p) { return p->enumType() == type; });
    if (it != items.end())
    {
        return false;
    }
    if (!m_qualityFaultCategory2 && (type < int(ResultType::QualityFaultTypeA) || type > int(ResultType::QualityFaultTypeP)) )
    {
        return false;
    }
    return true;
}

void ErrorTemplateFilterModel::setErrorSettingModel(ErrorSettingModel *model)
{
    if (m_errorSettingModel == model)
    {
        return;
    }
    m_errorSettingModel = model;
    emit errorSettingModelChanged();
}

void ErrorTemplateFilterModel::setQualityFaultCategory2(bool value)
{
    if (m_qualityFaultCategory2 == value)
    {
        return;
    }
    m_qualityFaultCategory2 = value;
    emit qualityFaultCategory2Changed();
}

}
}
