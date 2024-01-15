#include "lineLaserFilterModel.h"

namespace precitec
{
namespace gui
{

LineLaserFilterModel::LineLaserFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &LineLaserFilterModel::lineLaser1AvailableChanged, this, &LineLaserFilterModel::invalidate);
    connect(this, &LineLaserFilterModel::lineLaser2AvailableChanged, this, &LineLaserFilterModel::invalidate);
    connect(this, &LineLaserFilterModel::lineLaser3AvailableChanged, this, &LineLaserFilterModel::invalidate);
}

LineLaserFilterModel::~LineLaserFilterModel() = default;

bool LineLaserFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)

    switch (source_row)
    {
        case 0:
            return lineLaser1Available();
        case 1:
            return lineLaser2Available();
        case 2:
            return lineLaser3Available();
        default:
            return false;
    }
}

void LineLaserFilterModel::setLineLaser1Available(bool set)
{
    if (m_lineLaser1Available == set)
    {
        return;
    }
    m_lineLaser1Available = set;
    emit lineLaser1AvailableChanged();
}

void LineLaserFilterModel::setLineLaser2Available(bool set)
{
    if (m_lineLaser2Available == set)
    {
        return;
    }
    m_lineLaser2Available = set;
    emit lineLaser2AvailableChanged();
}

void LineLaserFilterModel::setLineLaser3Available(bool set)
{
    if (m_lineLaser3Available == set)
    {
        return;
    }
    m_lineLaser3Available = set;
    emit lineLaser3AvailableChanged();
}

}
}


