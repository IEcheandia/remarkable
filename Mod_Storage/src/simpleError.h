#pragma once

#include <QObject>
#include <QUuid>

class TestSimpleError;

namespace precitec
{
namespace storage
{

class AbstractMeasureTask;
class AttributeModel;
class ChangeTracker;
class Seam;
class ReferenceCurve;

class SimpleError : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int resultValue READ resultValue WRITE setResultValue NOTIFY resultValueChanged)

    Q_PROPERTY(int errorType READ errorType WRITE setErrorType NOTIFY errorTypeChanged)

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    Q_PROPERTY(double shift READ shift WRITE setShift NOTIFY shiftChanged)

    Q_PROPERTY(double minLimit READ minLimit WRITE setMinLimit NOTIFY minLimitChanged)

    Q_PROPERTY(double maxLimit READ maxLimit WRITE setMaxLimit NOTIFY maxLimitChanged)

    Q_PROPERTY(precitec::storage::SimpleError::BoundaryType boundaryType READ boundaryType NOTIFY boundaryTypeChanged)

public:
    SimpleError(QObject* parent = nullptr);
    SimpleError(QUuid uuid, QObject* parent = nullptr);
    ~SimpleError() override;

    enum class BoundaryType {
        Static,
        Reference
    };
    Q_ENUM(BoundaryType)

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

    double shift() const
    {
        return m_shift;
    }
    void setShift(double shift);

    double minLimit() const
    {
        return m_minLimit;
    }
    void setMinLimit(double min);

    double maxLimit() const
    {
        return m_maxLimit;
    }
    void setMaxLimit(double max);

    QString name() const
    {
        return m_name;
    }
    void setName(const QString &name);

    BoundaryType boundaryType() const
    {
        return m_boundaryType;
    }

    AbstractMeasureTask *measureTask() const
    {
        return m_measureTask;
    }
    virtual void setMeasureTask(AbstractMeasureTask *task);

    virtual void initFromAttributes(const AttributeModel *attributeModel);

    /**
     * @returns whether change tracking is enabled
     **/
    bool isChangeTracking() const;

    /**
     * @returns the tracked changes as a json array.
     * Implementing subclasses should call the super method to get the
     * changes tracked in the AbstractMeasureTask
     **/
    QJsonArray changes() const;

    virtual QJsonObject toJson() const;
    void fromJson(const QJsonObject &object, AbstractMeasureTask *parent);

Q_SIGNALS:
    void resultValueChanged();
    void errorTypeChanged();
    void shiftChanged();
    void minLimitChanged();
    void maxLimitChanged();
    void nameChanged();
    void measureTaskChanged();
    void boundaryTypeChanged();

protected:
    int getIntValue(const QString &name) const;
    virtual std::string getStringValue(const QString& name) const;
    void addChange(ChangeTracker &&change);
    void updateBoundaryType();

private:
    QUuid m_uuid;
    QUuid m_variantId;
    QString m_name;
    int m_resultValue = 0;
    int m_errorType = 0;
    double m_shift = 0.0;
    double m_minLimit = -100000.0;
    double m_maxLimit = 100000.0;
    BoundaryType m_boundaryType = BoundaryType::Static;

    AbstractMeasureTask *m_measureTask = nullptr;
    std::vector<ChangeTracker> m_changeTracker;

    friend TestSimpleError;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::SimpleError*)

