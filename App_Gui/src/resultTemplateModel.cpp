#include "resultTemplateModel.h"
#include "resultTemplate.h"
#include "resultHelper.h"
#include "event/resultType.h"

using precitec::storage::ResultTemplate;

namespace precitec
{
    namespace gui
    {

        ResultTemplateModel::ResultTemplateModel(QObject *parent)
        : QAbstractListModel(parent)
        {
            createResultEntries();
        }

        ResultTemplateModel::~ResultTemplateModel()
        {
        }

        int ResultTemplateModel::rowCount(const QModelIndex &parent) const
        {
            if (parent.isValid())
            {
                return 0;
            }
            return m_resultItems.size();
        }

        QVariant ResultTemplateModel::data(const QModelIndex &index, int role) const
        {
            const auto row = index.row();
            if(row < 0 || row >= (int)m_resultItems.size())
            {
                return {};
            }

            const auto &resultItem = m_resultItems.at(index.row());
            switch(role)
            {
                case Qt::DisplayRole:
                    return resultItem->enumType();
                case Qt::UserRole:
                    return resultItem->name();
                default:
                    return {};
            }
            return {};
        }


        QHash<int, QByteArray> ResultTemplateModel::roleNames() const
        {
            return QHash<int, QByteArray>{
                {Qt::DisplayRole, QByteArrayLiteral("enumType")},
                {Qt::UserRole, QByteArrayLiteral("name")},
            };
        }

        void ResultTemplateModel::createResultEntries()
        {
            beginResetModel();

            m_resultItems.clear();
            m_resultItems.emplace_back(new ResultTemplate(5000, QStringLiteral("User Defined"), this));

            for (int enumType = interface::ResultType::QualityFaultTypeA; enumType <= interface::ResultType::QualityFaultTypeX; enumType++)
            {
                m_resultItems.emplace_back(new ResultTemplate(enumType, nameForQualityError(enumType), this));
            }

            for (int enumType = interface::ResultType::QualityFaultTypeA_Cat2; enumType <= interface::ResultType::QualityFaultTypeX_Cat2; enumType++)
            {
                m_resultItems.emplace_back(new ResultTemplate(enumType, nameForQualityError(enumType), this));
            }

            m_resultItems.emplace_back(new ResultTemplate(interface::ResultType::FastStop_DoubleBlank, nameForQualityError(interface::ResultType::FastStop_DoubleBlank), this));

            endResetModel();
        }

    }
}

