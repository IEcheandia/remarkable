#include "resultTemplateFilterModel.h"
#include "resultSettingModel.h"
#include "resultSetting.h"
#include "event/resultType.h"

using precitec::storage::ResultSettingModel;
using precitec::interface::ResultType;

namespace precitec
{
    namespace gui
    {

        ResultTemplateFilterModel::ResultTemplateFilterModel(QObject* parent)
        : QSortFilterProxyModel(parent)
        {
            connect(this, &ResultTemplateFilterModel::qualityFaultCategory2Changed, this, &ResultTemplateFilterModel::invalidate);
        }

        ResultTemplateFilterModel::~ResultTemplateFilterModel()
        {
        }

        bool ResultTemplateFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
        {
            const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
            if (!sourceIndex.isValid())
            {
                return false;
            }
            if (!m_resultSettingModel)
            {
                return true;
            }

            auto type = sourceIndex.data(Qt::DisplayRole).toInt();
            if (type >= 5000)
            {
                return true;
            }
            const auto items = m_resultSettingModel->resultItems();
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

        void ResultTemplateFilterModel::setResultSettingModel(ResultSettingModel *model)
        {
            if (m_resultSettingModel == model)
            {
                return;
            }
            m_resultSettingModel = model;
            emit resultSettingModelChanged();
        }

        void ResultTemplateFilterModel::setQualityFaultCategory2(bool value)
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
