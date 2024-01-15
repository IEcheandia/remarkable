#pragma once

#include <QObject>
#include <QUuid>

#include "changeTracker.h"

class TestReferenceCurve;

namespace precitec
{
namespace storage
{

class AbstractMeasureTask;
enum class CopyMode;

class ReferenceCurve : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    Q_PROPERTY(int resultType READ resultType NOTIFY resultTypeChanged)

    Q_PROPERTY(ReferenceType referenceType READ referenceType WRITE setReferenceType NOTIFY referenceTypeChanged)

    Q_PROPERTY(QString referenceTypeString READ referenceTypeString NOTIFY referenceTypeChanged)

    Q_PROPERTY(float jitter READ jitter WRITE setJitter NOTIFY jitterChanged)

    Q_PROPERTY(bool used READ used NOTIFY usedChanged)

public:
    ReferenceCurve(QObject* parent = nullptr);
    ~ReferenceCurve() override;

    enum class ReferenceType {
        Average,
        Median,
        MinMax
    };
    Q_ENUM(ReferenceType)

    /**
     * Duplicates this ReferenceCurve and assigns a new QUuid to the duplicated ReferenceCurve.
     * The @p parent is used as the AbstractMeasureTask of the duplicated ReferenceCurve.
     * @param parent The parent for the new duplicated ReferenceCurve.
     **/
    ReferenceCurve* duplicate(CopyMode mode, AbstractMeasureTask* parent) const;

    QUuid uuid() const
    {
        return m_uuid;
    }

    QUuid lower() const
    {
        return m_lowerUuid;
    }

    QUuid middle() const
    {
        return m_middleUuid;
    }

    QUuid upper() const
    {
        return m_upperUuid;
    }

    int resultType() const
    {
        return m_resultType;
    }
    void setResultType(int type);

    QString name() const
    {
        return m_name;
    }
    void setName(const QString& name);

    AbstractMeasureTask* measureTask() const
    {
        return m_task;
    }
    void setMeasureTask(AbstractMeasureTask* task);

    float jitter() const
    {
        return m_jitter;
    }
    void setJitter(float jitter);

    ReferenceType referenceType() const
    {
        return m_referenceType;
    }
    void setReferenceType(ReferenceType referenceType);

    QString referenceTypeString() const;

    const std::vector<QString>& sourceOfCurve() const
    {
        return m_sourceOfCurve;
    }
    void setSourceOfCurve(const std::vector<QString>& sourceOfCurve);

    bool isSource(const QString &serialNumber) const;
    bool hasSource() const
    {
        return !m_sourceOfCurve.empty();
    }

    bool used() const
    {
        return !m_subscriptions.empty();
    }
    void subscribe(const QUuid& errorId);
    void unsubscribe(const QUuid& errorId);

    bool isChangeTracking() const;
    QJsonArray changes() const;

    QJsonObject toJson() const;
    static ReferenceCurve* fromJson(const QJsonObject &object, AbstractMeasureTask* parent);

    bool hasId(const QUuid& id) const
    {
        return id == m_lowerUuid || id == m_middleUuid || id == m_upperUuid;
    }

Q_SIGNALS:
    void resultTypeChanged();
    void nameChanged();
    void measureTaskChanged();
    void jitterChanged();
    void referenceTypeChanged();
    void markAsChanged();
    void usedChanged();

private:
    void addChange(ChangeTracker&& change);

    static QString typeToString(const ReferenceType type);
    static ReferenceType stringToType(const QString& str);

    QUuid m_uuid;
    QUuid m_lowerUuid;
    QUuid m_middleUuid;
    QUuid m_upperUuid;
    QString m_name;
    int m_resultType = -1;
    ReferenceType m_referenceType = ReferenceType::Average;
    float m_jitter = 0.0f;
    AbstractMeasureTask* m_task = nullptr;

    std::vector<QUuid> m_subscriptions;

    std::vector<QString> m_sourceOfCurve;

    std::vector<ChangeTracker> m_changeTracker;

    friend TestReferenceCurve;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::ReferenceCurve*)


