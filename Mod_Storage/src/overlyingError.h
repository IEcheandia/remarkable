#pragma once

#include <QObject>
#include <QUuid>
#include <memory>


namespace precitec
{
namespace storage
{

class AttributeModel;
class ChangeTracker;

class OverlyingError : public QObject
{
    Q_OBJECT
    /**
     * The monitored value (enum of result)
     **/
    Q_PROPERTY(int resultValue READ resultValue WRITE setResultValue NOTIFY resultValueChanged)
    /**
     * The error to throw (enum of the error)
     **/
    Q_PROPERTY(int errorType READ errorType WRITE setErrorType NOTIFY errorTypeChanged)

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    Q_PROPERTY(int threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)

public:
    OverlyingError(QObject* parent = nullptr);
    OverlyingError(QUuid uuid, QObject* parent = nullptr);
    ~OverlyingError() override;

    QUuid uuid() const
    {
        return m_uuid;
    }

    QUuid variantId() const
    {
        return m_variantId;
    }
    void setVariantId(const QUuid &id);

    int resultValue() const
    {
        return m_resultValue;
    }
    void setResultValue(int resultValue);

    int errorType() const
    {
        return m_errorType;
    }
    void setErrorType(int errorType);

    QString name() const
    {
        return m_name;
    }
    void setName(const QString &name);

    int threshold() const
    {
        return m_threshold;
    }
    void setThreshold(int threshold);

    virtual void initFromAttributes(const AttributeModel *attributeModel);

    /**
     * @returns whether change tracking is enabled
     **/
    virtual bool isChangeTracking() = 0;

    /**
     * @returns the tracked changes as a json array.
     * Implementing subclasses should call the super method to get the
     * changes tracked in the AbstractMeasureTask
     **/
    QJsonArray changes() const;

    virtual QJsonObject toJson() const;
    void fromJson(const QJsonObject &object);

Q_SIGNALS:
    void resultValueChanged();
    void errorTypeChanged();
    void nameChanged();
    void thresholdChanged();

protected:
    int getIntValue(const QString &name) const;
    virtual double getDoubleValue(const QString &name) const;
    void addChange(ChangeTracker &&change);

private:
    QUuid m_uuid;
    QUuid m_variantId;
    QString m_name;
    int m_resultValue = 0;
    int m_errorType = 0;
    int m_threshold = 0;

    std::vector<ChangeTracker> m_changeTracker;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::OverlyingError*)


