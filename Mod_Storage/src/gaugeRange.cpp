#include "gaugeRange.h"
#include "jsonSupport.h"

#include <QJsonObject>

namespace precitec
{
namespace storage
{
namespace
{

double parseReal(const QJsonObject &object, const QString &key, qreal defaultValue = 0.0)
{
    auto it = object.find(key);
    if (it == object.end())
    {
        return defaultValue;
    }
    return it.value().toDouble();
}

}

GaugeRange::GaugeRange(QObject* parent)
    : QObject(parent)
{
}

GaugeRange::~GaugeRange() = default;

void GaugeRange::setRange(qreal range)
{
    if (m_range == range)
    {
        return;
    }
    m_range = range;
    emit rangeChanged();
}

void GaugeRange::setMinFactor(qreal minFactor)
{
    if (m_minFactor == minFactor)
    {
        return;
    }
    m_minFactor = minFactor;
    emit minFactorChanged();
}

void GaugeRange::setMinOffset(qreal minOffset)
{
    if (m_minOffset == minOffset)
    {
        return;
    }
    m_minOffset = minOffset;
    emit minOffsetChanged();
}

void GaugeRange::setMaxFactor(qreal maxFactor)
{
    if (m_maxFactor == maxFactor)
    {
        return;
    }
    m_maxFactor = maxFactor;
    emit maxFactorChanged();
}

void GaugeRange::setMaxOffset(qreal maxOffset)
{
    if (m_maxOffset == maxOffset)
    {
        return;
    }
    m_maxOffset = maxOffset;
    emit maxOffsetChanged();
}

void GaugeRange::setLength(qreal length)
{
    if (m_length == length)
    {
        return;
    }
    m_length = length;
    emit lengthChanged();
}

void GaugeRange::setSecondThreshold(qreal secondThreshold)
{
    if (m_secondThreshold == secondThreshold)
    {
        return;
    }
    m_secondThreshold = secondThreshold;
    emit secondThresholdChanged();
}

QJsonObject GaugeRange::toJson() const
{
    return {{
        qMakePair(QStringLiteral("range"), m_range),
        qMakePair(QStringLiteral("minFactor"), m_minFactor),
        qMakePair(QStringLiteral("minOffset"), m_minOffset),
        qMakePair(QStringLiteral("maxFactor"), m_maxFactor),
        qMakePair(QStringLiteral("maxOffset"), m_maxOffset),
        qMakePair(QStringLiteral("length"), m_length),
        qMakePair(QStringLiteral("secondThreshold"), m_secondThreshold)
    }};
}

GaugeRange* GaugeRange::fromJson(const QJsonObject& object, QObject* parent)
{
    auto gaugeRange = new GaugeRange(parent);

    gaugeRange->setRange(parseReal(object, QStringLiteral("range")));
    gaugeRange->setMinFactor(parseReal(object, QStringLiteral("minFactor")));
    gaugeRange->setMinOffset(parseReal(object, QStringLiteral("minOffset")));
    gaugeRange->setMaxFactor(parseReal(object, QStringLiteral("maxFactor")));
    gaugeRange->setMaxOffset(parseReal(object, QStringLiteral("maxOffset")));
    gaugeRange->setLength(parseReal(object, QStringLiteral("length")));
    gaugeRange->setSecondThreshold(parseReal(object, QStringLiteral("secondThreshold")));

    return gaugeRange;
}

}
}


