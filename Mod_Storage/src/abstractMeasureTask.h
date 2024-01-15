#pragma once
#include "changeTracker.h"
#include "graphReference.h"

#include <QObject>

#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>

class QJsonObject;

namespace precitec
{
namespace storage
{
class ParameterSet;
class SeamError;
class ReferenceCurve;
class Product;
enum class CopyMode;

/**
 * @brief Base class for everything which can be converted to a MeasureTask, that is SeamSeries, Seam and SeamInterval.
 **/
class AbstractMeasureTask : public QObject
{
    Q_OBJECT
    /**
     * The uuid for this Seam
     **/
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT)
    /**
     * The number of the Seam in the SeamSeries.
     * For all user facing code use @link{visualNumber} instead of number
     **/
    Q_PROPERTY(quint32 number READ number WRITE setNumber NOTIFY numberChanged)
    /**
     * The visual number of this MeasureTask. The visual number needs to be used
     * in user facing code. It is the same as @code{number + 1}.
     **/
    Q_PROPERTY(quint32 visualNumber READ visualNumber NOTIFY numberChanged)
    /**
     * The uuid of the Graph used for this AbstractMeasureTask.
     * In case sub graphs are used this is a null uuid.
     * If set to a not null uuid, the sub graphs are cleared.
     **/
    Q_PROPERTY(QUuid graph READ graph WRITE setGraph NOTIFY graphChanged)
    /**
     * The hardware parameter set for this Product, may be @c null.
     **/
    Q_PROPERTY(precitec::storage::ParameterSet *hardwareParameters READ hardwareParameters WRITE setHardwareParameters NOTIFY hardwareParametersChanged)
    /**
     * Name of this AbstractMeasureTask
     **/
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    /**
     * The uuid of the filter graph parameter set
     **/
    Q_PROPERTY(QUuid graphParamSet READ graphParamSet WRITE setGraphParamSet NOTIFY graphParamSetChanged)
    /**
     * Whether this AbstractMeasureTask uses sub graphs or a single graph.
     * @c true if @link{graph} is null.
     **/
    Q_PROPERTY(bool usesSubGraph READ usesSubGraph NOTIFY graphChanged)

    /**
     * SubGraph ids exposed to QML. Conversion is expensive, don't use in C++
     **/
    Q_PROPERTY(QVariantList subGraphs READ subGraphsAsList NOTIFY graphChanged)

    Q_PROPERTY(QUuid laserControlPreset READ laserControlPreset WRITE setLaserControlPreset NOTIFY laserControlPresetChanged)
    /**
     * The Product this Measure Task belongs to.
     **/
    Q_PROPERTY(precitec::storage::Product* product READ product CONSTANT)
public:
    ~AbstractMeasureTask() override;

    QUuid uuid() const
    {
        return m_uuid;
    }

    quint32 number() const
    {
        return m_number;
    }

    quint32 visualNumber() const
    {
        return number() + 1;
    }

    virtual QUuid graph() const
    {
        return valueOrDefault<SingleGraphReference>(m_graphReference);
    }

    virtual const GraphReference& graphReference() const
    {
        return m_graphReference;
    }

    virtual QUuid graphParamSet() const;

    virtual ParameterSet *hardwareParameters() const
    {
        return m_hardwareParameters;
    }

    QString name() const
    {
        return m_name;
    }

    QUuid laserControlPreset() const
    {
        return m_laserControlPreset;
    }
    
    void setNumber(quint32 number);
    void setGraph(const QUuid &uuid);
    void setGraphReference(const GraphReference& newRef);
    void setGraphParamSet( const QUuid &uuid );
    void setHardwareParameters(ParameterSet *parameters);
    void setName(const QString &name);
    void setLaserControlPreset(const QUuid &preset);

    /**
     * @returns A QJsonObject representation for this object.
     * @see fromJson
     **/
    virtual QJsonObject toJson() const;

    /**
     * @returns whether change tracking is enabled
     * Pure virtual so that the subclasses can delegate to Product.
     **/
    virtual bool isChangeTracking() const = 0;

    /**
     * @returns the tracked changes as a json array.
     * Implementing subclasses should call the super method to get the
     * changes tracked in the AbstractMeasureTask
     **/
    virtual QJsonArray changes() const;

    /**
     * Creates a hardware parameter set if there is none yet.
     **/
    void createHardwareParameters();

    const std::vector<SeamError*> seamErrors() const
    {
        return m_errors;
    }

    SeamError *addError(const QUuid &variantId);
    void removeError(int index);

    /**
     * The sub graphs for this AbstractMeasureTask.
     **/
    virtual const std::vector<QUuid> &subGraphs() const
    {
        return valueOrDefault<SubGraphReference>(m_graphReference);
    }

    QVariantList subGraphsAsList() const;

    /**
     * Sets the sub graphs for this AbstractMeasureTask.
     * If set to a non empty list of uuids, the graphUuid is set to a null uuid.
     **/
    void setSubGraphs(const std::vector<QUuid> &subGraphs);

    virtual bool usesSubGraph() const
    {
        return hasSubGraphs(m_graphReference);
    }

    static constexpr uint maxLevel()
    {
        return 3;
    }
    static QColor levelColor(uint level);

    const std::vector<ReferenceCurve*>& referenceCurves() const
    {
        return m_referenceCurves;
    }

    ReferenceCurve* createReferenceCurve(const int type);
    ReferenceCurve* copyReferenceCurve(ReferenceCurve* curve);
    void removeReferenceCurve(ReferenceCurve* curve);
    ReferenceCurve* findReferenceCurve(const QUuid& uuid);

    Q_INVOKABLE QVariantList referenceCurveList() const;

    /**
     * Duplicate the Errors and Reference Curves of @p source to this task
     **/
    void copyErrorsAndReferenceCurves(CopyMode mode, const AbstractMeasureTask *source);

    /**
     * Calculates the number from provided @p visualNumber.
     **/
    Q_INVOKABLE quint32 numberFromVisualNumber(quint32 visualNumber)
    {
        return visualNumber - 1;
    }

    virtual Product *product() const = 0;

Q_SIGNALS:
    void numberChanged();
    void graphChanged();
    void subGraphChanged();
    void graphParamSetChanged();
    void hardwareParametersChanged();
    void nameChanged();
    void laserControlPresetChanged();

protected:
    /**
     * Constructs an AbstractMeasureTask with the given @p uuid and the given @p parent.
     **/
    explicit AbstractMeasureTask(const QUuid &uuid, QObject *parent);

    /**
     * Inits the properties from the given Json @p object.
     **/
    void fromJson(const QJsonObject &object);

    /**
     * Adds a @p change to the change tracker.
     **/
    void addChange(ChangeTracker &&change);

    /**
     * Initializes this AbstractMeasureTask as a copy of @p source.
     * Hardware ParameterSet, SeamError, and Reference Curves get duplicated
     **/
    void copy(CopyMode mode, const AbstractMeasureTask *source);

private:
    QUuid m_uuid;
    quint32 m_number = 0;
    GraphReference m_graphReference;
    QUuid m_graphParamSetUuid;
    ParameterSet *m_hardwareParameters = nullptr;
    QString m_name;
    std::vector<ChangeTracker> m_changeTracker;
    std::vector<SeamError*> m_errors;
    std::vector<ReferenceCurve*> m_referenceCurves;
    QUuid m_laserControlPreset;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::AbstractMeasureTask*)
