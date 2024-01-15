#pragma once

#include "simpleError.h"

#include <QObject>
#include <QUuid>
#include <memory>

class TestSeamError;

namespace precitec
{
namespace gui
{
namespace components
{
namespace plotter
{

class DataSet;
class InfiniteSet;

}
}
}
namespace interface
{

class FilterParameter;

}
namespace storage
{

class AbstractMeasureTask;
class AttributeModel;
class ChangeTracker;
class Seam;
enum class CopyMode;

class SeamError : public SimpleError
{
    Q_OBJECT

    Q_PROPERTY(double threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged)
    
    Q_PROPERTY(double secondThreshold READ secondThreshold WRITE setSecondThreshold NOTIFY secondThresholdChanged)

    Q_PROPERTY(double min READ min WRITE setMin NOTIFY minChanged)

    Q_PROPERTY(double max READ max WRITE setMax NOTIFY maxChanged)

    Q_PROPERTY(QUuid envelope READ envelope WRITE setEnvelope NOTIFY envelopeChanged)

    Q_PROPERTY(QString envelopeName READ envelopeName NOTIFY envelopeNameChanged)

    Q_PROPERTY(bool useMiddleCurveAsReference READ useMiddleCurveAsReference WRITE setUseMiddleCurveAsReference NOTIFY useMiddleCurveAsReferenceChanged)

    Q_PROPERTY(bool showSecondThreshold READ showSecondThreshold CONSTANT)

public:
    SeamError(QObject* parent = nullptr);
    SeamError(QUuid uuid, QObject* parent = nullptr);
    ~SeamError() override;

    /**
     * Duplicates this SeamError and assigns a new QUuid to the duplicated SeamError.
     * The @p parent is used as the measureTask of the duplicated SeamError.
     * @param mode selects whether the duplicate gets a new, random Uuid or the same Uuid as this object.
     * @param parent The parent for the new duplicated SeamError.
     **/
    SeamError *duplicate(CopyMode mode, AbstractMeasureTask *parent) const;

    double threshold() const
    {
        return m_threshold;
    }
    
    double secondThreshold() const
    {
        return m_secondThreshold;
    }
    
    void setThreshold(double threshold);

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

    bool useMiddleCurveAsReference() const
    {
        return m_useMiddleCurveAsReference;
    }
    void setUseMiddleCurveAsReference(bool use);

    QUuid envelope() const
    {
        return m_envelope;
    }
    void setEnvelope(const QUuid &id);

    QString envelopeName() const
    {
        return m_envelopeName;
    }

    void unsubscribe();

    std::vector<std::shared_ptr<precitec::interface::FilterParameter> > toParameterList() const;

    void setMeasureTask(AbstractMeasureTask *task) override;

    void initFromAttributes(const AttributeModel *attributeModel) override;

    QJsonObject toJson() const override;
    static SeamError *fromJson(const QJsonObject &object, AbstractMeasureTask *parent);
    
    bool showSecondThreshold();

Q_SIGNALS:
    void thresholdChanged();
    void secondThresholdChanged();
    void minChanged();
    void maxChanged();
    void visualReferenceChanged();
    void envelopeChanged();
    void envelopeNameChanged();
    void useMiddleCurveAsReferenceChanged();

private:
    void setEnvelopeName(const QString& name);
    void updateEnvelopeName();
    double getDoubleValue(const QString &name) const;
    std::string getStringValue(const QString& name) const override;
    void updateLowerBounds();
    void updateUpperBounds();

    double m_threshold = 0.0;
    double m_secondThreshold = 0.0;
    double m_min = 0.0;
    double m_max = 0.0;
    bool m_useMiddleCurveAsReference = true;
    QUuid m_envelope;
    QString m_envelopeName;
    QMetaObject::Connection m_nameChangedConnection;

    friend TestSeamError;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::SeamError*)
