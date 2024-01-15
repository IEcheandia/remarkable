#pragma once
#include "abstractMeasureTask.h"

#include <QObject>
#include <QPointF>
#include <QUuid>
#include <QRect>

#include <vector>

class QXmlStreamReader;

namespace precitec
{
namespace storage
{
class SeamSeries;
class SeamInterval;
class IntervalError;
class Product;
class LinkedSeam;
enum class CopyMode;

/**
 * @brief Represents one Seam in a SeamSeries.
 *
 **/
class Seam : public AbstractMeasureTask
{
    Q_OBJECT
    /**
     * The SeamSeries this Seam belongs to.
     **/
    Q_PROPERTY(precitec::storage::SeamSeries *seamSeries READ seamSeries CONSTANT)
    /**
     * Length of all @link{SeamInterval}s (internally [um], user level [mm]), negative = infinite
     **/
    Q_PROPERTY(int length READ length NOTIFY lengthChanged)

    /**
     * The first SeamInterval for this Seam. This is mostly useful for the case that there is only one SeamInterval.
     **/
    Q_PROPERTY(precitec::storage::SeamInterval *firstSeamInterval READ firstSeamInterval NOTIFY firstSeamIntervalChanged)

    /**
     * All SeamIntevals from Seams. This property is intended for interchange with QML and
     * thus returns a QVariantList instead of std::vector<SeamInterval*>. Do not use from C++ side, instead
     * iterate over seams.
     **/
    Q_PROPERTY(QVariantList allSeamIntervals READ allSeamIntervals NOTIFY allSeamIntervalsChanged)

    /**
     * The count of the seamItervals
     **/
    Q_PROPERTY(int seamIntervalsCount READ seamIntervalsCount NOTIFY seamIntervalsCountChanged)

    /**
     * The position of this Seam in the assembly image. If not set the position is -1, -1
     **/
    Q_PROPERTY(QPointF positionInAssemblyImage READ positionInAssemblyImage WRITE setPositionInAssemblyImage NOTIFY positionInAssemblyImageChanged)

    Q_PROPERTY(QRect roi READ roi WRITE setRoi NOTIFY roiChanged)
    /**
     * Space delta between two successive triggers [um]
     **/
    Q_PROPERTY(int triggerDelta READ triggerDelta WRITE setTriggerDelta NOTIFY triggerDeltaChanged)
    /**
     * Welding/ cutting velocity (internally [um/s], user level [mm/s])
     **/
    Q_PROPERTY(int velocity READ velocity WRITE setVelocity NOTIFY velocityChanged)
    /**
     * Left thinckness for SOUVIS6000 [mm]
     **/
    Q_PROPERTY(int thicknessLeft READ thicknessLeft WRITE setThicknessLeft NOTIFY thicknessLeftChanged)
    /**
     * Right thinckness for SOUVIS6000 [mm]
     **/
    Q_PROPERTY(int thicknessRight READ thicknessRight WRITE setThicknessRight NOTIFY thicknessRightChanged)
    /**
     * Target difference for SOUVIS6000 [mm]
     **/
    Q_PROPERTY(int targetDifference READ targetDifference WRITE setTargetDifference NOTIFY targetDifferenceChanged)
    /**
     * Moving direction for SOUVIS6000
     **/
    Q_PROPERTY(MovingDirection movingDirection READ movingDirection WRITE setMovingDirection NOTIFY movingDirectionChanged)

    /**
     * All linked seams. Property only for QML.
     **/
    Q_PROPERTY(QVariantList linkedSeams READ allLinkedSeams NOTIFY linkedSeamsChanged)

    /**
     * Whether the seam has linked seams.
     **/
    Q_PROPERTY(bool hasLinkedSeams READ hasLinkedSeams NOTIFY linkedSeamsChanged)

public:
    /**
     * Constructs a Seam with the given @p uuid and the given @p parentSeries.
     * The @p parentSeries is also set as the QObject parent for this SeamSeries
     **/
    explicit Seam(const QUuid &uuid, SeamSeries *parentSeries);
    ~Seam() override;

    enum class MovingDirection
    {
        FromUpper,
        FromLower
    };
    Q_ENUM(MovingDirection)

    /**
     * Duplicates this Seam.
     * @param mode selects whether the duplicate gets a new, random Uuid or the same Uuid as this object.
     * @param parent The parent for the new duplicated Seam.
     **/
    Seam *duplicate(CopyMode mode, SeamSeries *parent = nullptr) const;

    /**
     * Creates a link to this Seam with @p label.
     **/
    Seam *link(const QString &label);

    /**
     * Finds a link to this Seam with @p label.
     **/
    Seam *findLink(const QString &label);

    /**
     * Duplicates the Seam Intervals from another Seam
     * @param mode selects whether the duplicate gets a new, random Uuid or the same Uuid as their source.
     **/
    void duplicateSeamIntervals(CopyMode mode, const Seam* source);

    /**
     * Duplicates the Interval Errors from another Seam
     * @param mode selects whether the duplicate gets a new, random Uuid or the same Uuid as their source.
     **/
    void duplicateIntervalErrors(CopyMode mode, const Seam* source);

    SeamSeries *seamSeries() const
    {
        return m_seamSeries;
    }

    /**
     * @returns The @link{SeamInterval}s for this Seam.
     **/
    const std::vector<SeamInterval*> &seamIntervals() const
    {
        return m_intervals;
    }

    int length() const;

    QPointF positionInAssemblyImage() const
    {
        return m_positionInAssemblyImage;
    }
    void setPositionInAssemblyImage(const QPointF &position);

    QRect roi() const
    {
        return m_roi;
    }
    void setRoi(const QRect &roi);

    int triggerDelta() const
    {
        return m_triggerDelta;
    }
    void setTriggerDelta(int delta);

    int velocity() const
    {
        return m_velocity;
    }
    void setVelocity(int velocity);

    int thicknessLeft() const
    {
        return m_thicknessLeft;
    }
    void setThicknessLeft(int thickness);

    int thicknessRight() const
    {
        return m_thicknessRight;
    }
    void setThicknessRight(int thickness);

    int targetDifference() const
    {
        return m_targetDifference;
    }
    void setTargetDifference(int difference);

    int moveDirection() const
    {
        return static_cast<int>(m_movingDirection);
    }
    MovingDirection movingDirection() const
    {
        return m_movingDirection;
    }
    void setMovingDirection(MovingDirection direction);

    Q_INVOKABLE precitec::storage::SeamInterval *firstSeamInterval() const;

    const std::vector<IntervalError*> intervalErrors() const
    {
        return m_intervalErrors;
    }
    IntervalError *addIntervalError(const QUuid &variantId);
    void removeIntervalError(int index);
    int intervalErrorCount();

    /**
     * @returns the ParameterSet with the given @p id, evaluates the hardware parameters
     * of this Seam and all contained seamIntervals. If there is no such ParameterSet @c null is returned
     **/
    ParameterSet *findHardwareParameterSet(const QUuid &id) const;

    QJsonObject toJson() const override;

    bool isChangeTracking() const override;

    QJsonArray changes() const override;

    /**
     * Creates the first SeamInterval for this Seam.
     * Does nothing if the Seam already contains SeamIntervals.
     **/
    void createFirstSeamInterval();

    /**
     * Creates the Filter Parameter Set for this Seam.
     * Does nothing if the Seam already contains a valid ParameterSet.
     **/
    void createFilterParameterSet();

    /**
     * Creates a new SeamInterval for this Seam.
     **/
    Q_INVOKABLE precitec::storage::SeamInterval *createSeamInterval();

    /**
     * All SeamIntevals from Seams. This property is intended for interchange with QML and
     * thus returns a QVariantList instead of std::vector<SeamInterval*>. Do not use from C++ side, instead
     * iterate over seams.
     **/
    Q_INVOKABLE QVariantList allSeamIntervals() const;

    /**
     * Removes the @p seamInterval from this Seam and deletes @p seamInterval
     **/
    Q_INVOKABLE void destroySeamInterval(precitec::storage::SeamInterval *seamInterval);

    int seamIntervalsCount() const
    {
        return m_intervals.size();
    }

    int maxSeamIntervalNumber() const;

    /**
     * finds a SeamInterval of this Seam, given by the Id
     **/
    SeamInterval *findSeamInterval(const QUuid &id) const;

    /**
     * Returns the IntervalErrors of this Seam.
     **/
    std::vector<IntervalError*> allIntervalErrors() const;

    const std::vector<LinkedSeam*> &linkedSeams() const
    {
        return m_linkedSeams;
    }

    QVariantList allLinkedSeams() const;

    bool hasLinkedSeams() const
    {
        return !m_linkedSeams.empty();
    }

    /**
     * Creates a new Seam from the provided json @p object.
     * If the @p object is empty @c null is returned.
     *
     * The uuid is taken from the @p object if provided. If no uuid is
     * provided a new uuid gets generated.
     *
     * The @p parent is set as the parent SeamSeries for the created seam.
     *
     * @returns New created Seam or @c null if @p object is empty.
     **/
    static Seam *fromJson(const QJsonObject &object, SeamSeries *parent);

    /**
     * Imports SeamSeries from @p xml. The QXmlStreamReader must be at a
     * start element for a SeamSeries element.
     *
     * @returns the imported SeamSeries on succes or @c null on error.
     **/
    static Seam *fromXml(QXmlStreamReader &xml, SeamSeries *parent);

    Product *product() const override;

Q_SIGNALS:
    void lengthChanged();
    void firstSeamIntervalChanged();
    void allSeamIntervalsChanged();
    void seamIntervalsCountChanged();
    void positionInAssemblyImageChanged();
    void roiChanged();
    void triggerDeltaChanged();
    void velocityChanged();
    void movingDirectionChanged();
    void thicknessLeftChanged();
    void thicknessRightChanged();
    void targetDifferenceChanged();
    void linkedSeamsChanged();

private:
    SeamSeries *m_seamSeries;
    QRect m_roi = QRect{0, 0, 0, 0};
    int m_triggerDelta = 10000;
    int m_velocity = 100000;
    MovingDirection m_movingDirection = MovingDirection::FromUpper;
    int m_thicknessLeft = 0;
    int m_thicknessRight = 0;
    int m_targetDifference = 0;
    std::vector<SeamInterval*> m_intervals;
    std::vector<IntervalError*> m_intervalErrors;
    QPointF m_positionInAssemblyImage{-1, -1};

    void connectDestroySignal(Seam * linked);

    std::vector<LinkedSeam*> m_linkedSeams;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::Seam*)
