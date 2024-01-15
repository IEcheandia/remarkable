#include "intervalErrorConfigModel.h"
#include "seam.h"
#include "intervalError.h"
#include "qualityNorm.h"
#include "qualityNormResult.h"
#include "qualityNormLevel.h"
#include "gaugeRange.h"
#include "precitec/dataSet.h"
#include "precitec/infiniteSet.h"

#include <QColor>

using precitec::storage::Seam;
using precitec::storage::IntervalError;
using precitec::storage::QualityNorm;
using precitec::storage::QualityNormResult;
using precitec::storage::QualityNormLevel;
using precitec::storage::GaugeRange;
using precitec::storage::AbstractMeasureTask;
using precitec::gui::components::plotter::DataSet;
using precitec::gui::components::plotter::InfiniteSet;

namespace precitec
{
namespace gui
{

IntervalErrorConfigModel::IntervalErrorConfigModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_visualReference(new DataSet(this))
{
    m_visualReference->setDrawingOrder(DataSet::DrawingOrder::OnTop);

    for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
    {
        const auto color = AbstractMeasureTask::levelColor(i);
        auto lower = new InfiniteSet(this);
        lower->setName(QStringLiteral("Level %1 Min").arg(i + 1));
        lower->setColor(color);
        m_lowerBoundaries.push_back(lower);

        auto upper = new InfiniteSet(this);
        upper->setName(QStringLiteral("Level %1 Max").arg(i + 1));
        upper->setColor(color);
        m_upperBoundaries.push_back(upper);

        auto shiftedLower = new InfiniteSet(this);
        shiftedLower->setName(QStringLiteral("Level %1 Min Shift").arg(i + 1));
        shiftedLower->setColor(color);
        m_shiftedLowerBoundaries.push_back(shiftedLower);

        auto shiftedUpper = new InfiniteSet(this);
        shiftedUpper->setName(QStringLiteral("Level %1 Max Shift").arg(i + 1));
        shiftedUpper->setColor(color);
        m_shiftedUpperBoundaries.push_back(shiftedUpper);
    }

    connect(this, &IntervalErrorConfigModel::intervalErrorChanged, this, &IntervalErrorConfigModel::updateLowerBoundary);
    connect(this, &IntervalErrorConfigModel::intervalErrorChanged, this, &IntervalErrorConfigModel::updateUpperBoundary);
    connect(this, &IntervalErrorConfigModel::intervalErrorChanged, this, &IntervalErrorConfigModel::updateQualityNormResult);
    connect(this, &IntervalErrorConfigModel::qualityNormChanged, this, &IntervalErrorConfigModel::updateQualityNormResult);
    connect(this, &IntervalErrorConfigModel::qualityNormResultChanged, this, &IntervalErrorConfigModel::updateQualityNormValues);
}

IntervalErrorConfigModel::~IntervalErrorConfigModel() = default;

QHash<int, QByteArray> IntervalErrorConfigModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("color")},
        {Qt::UserRole, QByteArrayLiteral("min")},
        {Qt::UserRole + 1, QByteArrayLiteral("max")},
        {Qt::UserRole + 2, QByteArrayLiteral("threshold")},
        {Qt::UserRole + 3, QByteArrayLiteral("lower")},
        {Qt::UserRole + 4, QByteArrayLiteral("upper")},
        {Qt::UserRole + 5, QByteArrayLiteral("shiftedLower")},
        {Qt::UserRole + 6, QByteArrayLiteral("shiftedUpper")},
        {Qt::UserRole + 7, QByteArrayLiteral("qnMin")},
        {Qt::UserRole + 8, QByteArrayLiteral("qnMax")},
        {Qt::UserRole + 9, QByteArrayLiteral("qnThreshold")},
        {Qt::UserRole + 10, QByteArrayLiteral("secondThreshold")},
        {Qt::UserRole + 11, QByteArrayLiteral("qnSecondThreshold")}
    };
}

QVariant IntervalErrorConfigModel::data(const QModelIndex& index, int role) const
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
        return QVariant::fromValue(m_lowerBoundaries.at(index.row()));
    }
    if (role == Qt::UserRole + 4)
    {
        return QVariant::fromValue(m_upperBoundaries.at(index.row()));
    }
    if (role == Qt::UserRole + 5)
    {
        return QVariant::fromValue(m_shiftedLowerBoundaries.at(index.row()));
    }
    if (role == Qt::UserRole + 6)
    {
        return QVariant::fromValue(m_shiftedUpperBoundaries.at(index.row()));
    }
    if (role == Qt::UserRole + 7)
    {
        return qualityNormMin(index.row());
    }
    if (role == Qt::UserRole + 8)
    {
        return qualityNormMax(index.row());
    }
    if (role == Qt::UserRole + 9)
    {
        return qualityNormThreshold(index.row());
    }
    if (role == Qt::UserRole + 10)
    {
        return m_intervalError->secondThreshold(index.row());
    }
    if (role == Qt::UserRole + 11)
    {
        return qualityNormSecondThreshold(index.row());
    }
    return {};
}

bool IntervalErrorConfigModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || !m_intervalError || index.row() >= int(AbstractMeasureTask::maxLevel()))
    {
        return false;
    }
    if (role == Qt::UserRole)
    {
        m_intervalError->setMin(index.row(), value.toDouble());
        return true;
    }
    if (role == Qt::UserRole + 1)
    {
        m_intervalError->setMax(index.row(), value.toDouble());
        return true;
    }
    if (role == Qt::UserRole + 2)
    {
        m_intervalError->setThreshold(index.row(), value.toDouble());
        return true;
    }
    if (role == Qt::UserRole + 10)
    {
        m_intervalError->setSecondThreshold(index.row(), value.toDouble());
        return true;
    }
    return false;
}

int IntervalErrorConfigModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return AbstractMeasureTask::maxLevel();
}

Qt::ItemFlags IntervalErrorConfigModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void IntervalErrorConfigModel::setIntervalError(IntervalError *intervalError)
{
    if (m_intervalError == intervalError)
    {
        return;
    }
    beginResetModel();
    if (m_intervalError)
    {
        disconnect(m_intervalError, &IntervalError::minChanged, this, &IntervalErrorConfigModel::updateLowerBoundary);
        disconnect(m_intervalError, &IntervalError::maxChanged, this, &IntervalErrorConfigModel::updateUpperBoundary);
        disconnect(m_intervalError, &IntervalError::shiftChanged, this, &IntervalErrorConfigModel::updateLowerBoundary);
        disconnect(m_intervalError, &IntervalError::shiftChanged, this, &IntervalErrorConfigModel::updateUpperBoundary);
        disconnect(m_intervalError, &IntervalError::resultValueChanged, this, &IntervalErrorConfigModel::updateQualityNormResult);
        disconnect(m_destroyedConnection);
        disconnect(m_resultValueChangedConnection);
        disconnect(m_minChangedConnection);
        disconnect(m_maxChangedConnection);
        disconnect(m_thresholdChangedConnection);
        disconnect(m_secondThresholdChangedConnection);
    }
    m_intervalError = intervalError;
    if (m_intervalError)
    {
        connect(m_intervalError, &IntervalError::minChanged, this, &IntervalErrorConfigModel::updateLowerBoundary);
        connect(m_intervalError, &IntervalError::maxChanged, this, &IntervalErrorConfigModel::updateUpperBoundary);
        connect(m_intervalError, &IntervalError::shiftChanged, this, &IntervalErrorConfigModel::updateLowerBoundary);
        connect(m_intervalError, &IntervalError::shiftChanged, this, &IntervalErrorConfigModel::updateUpperBoundary);
        connect(m_intervalError, &IntervalError::resultValueChanged, this, &IntervalErrorConfigModel::updateQualityNormResult);
        m_destroyedConnection = connect(m_intervalError, &QObject::destroyed, this, std::bind(&IntervalErrorConfigModel::setIntervalError, this, nullptr));
        m_resultValueChangedConnection = connect(m_intervalError, &IntervalError::resultValueChanged, this, std::bind(&IntervalErrorConfigModel::setVisualReference, this, nullptr));
        m_minChangedConnection = connect(m_intervalError, &IntervalError::minChanged, this, std::bind(&IntervalErrorConfigModel::updateData, this, QVector<int>{Qt::UserRole}));
        m_maxChangedConnection = connect(m_intervalError, &IntervalError::maxChanged, this, std::bind(&IntervalErrorConfigModel::updateData, this, QVector<int>{Qt::UserRole + 1}));
        m_thresholdChangedConnection = connect(m_intervalError, &IntervalError::thresholdChanged, this, std::bind(&IntervalErrorConfigModel::updateData, this, QVector<int>{Qt::UserRole + 2}));
        m_secondThresholdChangedConnection = connect(m_intervalError, &IntervalError::secondThresholdChanged, this, std::bind(&IntervalErrorConfigModel::updateData, this, QVector<int>{Qt::UserRole + 10}));

        for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
        {
            m_lowerBoundaries.at(i)->setValue(m_intervalError->min(i));
            m_upperBoundaries.at(i)->setValue(m_intervalError->max(i));
            m_shiftedLowerBoundaries.at(i)->setValue(m_intervalError->min(i) + m_intervalError->shift());
            m_shiftedUpperBoundaries.at(i)->setValue(m_intervalError->max(i) + m_intervalError->shift());
        }
    } 
    else
    {
        m_destroyedConnection = {};
        m_resultValueChangedConnection = {};
        m_minChangedConnection = {};
        m_maxChangedConnection = {};
        m_thresholdChangedConnection = {};
        m_secondThresholdChangedConnection = {};
        setVisualReference(nullptr);
    }
    endResetModel();
    emit intervalErrorChanged();
}

void IntervalErrorConfigModel::setVisualReference(DataSet *ds)
{
    m_visualReference->clear();
    m_visualReference->setColor("white");
    m_visualReference->setName(QStringLiteral(""));
    if (ds)
    {
        m_visualReference->addSamples(ds->samples());
        m_visualReference->setColor(ds->color());
        m_visualReference->setName(ds->name());
    }
    emit visualReferenceChanged();
}

void IntervalErrorConfigModel::setMinFromReference(uint level)
{
    const auto value = m_visualReference->minY();
    if (m_visualReference->isEmpty() || !m_intervalError || level >= AbstractMeasureTask::maxLevel() || qFuzzyCompare(float(m_intervalError->min(level)), value))
    {
        return;
    }
    m_intervalError->setMin(level, value);
    m_lowerBoundaries.at(level)->setValue(value);
}

void IntervalErrorConfigModel::setMaxFromReference(uint level)
{
    const auto value = m_visualReference->maxY();

    if (m_visualReference->isEmpty() || !m_intervalError || level >= AbstractMeasureTask::maxLevel() || qFuzzyCompare(float(m_intervalError->max(level)), value))
    {
        return;
    }

    m_intervalError->setMax(level, value);
    m_upperBoundaries.at(level)->setValue(value);
}

void IntervalErrorConfigModel::updateLowerBoundary()
{
    if (!m_intervalError)
    {
        for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
        {
            m_lowerBoundaries.at(i)->setEnabled(false);
            m_shiftedLowerBoundaries.at(i)->setEnabled(false);
        }
        return;
    }

    for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
    {
        const auto lower = m_lowerBoundaries.at(i);
        lower->setEnabled(true);
        lower->setValue(m_intervalError->min(i));
        m_shiftedLowerBoundaries.at(i)->setValue(m_intervalError->min(i) + m_intervalError->shift());
    }

    if (qFuzzyIsNull(m_intervalError->shift()))
    {
        for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
        {
            m_lowerBoundaries.at(i)->setAlpha(255);
            m_shiftedLowerBoundaries.at(i)->setEnabled(false);
        }
    } else
    {
        for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
        {
            m_lowerBoundaries.at(i)->setAlpha(150);
            m_shiftedLowerBoundaries.at(i)->setEnabled(true);
        }
    }
}

void IntervalErrorConfigModel::updateUpperBoundary()
{
    if (!m_intervalError)
    {
        for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
        {
            m_upperBoundaries.at(i)->setEnabled(false);
            m_shiftedUpperBoundaries.at(i)->setEnabled(false);
        }
        return;
    }

    for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
    {
        const auto upper = m_upperBoundaries.at(i);
        upper->setEnabled(true);
        upper->setValue(m_intervalError->max(i));
        m_shiftedUpperBoundaries.at(i)->setValue(m_intervalError->max(i) + m_intervalError->shift());
    }

    if (qFuzzyIsNull(m_intervalError->shift()))
    {
        for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
        {
            m_upperBoundaries.at(i)->setAlpha(255);
            m_shiftedUpperBoundaries.at(i)->setEnabled(false);
        }
    } else
    {
        for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
        {
            m_upperBoundaries.at(i)->setAlpha(150);
            m_shiftedUpperBoundaries.at(i)->setEnabled(true);
        }
    }
}

void IntervalErrorConfigModel::updateData(QVector<int> &roles)
{
    emit dataChanged(index(0), index(rowCount() - 1), roles);
}

void IntervalErrorConfigModel::setQualityNorm(QualityNorm* qualityNorm)
{
    if (m_qualityNorm == qualityNorm)
    {
        return;
    }
    m_qualityNorm = qualityNorm;
    disconnect(m_destroyQualityNormConnection);
    if (m_qualityNorm)
    {
        m_destroyQualityNormConnection = connect(m_qualityNorm, &QObject::destroyed, this, std::bind(&IntervalErrorConfigModel::setQualityNorm, this, nullptr));
    } else
    {
        m_destroyQualityNormConnection = {};
    }
    emit qualityNormChanged();
}

void IntervalErrorConfigModel::updateQualityNormResult()
{
    if (!m_qualityNorm || !m_intervalError)
    {
        return;
    }

    auto qualityNormResult = m_qualityNorm->qualityNormResult(m_intervalError->resultValue());
    if (m_qualityNormResult == qualityNormResult)
    {
        return;
    }

    m_qualityNormResult = qualityNormResult;
    emit qualityNormResultChanged();
}

void IntervalErrorConfigModel::updateQualityNormValues()
{
    emit dataChanged(index(0), index(rowCount() - 1), {Qt::UserRole + 7, Qt::UserRole + 8, Qt::UserRole + 9, Qt::UserRole + 11});
}

qreal IntervalErrorConfigModel::qualityNormMin(int level) const
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

qreal IntervalErrorConfigModel::qualityNormMax(int level) const
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

qreal IntervalErrorConfigModel::qualityNormThreshold(int level) const
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

qreal IntervalErrorConfigModel::qualityNormSecondThreshold(int level) const
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

void IntervalErrorConfigModel::setCurrentSeam(Seam *seam)
{
    if (m_currentSeam == seam)
    {
        return;
    }
    if (m_currentSeam)
    {
        disconnect(m_currentSeam, &Seam::thicknessLeftChanged, this, &IntervalErrorConfigModel::updateQualityNormValues);
        disconnect(m_currentSeam, &Seam::thicknessRightChanged, this, &IntervalErrorConfigModel::updateQualityNormValues);
        disconnect(m_seamDestroyedConnection);
    }
    m_currentSeam = seam;
    if (m_currentSeam)
    {
        m_seamDestroyedConnection = connect(m_currentSeam, &QObject::destroyed, this, std::bind(&IntervalErrorConfigModel::setCurrentSeam, this, nullptr));
        connect(m_currentSeam, &Seam::thicknessLeftChanged, this, &IntervalErrorConfigModel::updateQualityNormValues);
        connect(m_currentSeam, &Seam::thicknessRightChanged, this, &IntervalErrorConfigModel::updateQualityNormValues);
    } else
    {
        m_seamDestroyedConnection = {};
    }
    emit currentSeamChanged();
}

}
}


