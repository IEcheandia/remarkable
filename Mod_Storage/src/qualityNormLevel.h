#pragma once

#include <QObject>

class TestQualityNormLevel;
class TestQualityNormResult;

namespace precitec
{
namespace storage
{

class QualityNormResult;
class GaugeRange;

class QualityNormLevel : public QObject
{
    Q_OBJECT

public:
    QualityNormLevel(int level = 1, QObject* parent = nullptr);
    ~QualityNormLevel() override;

    int level() const
    {
        return m_level;
    }

    static QualityNormLevel* fromJson(const QJsonObject& object, QualityNormResult* parent, int level);

    GaugeRange* range(qreal gauge);

private:
    int m_level;
    std::vector<GaugeRange*> m_gaugeRanges;

    friend TestQualityNormLevel;
    friend TestQualityNormResult;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::QualityNormLevel*)


