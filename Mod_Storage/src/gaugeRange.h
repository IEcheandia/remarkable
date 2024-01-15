#pragma once

#include <QObject>

namespace precitec
{
namespace storage
{

class GaugeRange : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal range READ range WRITE setRange NOTIFY rangeChanged)

    Q_PROPERTY(qreal minFactor READ minFactor WRITE setMinFactor NOTIFY minFactorChanged)

    Q_PROPERTY(qreal minOffset READ minOffset WRITE setMinOffset NOTIFY minOffsetChanged)

    Q_PROPERTY(qreal maxFactor READ maxFactor WRITE setMaxFactor NOTIFY maxFactorChanged)

    Q_PROPERTY(qreal maxOffset READ maxOffset WRITE setMaxOffset NOTIFY maxOffsetChanged)

    Q_PROPERTY(qreal length READ length WRITE setLength NOTIFY lengthChanged)
    
    Q_PROPERTY(qreal secondTreshold READ secondThreshold WRITE setSecondThreshold NOTIFY secondThresholdChanged)

public:
    GaugeRange(QObject* parent = nullptr);
    ~GaugeRange() override;

    qreal range() const
    {
        return m_range;
    }
    void setRange(qreal range);

    qreal minFactor() const
    {
        return m_minFactor;
    }
    void setMinFactor(qreal minFactor);

    qreal minOffset() const
    {
        return m_minOffset;
    }
    void setMinOffset(qreal minOffset);

    qreal maxFactor() const
    {
        return m_maxFactor;
    }
    void setMaxFactor(qreal maxFactor);

    qreal maxOffset() const
    {
        return m_maxOffset;
    }
    void setMaxOffset(qreal maxOffset);

    qreal length() const
    {
        return m_length;
    }
    void setLength(qreal length);
    
    qreal secondThreshold() const
    {
        return m_secondThreshold;
    }
    void setSecondThreshold(qreal secondThreshold);

    QJsonObject toJson() const;
    static GaugeRange* fromJson(const QJsonObject& object, QObject* parent);

    qreal minThreshold(qreal gauge) const
    {
        return m_minFactor * gauge + m_minOffset;
    }

    qreal maxThreshold(qreal gauge) const
    {
        return m_maxFactor * gauge + m_maxOffset;
    }

Q_SIGNALS:
    void rangeChanged();
    void minFactorChanged();
    void minOffsetChanged();
    void maxFactorChanged();
    void maxOffsetChanged();
    void lengthChanged();
    void secondThresholdChanged();

private:
    qreal m_range = 0.0;
    qreal m_minFactor = 1.0;
    qreal m_minOffset = 0.0;
    qreal m_maxFactor = 1.0;
    qreal m_maxOffset = 0.0;
    qreal m_length = 1.0;
    qreal m_secondThreshold = 0.0;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::GaugeRange*)



