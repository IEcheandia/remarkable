#include "referenceCurvesModel.h"
#include "referenceCurve.h"
#include "seam.h"

#include <QUuid>

using precitec::storage::Seam;
using precitec::storage::ReferenceCurve;

namespace precitec
{
namespace gui
{

ReferenceCurvesModel::ReferenceCurvesModel(QObject* parent)
    : QAbstractListModel(parent)
{
    connect(this, &ReferenceCurvesModel::rowsInserted, this, &ReferenceCurvesModel::markAsChanged);
    connect(this, &ReferenceCurvesModel::rowsRemoved, this, &ReferenceCurvesModel::markAsChanged);
}

ReferenceCurvesModel::~ReferenceCurvesModel() = default;

QVariant ReferenceCurvesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_currentSeam)
    {
        return {};
    }

    const auto& referenceCurves = m_currentSeam->referenceCurves();
    if (index.row() > int(referenceCurves.size()))
    {
        return {};
    }

    auto curve = referenceCurves.at(index.row());
    if (role == Qt::DisplayRole)
    {
         return curve->name();
    }
    if (role == Qt::UserRole)
    {
         return QVariant::fromValue(curve);
    }
    if (role == Qt::UserRole + 1)
    {
        return curve->uuid();
    }
    return {};
}

int ReferenceCurvesModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_currentSeam)
    {
        return 0;
    }
    return m_currentSeam->referenceCurves().size();
}

QHash<int, QByteArray> ReferenceCurvesModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("curve")},
        {Qt::UserRole + 1, QByteArrayLiteral("id")}
    };
}

void ReferenceCurvesModel::setCurrentSeam(Seam* seam)
{
    if (m_currentSeam == seam)
    {
        return;
    }

    disconnect(m_seamDestroyConnection);

    beginResetModel();

    m_currentSeam = seam;

    if (m_currentSeam)
    {
        m_seamDestroyConnection = connect(m_currentSeam, &QObject::destroyed, this, std::bind(&ReferenceCurvesModel::setCurrentSeam, this, nullptr));
    } else
    {
        m_seamDestroyConnection = {};
    }

    endResetModel();

    emit currentSeamChanged();
}

ReferenceCurve* ReferenceCurvesModel::createReferenceCurve(int type)
{
    if (!m_currentSeam)
    {
        return nullptr;
    }

    beginInsertRows({}, rowCount(), rowCount());
    auto curve = m_currentSeam->createReferenceCurve(type);
    endInsertRows();

    return curve;
}

ReferenceCurve* ReferenceCurvesModel::copyReferenceCurve(ReferenceCurve* curve)
{
    if (!m_currentSeam || !curve)
    {
        return nullptr;
    }

    beginInsertRows({}, rowCount(), rowCount());
    auto newCurve = m_currentSeam->copyReferenceCurve(curve);
    endInsertRows();

    return newCurve;
}

void ReferenceCurvesModel::removeReferenceCurve(ReferenceCurve* curve)
{
    if (!curve || !m_currentSeam)
    {
        return;
    }

    const auto& referenceCurves = m_currentSeam->referenceCurves();
    const auto it = std::find(referenceCurves.begin(), referenceCurves.end(), curve);
    if (it == referenceCurves.end())
    {
        return;
    }

    const auto index = std::distance(referenceCurves.begin(), it);

    beginRemoveRows({}, index, index);
    m_currentSeam->removeReferenceCurve(curve);
    endRemoveRows();
}

}
}

