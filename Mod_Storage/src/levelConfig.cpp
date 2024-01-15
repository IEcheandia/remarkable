#include "intervalError.h"
#include "levelConfig.h"

#include <QJsonObject>

namespace precitec
{
namespace storage
{
namespace
{

double parseDouble(const QJsonObject &object, const QString &key, double defaultValue = 0.0)
{
    auto it = object.find(key);
    if (it == object.end())
    {
        return defaultValue;
    }
    return it.value().toDouble();
}

}

LevelConfig::LevelConfig(QObject *parent)
    : QObject(parent)
{
}

LevelConfig::~LevelConfig() = default;

LevelConfig *LevelConfig::duplicate(IntervalError *parent) const
{
    auto level = new LevelConfig{parent};

    level->setThreshold(threshold());
    level->setSecondThreshold(secondThreshold());
    level->setMax(max());
    level->setMin(min());

    return level;
}

void LevelConfig::setThreshold(double threshold)
{
    if (qFuzzyCompare(m_threshold, threshold) || (qFuzzyIsNull(m_threshold) && qFuzzyIsNull(threshold)))
    {
        return;
    }
    m_threshold = threshold;
    emit thresholdChanged();
}

void LevelConfig::setSecondThreshold(double secondThreshold)
{
    if (qFuzzyCompare(m_secondThreshold, secondThreshold) || (qFuzzyIsNull(m_secondThreshold) && qFuzzyIsNull(secondThreshold)))
    {
        return;
    }
    m_secondThreshold = secondThreshold;
    emit secondThresholdChanged();
}

void LevelConfig::setMin(double min)
{
    if (qFuzzyCompare(m_min, min) || (qFuzzyIsNull(m_min) && qFuzzyIsNull(min)))
    {
        return;
    }
    m_min = min;
    emit minChanged();
}

void LevelConfig::setMax(double max)
{
    if (qFuzzyCompare(m_max, max) || (qFuzzyIsNull(m_max) && qFuzzyIsNull(max)))
    {
        return;
    }
    m_max = max;
    emit maxChanged();
}

QJsonObject LevelConfig::toJson() const
{
    return {{
        qMakePair(QStringLiteral("threshold"), m_threshold),
        qMakePair(QStringLiteral("secondThreshold"), m_secondThreshold),
        qMakePair(QStringLiteral("min"), m_min),
        qMakePair(QStringLiteral("max"), m_max)
    }};
}

LevelConfig *LevelConfig::fromJson(const QJsonObject &object, IntervalError *parent)
{
    auto level = new LevelConfig(parent);

    if (object.isEmpty())
    {
        return level;
    }

    level->setThreshold(parseDouble(object, QStringLiteral("threshold")));
    level->setSecondThreshold(parseDouble(object, QStringLiteral("secondThreshold")));
    level->setMax(parseDouble(object, QStringLiteral("max")));
    level->setMin(parseDouble(object, QStringLiteral("min")));

    return level;
}

}
}

