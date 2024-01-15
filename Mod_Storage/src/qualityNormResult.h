#pragma once

#include <QObject>

class TestQualityNormResult;

namespace precitec
{
namespace storage
{

class QualityNorm;
class QualityNormLevel;

class QualityNormResult : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int resultType READ resultType WRITE setResultType NOTIFY resultTypeChanged)

public:
    QualityNormResult(QObject* parent = nullptr);
    ~QualityNormResult() override;

    int resultType() const
    {
        return m_resultType;
    }
    void setResultType(int resultType);

    static QualityNormResult* fromJson(const QJsonObject& object, QualityNorm* parent);

    QualityNormLevel* levelAt(int level);

Q_SIGNALS:
    void resultTypeChanged();

private:
    int m_resultType = -1;
    std::vector<QualityNormLevel*> m_levels;

    friend TestQualityNormResult;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::QualityNormResult*)


