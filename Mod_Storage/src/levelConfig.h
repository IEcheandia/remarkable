#pragma once

#include <QObject>

namespace precitec
{
namespace storage
{

class IntervalError;

class LevelConfig : public QObject
{
    Q_OBJECT

public:
    LevelConfig(QObject* parent = nullptr);
    ~LevelConfig() override;

    LevelConfig *duplicate(IntervalError *parent) const;

    double threshold() const
    {
        return m_threshold;
    }
    void setThreshold(double threshold);
    
    double secondThreshold() const
    {
        return m_secondThreshold;
    }
    void setSecondThreshold(double secondThreshold);

    double min() const
    {
        return m_min;
    }
    void setMin(double min);

    double max() const
    {
        return m_max;
    }
    void setMax(double max);

    QJsonObject toJson() const;
    static LevelConfig *fromJson(const QJsonObject &object, IntervalError *parent);

Q_SIGNALS:
    void thresholdChanged();
    void secondThresholdChanged();
    void minChanged();
    void maxChanged();

private:
    double m_threshold = 0.0;
    double m_secondThreshold = 0.0;
    double m_min = 0.0;
    double m_max = 0.0;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::LevelConfig*)

