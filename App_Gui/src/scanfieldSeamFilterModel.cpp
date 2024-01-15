#include "scanfieldSeamFilterModel.h"
#include "scanfieldSeamModel.h"

namespace precitec
{
namespace gui
{

ScanfieldSeamFilterModel::ScanfieldSeamFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &ScanfieldSeamFilterModel::sourceModelChanged, this, &ScanfieldSeamFilterModel::invalidate);
}

ScanfieldSeamFilterModel::~ScanfieldSeamFilterModel() = default;

bool ScanfieldSeamFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (!sourceModel())
    {
        return false;
    }

    // UserRole + 1 is cameraCenterValid role of ScanfieldSeamModel
    return sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 1).toBool();
}

void ScanfieldSeamFilterModel::setScanfieldSeamModel(ScanfieldSeamModel* scanfieldSeamModel)
{
    if (m_scanfieldSeamModel == scanfieldSeamModel)
    {
        return;
    }

    if (m_scanfieldSeamModel)
    {
        disconnect(m_scanfieldSeamModelDestroyed);
        disconnect(m_scanfieldSeamModelChanged);
        disconnect(m_scanfieldSeamModel, &ScanfieldSeamModel::modelReset, this, &ScanfieldSeamFilterModel::invalidate);
    }

    m_scanfieldSeamModel = scanfieldSeamModel;
    setSourceModel(m_scanfieldSeamModel);

    if (m_scanfieldSeamModel)
    {
        m_scanfieldSeamModelDestroyed = connect(m_scanfieldSeamModel, &QObject::destroyed, this, std::bind(&ScanfieldSeamFilterModel::setScanfieldSeamModel, this, nullptr));
        m_scanfieldSeamModelChanged = connect(m_scanfieldSeamModel, &ScanfieldSeamModel::dataChanged, this, [this] (const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
        {
            Q_UNUSED(topLeft)
            Q_UNUSED(bottomRight)

            // cameraCenterValid role (Qt::UserRole + 1)
            if (roles.empty() || std::any_of(roles.begin(), roles.end(), [] (const int entry) { return entry == Qt::UserRole + 1; }))
            {
                invalidate();
            }
        });
        connect(m_scanfieldSeamModel, &ScanfieldSeamModel::modelReset, this, &ScanfieldSeamFilterModel::invalidate);
    } else
    {
        m_scanfieldSeamModelDestroyed = {};
        m_scanfieldSeamModelChanged = {};
    }

    emit scanfieldSeamModelChanged();
}

}
}


