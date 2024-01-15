#pragma once

#include <QObject>
#include <QUuid>

class TestQualityNorm;

namespace precitec
{
namespace storage
{

class QualityNormResult;

class QualityNorm : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
    QualityNorm(QObject* parent = nullptr);
    QualityNorm(const QUuid& id, const QString& name, QObject* parent = nullptr);
    ~QualityNorm() override;

    QUuid uuid() const
    {
        return m_uuid;
    }

    QString name() const
    {
        return m_name;
    }
    void setName(const QString& name);

    static QualityNorm* fromJson(const QJsonObject& object, QObject* parent);
    void addCommonResultsFromJson(const QJsonObject& object);

    Q_INVOKABLE precitec::storage::QualityNormResult* qualityNormResult(int resultType);

Q_SIGNALS:
    void nameChanged();

private:
    QUuid m_uuid;
    QString m_name;
    std::vector<QualityNormResult*> m_qualityNormResults;

    friend TestQualityNorm;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::QualityNorm*)

