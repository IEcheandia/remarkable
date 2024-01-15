#include "errorTemplateModel.h"
#include "errorTemplate.h"
#include "resultHelper.h"
#include "event/resultType.h"

using precitec::storage::ErrorTemplate;

namespace precitec
{
namespace gui
{

ErrorTemplateModel::ErrorTemplateModel(QObject *parent)
    : QAbstractListModel(parent)
{
    createErrorEntries();
}

ErrorTemplateModel::~ErrorTemplateModel()
{
}

int ErrorTemplateModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_errorItems.size();
}

QVariant ErrorTemplateModel::data(const QModelIndex &index, int role) const
{
    const auto row = index.row();
    if(row < 0 || row >= (int)m_errorItems.size())
    {
        return {};
    }

    const auto &errorItem = m_errorItems.at(index.row());
    switch(role)
    {
        case Qt::DisplayRole:
            return errorItem->enumType();
        case Qt::UserRole:
            return errorItem->name();
        default:
            return {};
    }
    return {};
}


QHash<int, QByteArray> ErrorTemplateModel::roleNames() const
{
    return QHash<int, QByteArray>{
        {Qt::DisplayRole, QByteArrayLiteral("enumType")},
        {Qt::UserRole, QByteArrayLiteral("name")},
    };
}

void ErrorTemplateModel::createErrorEntries()
{
    beginResetModel();

    m_errorItems.clear();
    m_errorItems.emplace_back(new ErrorTemplate(5000, QStringLiteral("User Defined"), this));

    for (int enumType = interface::ResultType::QualityFaultTypeA; enumType <= interface::ResultType::QualityFaultTypeX; enumType++)
    {
        m_errorItems.emplace_back(new ErrorTemplate(enumType, nameForQualityError(enumType), this));
    }

    for (int enumType = interface::ResultType::QualityFaultTypeA_Cat2; enumType <= interface::ResultType::QualityFaultTypeX_Cat2; enumType++)
    {
        m_errorItems.emplace_back(new ErrorTemplate(enumType, nameForQualityError(enumType), this));
    }

    m_errorItems.emplace_back(new ErrorTemplate(interface::ResultType::FastStop_DoubleBlank, nameForQualityError(interface::ResultType::FastStop_DoubleBlank), this));

    endResetModel();
}

}
}
