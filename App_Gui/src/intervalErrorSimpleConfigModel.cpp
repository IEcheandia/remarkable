#include "intervalErrorSimpleConfigModel.h"
#include "seam.h"
#include "intervalError.h"
#include "qualityNormResult.h"
#include "qualityNormLevel.h"
#include "gaugeRange.h"
#include "seam.h"

#include <QColor>

using precitec::storage::Seam;
using precitec::storage::IntervalError;
using precitec::storage::QualityNormResult;
using precitec::storage::AbstractMeasureTask;

namespace precitec
{
namespace gui
{

IntervalErrorSimpleConfigModel::IntervalErrorSimpleConfigModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

IntervalErrorSimpleConfigModel::~IntervalErrorSimpleConfigModel() = default;

QHash<int, QByteArray> IntervalErrorSimpleConfigModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("color")},
        {Qt::UserRole, QByteArrayLiteral("min")},
        {Qt::UserRole + 1, QByteArrayLiteral("max")},
        {Qt::UserRole + 2, QByteArrayLiteral("threshold")},
        {Qt::UserRole + 3, QByteArrayLiteral("qnMin")},
        {Qt::UserRole + 4, QByteArrayLiteral("qnMax")},
        {Qt::UserRole + 5, QByteArrayLiteral("qnThreshold")},
        {Qt::UserRole + 6, QByteArrayLiteral("secondThreshold")},
        {Qt::UserRole + 7, QByteArrayLiteral("qnSecondThreshold")}
    };
}

QVariant IntervalErrorSimpleConfigModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= int(AbstractMeasureTask::maxLevel()))
    {
        return {};
    }
    if (role == Qt::DisplayRole)
    {
        return AbstractMeasureTask::levelColor(index.row());
    }
    if (!m_intervalError)
    {
        return {};
    }
    if (role == Qt::UserRole)
    {
        return m_intervalError->min(index.row());
    }
    if (role == Qt::UserRole + 1)
    {
        return m_intervalError->max(index.row());
    }
    if (role == Qt::UserRole + 2)
    {
        return m_intervalError->threshold(index.row());
    }
    if (role == Qt::UserRole + 3)
    {
        return qualityNormMin(index.row());
    }
    if (role == Qt::UserRole + 4)
    {
        return qualityNormMax(index.row());
    }
    if (role == Qt::UserRole + 5)
    {
        return qualityNormThreshold(index.row());
    }
    if (role == Qt::UserRole + 6)
    {
        return m_intervalError->secondThreshold(index.row());
    }
    if (role == Qt::UserRole + 7)
    {
        return qualityNormSecondThreshold(index.row());
    }
    return {};
}

bool IntervalErrorSimpleConfigModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || !m_intervalError || index.row() >= int(AbstractMeasureTask::maxLevel()))
    {
        return false;
    }
    if (role == Qt::UserRole)
    {
        m_intervalError->setMin(index.row(), value.toDouble());
        emit dataChanged(index, index, {Qt::UserRole});
        return true;
    }
    if (role == Qt::UserRole + 1)
    {
        m_intervalError->setMax(index.row(), value.toDouble());
        emit dataChanged(index, index, {Qt::UserRole + 1});
        return true;
    }
    if (role == Qt::UserRole + 2)
    {
        m_intervalError->setThreshold(index.row(), value.toDouble());
        emit dataChanged(index, index, {Qt::UserRole + 2});
        return true;
    }
    return false;
}

int IntervalErrorSimpleConfigModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return AbstractMeasureTask::maxLevel();
}

Qt::ItemFlags IntervalErrorSimpleConfigModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void IntervalErrorSimpleConfigModel::setIntervalError(IntervalError *intervalError)
{
    if (m_intervalError == intervalError)
    {
        return;
    }
    if (m_intervalError)
    {
        disconnect(m_intervalError, &IntervalError::resultValueChanged, this, &IntervalErrorSimpleConfigModel::updateQualityNormValues);
        disconnect(m_intervalErrorDestroyedConnection);
    }
    beginResetModel();
    m_intervalError = intervalError;
    if (m_intervalError)
    {
        m_intervalErrorDestroyedConnection = connect(m_intervalError, &QObject::destroyed, this, std::bind(&IntervalErrorSimpleConfigModel::setIntervalError, this, nullptr));
        connect(m_intervalError, &IntervalError::resultValueChanged, this, &IntervalErrorSimpleConfigModel::updateQualityNormValues);
    } else
    {
        m_intervalErrorDestroyedConnection = {};
    }
    endResetModel();
    emit intervalErrorChanged();
}

void IntervalErrorSimpleConfigModel::setQualityNormResult(QualityNormResult* qualityNormResult)
{
    if (m_qualityNormResult == qualityNormResult)
    {
        return;
    }
    m_qualityNormResult = qualityNormResult;
    disconnect(m_qualityNormDestroyedConnection);
    if (m_qualityNormResult)
    {
        m_qualityNormDestroyedConnection = connect(m_qualityNormResult, &QObject::destroyed, this, std::bind(&IntervalErrorSimpleConfigModel::setQualityNormResult, this, nullptr));
    } else
    {
        m_qualityNormDestroyedConnection = {};
    }
    emit qualityNormResultChanged();
}

void IntervalErrorSimpleConfigModel::setCurrentSeam(Seam *seam)
{
    if (m_currentSeam == seam)
    {
        return;
    }
    if (m_currentSeam)
    {
        disconnect(m_currentSeam, &Seam::thicknessLeftChanged, this, &IntervalErrorSimpleConfigModel::updateQualityNormValues);
        disconnect(m_currentSeam, &Seam::thicknessRightChanged, this, &IntervalErrorSimpleConfigModel::updateQualityNormValues);
        disconnect(m_seamDestroyedConnection);
    }
    m_currentSeam = seam;
    if (m_currentSeam)
    {
        m_seamDestroyedConnection = connect(m_currentSeam, &QObject::destroyed, this, std::bind(&IntervalErrorSimpleConfigModel::setCurrentSeam, this, nullptr));
        connect(m_currentSeam, &Seam::thicknessLeftChanged, this, &IntervalErrorSimpleConfigModel::updateQualityNormValues);
        connect(m_currentSeam, &Seam::thicknessRightChanged, this, &IntervalErrorSimpleConfigModel::updateQualityNormValues);
    } else
    {
        m_seamDestroyedConnection = {};
    }
    emit currentSeamChanged();
}

qreal IntervalErrorSimpleConfigModel::qualityNormMin(int level) const
{
    if (!m_currentSeam || !m_qualityNormResult)
    {
        return 0.0;
    }

    const auto gauge = std::min(m_currentSeam->thicknessRight(), m_currentSeam->thicknessLeft()) / 1000.0;

    if (auto qualityLevel = m_qualityNormResult->levelAt(level))
    {
        if (auto range = qualityLevel->range(gauge))
        {
            return range->minFactor() * gauge + range->minOffset();
        }
    }

    return 0.0;
}

qreal IntervalErrorSimpleConfigModel::qualityNormMax(int level) const
{
    if (!m_currentSeam || !m_qualityNormResult)
    {
        return 0.0;
    }

    const auto gauge = std::min(m_currentSeam->thicknessRight(), m_currentSeam->thicknessLeft()) / 1000.0;

    if (auto qualityLevel = m_qualityNormResult->levelAt(level))
    {
        if (auto range = qualityLevel->range(gauge))
        {
            return range->maxFactor() * gauge + range->maxOffset();
        }
    }

    return 0.0;
}

qreal IntervalErrorSimpleConfigModel::qualityNormThreshold(int level) const
{
    if (!m_currentSeam || !m_qualityNormResult)
    {
        return 0.0;
    }

    const auto gauge = std::min(m_currentSeam->thicknessRight(), m_currentSeam->thicknessLeft()) / 1000.0;

    if (auto qualityLevel = m_qualityNormResult->levelAt(level))
    {
        if (auto range = qualityLevel->range(gauge))
        {
            return range->length();
        }
    }

    return 0.0;
}

qreal IntervalErrorSimpleConfigModel::qualityNormSecondThreshold(int level) const
{
    if (!m_currentSeam || !m_qualityNormResult)
    {
        return 0.0;
    }

    const auto gauge = std::min(m_currentSeam->thicknessRight(), m_currentSeam->thicknessLeft()) / 1000.0;

    if (auto qualityLevel = m_qualityNormResult->levelAt(level))
    {
        if (auto range = qualityLevel->range(gauge))
        {
            return range->secondThreshold();
        }
    }

    return 0.0;
}

void IntervalErrorSimpleConfigModel::updateQualityNormValues()
{
    emit dataChanged(index(0), index(rowCount() - 1), {Qt::UserRole + 3, Qt::UserRole + 4, Qt::UserRole + 5, Qt::UserRole + 7});
}

void IntervalErrorSimpleConfigModel::setValueFromQualityNorms(int level)
{
    if (m_qualityNormResult)
    {
        m_intervalError->setMin(level, qualityNormMin(level));
        m_intervalError->setMax(level, qualityNormMax(level));
        m_intervalError->setThreshold(level, qualityNormThreshold(level));
        m_intervalError->setSecondThreshold(level, qualityNormSecondThreshold(level));
        emit dataChanged(index(0), index(rowCount() - 1), {Qt::UserRole, Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 6});
    }
}

}
}


