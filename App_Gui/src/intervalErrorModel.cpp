#include "intervalErrorModel.h"
#include "seam.h"
#include "intervalError.h"
#include "qualityNorm.h"
#include "qualityNormResult.h"

using precitec::storage::IntervalError;
using precitec::storage::QualityNorm;

namespace precitec
{
namespace gui
{

IntervalErrorModel::IntervalErrorModel(QObject *parent)
    : SimpleErrorModel(parent)
{
    connect(this, &IntervalErrorModel::qualityNormChanged, this, &IntervalErrorModel::updateQualityNormResult);
}

IntervalErrorModel::~IntervalErrorModel() = default;

QHash<int, QByteArray> IntervalErrorModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("error")},
        {Qt::UserRole + 1, QByteArrayLiteral("type")},
        {Qt::UserRole + 2, QByteArrayLiteral("qualityNormResult")}
    };
}

QVariant IntervalErrorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !currentSeam())
    {
        return {};
    }
    const auto &errors = currentSeam()->intervalErrors();
    if (index.row() > int(errors.size()))
    {
        return {};
    }
    auto error = errors.at(index.row());
    if (role == Qt::DisplayRole)
    {
         return error->name();
    }
    if (role == Qt::UserRole)
    {
         return QVariant::fromValue(error);
    }
    if (role == Qt::UserRole + 1)
    {
        return nameFromId(error->variantId());
    }
    if (role == Qt::UserRole + 2)
    {
        return QVariant::fromValue(m_qualityNorm ? m_qualityNorm->qualityNormResult(error->resultValue()) : nullptr);
    }
    return {};
}

int IntervalErrorModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !currentSeam())
    {
        return 0;
    }
    return currentSeam()->intervalErrors().size();
}

void IntervalErrorModel::setQualityNorm(QualityNorm* qualityNorm)
{
    if (m_qualityNorm == qualityNorm)
    {
        return;
    }
    m_qualityNorm = qualityNorm;
    disconnect(m_destroyQualityNormConnection);
    if (m_qualityNorm)
    {
        m_destroyQualityNormConnection = connect(m_qualityNorm, &QObject::destroyed, this, std::bind(&IntervalErrorModel::setQualityNorm, this, nullptr));
    } else
    {
        m_destroyQualityNormConnection = {};
    }
    emit qualityNormChanged();
}

IntervalError *IntervalErrorModel::createError(ErrorType errorType)
{
    if (!currentSeam())
    {
        return nullptr;
    }
    beginInsertRows({}, rowCount(), rowCount());
    auto error = addIntervalError(errorType);
    connect(error, &IntervalError::resultValueChanged, this, &IntervalErrorModel::updateQualityNormResult);
    endInsertRows();

    return error;
}

void IntervalErrorModel::removeError(IntervalError *error)
{
    if (!error)
    {
        return;
    }
    if (!currentSeam())
    {
        return;
    }
    const auto &errors = currentSeam()->intervalErrors();
    auto it = std::find(errors.begin(), errors.end(), error);
    if (it == errors.end())
    {
        return;
    }
    auto index = std::distance(errors.begin(), it);
    disconnect(error, &IntervalError::resultValueChanged, this, &IntervalErrorModel::updateQualityNormResult);
    beginRemoveRows({}, index, index);
    currentSeam()->removeIntervalError(index);
    endRemoveRows();
}

void IntervalErrorModel::updateQualityNormResult()
{
    emit dataChanged(index(0), index(rowCount() - 1), {Qt::UserRole + 2});
}

}
}

