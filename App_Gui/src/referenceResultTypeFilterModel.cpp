#include "referenceResultTypeFilterModel.h"
#include "referenceCurve.h"

using precitec::storage::ReferenceCurve;

namespace precitec
{
namespace gui
{

ReferenceResultTypeFilterModel::ReferenceResultTypeFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &ReferenceResultTypeFilterModel::resultTypeChanged, this, &ReferenceResultTypeFilterModel::invalidate);
}

ReferenceResultTypeFilterModel::~ReferenceResultTypeFilterModel() = default;

bool ReferenceResultTypeFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto curve = sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole).value<ReferenceCurve*>();

    if (!curve)
    {
        return false;
    }

    return curve->resultType() == m_resultType;
}

void ReferenceResultTypeFilterModel::setResultType(int type)
{
    if(m_resultType == type)
    {
        return;
    }
    m_resultType = type;
    emit resultTypeChanged();
}

}
}
