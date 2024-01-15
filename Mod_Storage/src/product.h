#pragma once
#include "changeTracker.h"

#include <QObject>
#include <QJsonArray>
#include <QUuid>

#include <vector>
#include <forward_list>

class QIODevice;
class QFile;
class QDataStream;

namespace precitec
{
namespace gui
{
namespace components
{
namespace plotter
{
class ColorMap;
}
}
}
namespace storage
{
class AbstractMeasureTask;
class ParameterSet;
class Seam;
class SeamSeries;
class SeamError;
class IntervalError;
class SeamSeriesError;
class ProductError;
class ReferenceCurve;
class ReferenceCurveData;
enum class CopyMode;

class Product : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(int type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(bool endless READ isEndless WRITE setEndless NOTIFY endlessChanged)
    Q_PROPERTY(TriggerSource triggerSource READ triggerSource WRITE setTriggerSource NOTIFY triggerSourceChanged)
    Q_PROPERTY(TriggerMode triggerMode READ triggerMode WRITE setTriggerMode NOTIFY triggerModeChanged)
    Q_PROPERTY(int startPositionYAxis READ startPositionYAxis WRITE setStartPositionYAxis NOTIFY startPositionYAxisChanged)
    /**
     * The hardware parameter set for this Product, may be @c null.
     **/
    Q_PROPERTY(precitec::storage::ParameterSet *hardwareParameters READ hardwareParameters WRITE setHardwareParameters NOTIFY hardwareParametersChanged)
    /**
     * Whether this Product is the default product.
     **/
    Q_PROPERTY(bool defaultProduct READ isDefaultProduct WRITE setDefaultProduct NOTIFY defaultProductChanged)
    /**
     * The path of the file from which this Product was loaded
     **/
    Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged)
    /**
     * The length unit for all length in this Product and its Seam. Default is millimeter.
     **/
    Q_PROPERTY(LengthUnit lengthUnit READ lengthUnit WRITE setLengthUnit NOTIFY lengthUnitChanged)
    /**
     * All Seams from all SeamSeries. This property is intended for interchange with QML and
     * thus returns a QVariantList instead of std::vector<Seam*>. Do not use from C++ side, instead
     * iterate over seamSeries.
     **/
    Q_PROPERTY(QVariantList allSeams READ allSeams NOTIFY seamsChanged)
    /**
     * The same as @link {allSeams}, but excluding linked seams
     **/
    Q_PROPERTY(QVariantList allRealSeams READ allRealSeams NOTIFY seamsChanged)
    /**
     * File name of the assembly image for this Product. An empty name means no image is set
     **/
    Q_PROPERTY(QString assemblyImage READ assemblyImage WRITE setAssemblyImage NOTIFY assemblyImageChanged)

    /**
     * All SeamSeries of this Product. This property is intended for interchange with QML and
     * thus returns a QVariantList instead of std::vector<SeamSeries*>. Do not use from C++ side, instead
     * use the direct accessor.
     **/
    Q_PROPERTY(QVariantList seamSeries READ allSeamSeries NOTIFY seamSeriesChanged)

    Q_PROPERTY(precitec::gui::components::plotter::ColorMap *signalyQualityColorMap READ signalyQualityColorMap NOTIFY signalyQualityColorMapChanged)

    Q_PROPERTY(precitec::gui::components::plotter::ColorMap *errorLevelColorMap READ errorLevelColorMap NOTIFY errorLevelColorMapChanged)

    Q_PROPERTY(QUuid qualityNorm READ qualityNorm WRITE setQualityNorm NOTIFY qualityNormChanged)

    Q_PROPERTY(int lwmTriggerSignalType READ lwmTriggerSignalType WRITE setLwmTriggerSignalType NOTIFY lwmTriggerSignalTypeChanged)

    Q_PROPERTY(double lwmTriggerSignalThreshold READ lwmTriggerSignalThreshold WRITE setLwmTriggerSignalThreshold NOTIFY lwmTriggerSignalThresholdChanged)

public:
    enum class TriggerSource
    {
        Software,
        External,
        GrabberControlled
    };
    Q_ENUM(TriggerSource)
    enum class TriggerMode
    {
        Single,
        Burst,
        Continue,
        None
    };
    Q_ENUM(TriggerMode)
    /**
     * Enum describing the unit to use for length.
     **/
    enum class LengthUnit
    {
        Millimeter,
        Degree
    };
    Q_ENUM(LengthUnit)
    explicit Product(const QUuid &uuid, QObject *parent = nullptr);
    ~Product() override;

    /**
     * Duplicates this Products and assigns a new QUuid to the duplicated Product.
     * @param mode selects whether the duplicate gets a new, random Uuid or the same Uuid as this object.
     * @param parent The parent for the new duplicated Product.
     **/
    Product *duplicate(CopyMode mode, QObject *parent = nullptr) const;

    QUuid uuid() const
    {
        return m_uuid;
    }

    QString name() const
    {
        return m_name;
    }

    int type() const
    {
        return m_type;
    }

    bool isEndless() const
    {
        return m_endless;
    }

    TriggerSource triggerSource() const
    {
        return m_triggerSource;
    }

    TriggerMode triggerMode() const
    {
        return m_triggerMode;
    }

    int startPositionYAxis() const
    {
        return m_startPositionYAxis;
    }

    const std::vector<SeamSeries*> &seamSeries() const
    {
        return m_seamSeries;
    }

    ParameterSet *hardwareParameters() const
    {
        return m_hardwareParameters;
    }
    void createHardwareParameters();

    const std::vector<ProductError*> overlyingErrors() const
    {
        return m_overlyingErrors;
    }
    ProductError *addOverlyingError(const QUuid &variantId);
    void removeOverlyingError(int index);

    std::vector<SeamSeriesError*> allSeamSeriesErrors() const;

    std::vector<SeamError*> allSeamErrors() const;

    std::vector<IntervalError*> allIntervalErrors() const;
    int intervalErrorCount();

    bool isDefaultProduct() const
    {
        return m_default;
    }

    QString filePath() const
    {
        return m_filePath;
    }
    void setFilePath(const QString &filePath);

    LengthUnit lengthUnit() const
    {
        return m_lengthUnit;
    }
    QString assemblyImage() const
    {
        return m_assemblyImage;
    }

    QString referenceCruveStorageDir() const
    {
        return m_referenceCurveStorageDir;
    }

    precitec::gui::components::plotter::ColorMap *signalyQualityColorMap() const
    {
        return m_signalyQualityColorMap;
    }

    precitec::gui::components::plotter::ColorMap *errorLevelColorMap() const
    {
        return m_errorLevelColorMap;
    }

    QUuid qualityNorm() const
    {
        return m_qualityNorm;
    }

    QUuid laserControlPreset() const
    {
        return m_laserControlPreset;
    }

    int lwmTriggerSignalType() const
    {
        return m_lwmTriggerSignalType;
    }

    double lwmTriggerSignalThreshold() const
    {
        return m_lwmTriggerSignalThreshold;
    }

    void setName(const QString &name);
    void setType(int type);
    void setEndless(bool endless);
    void setTriggerSource(TriggerSource source);
    void setTriggerMode(TriggerMode mode);
    void setStartPositionYAxis(int pos);
    void setHardwareParameters(ParameterSet *parameters);
    void setDefaultProduct(bool defaultProduct);
    void setLengthUnit(LengthUnit unit);
    void setAssemblyImage(const QString &image);
    void setReferenceCurveStorageDir(const QString &dir);
    void setQualityNorm(const QUuid& qualityNorm);
    void setLaserControlPreset(const QUuid& preset);
    void setLwmTriggerSignalType(int enumType);
    void setLwmTriggerSignalThreshold(double threshold);

    /**
     * @returns the ParameterSet with the given @p id, evaluates the hardware parameters
     * of this Product, it's SeamSeries and all children.
     * If there is no such ParameterSet @c null is returned.
     **/
    ParameterSet *findHardwareParameterSet(const QUuid &id) const;

    /**
     * @returns The filter ParameterSet with the given @p id, or @c null if there is no such ParameterSet.
     **/
    Q_INVOKABLE precitec::storage::ParameterSet *filterParameterSet(const QUuid &id) const;

    /**
     * @returns the Seam in this Product with the given @p id, if there is no such Seam @c null is returned
     **/
    Q_INVOKABLE precitec::storage::Seam *findSeam(const QUuid &id) const;

    /**
     * @returns the Seam with the given @p seam number in the SeamSeries with the given @p seamSeries number, if there is no such Seam @c null is returned.
     **/
    Q_INVOKABLE precitec::storage::Seam *findSeam(quint32 seamSeries, quint32 seam) const;

    Q_INVOKABLE precitec::storage::SeamSeries *findSeamSeries(quint32 seamSeries) const;
    Q_INVOKABLE precitec::storage::SeamSeries *findSeamSeries(const QUuid &id) const;

    /**
     * Finds the AbstractMeasureTask with the given @p id or @c null if none exists.
     * Considers recursively all SeamSeries, Seams and SeamIntervals
     **/
    AbstractMeasureTask *findMeasureTask(const QUuid &id) const;

    /**
     * Creates a new Seam and adds it to the first SeamSeries.
     * The number of the new Seam is an increment to the current highest Seam number in the first SeamSeries.
     * @returns new created Seam.
     **/
    Q_INVOKABLE precitec::storage::Seam *createSeam();

    /**
     * Removes the @p seamSeries from this Product and deletes @p seamSeries if it was a SeamSeries for this Product.
     **/
    Q_INVOKABLE void destroySeamSeries(precitec::storage::SeamSeries *seam);

    /**
     * @returns all Seams from all SeamSeries. This method is intended for interchange with QML and
     * thus returns a QVariantList instead of std::vector<Seam*>. Do not use from C++ side, instead
     * iterate over seamSeries.
     **/
    Q_INVOKABLE QVariantList allSeams() const;

    /**
     * @returns all Seams from all SeamSeries, excluding LinkedSeams. This method is intended for interchange with QML and
     * thus returns a QVariantList instead of std::vector<Seam*>. Do not use from C++ side, instead
     * iterate over seamSeries.
     **/
    Q_INVOKABLE QVariantList allRealSeams() const;

    std::vector<Seam*> seams() const;

    /**
     * @returns the previous Seam from reference @p seam or @c nullptr if @p seam is the first Seam.
     **/
    Q_INVOKABLE precitec::storage::Seam *previousSeam(precitec::storage::Seam *seam) const;

    /**
     * @returns the next Seam from reference @p seam or @c nullptr if @p seam is the last Seam.
     **/
    Q_INVOKABLE precitec::storage::Seam *nextSeam(precitec::storage::Seam *seam) const;

    /**
     * @returns the previous SeamSeries from reference @p seamSeries or @c nullptr if @p seamSeries is the first SeamSeries.
     **/
    Q_INVOKABLE precitec::storage::SeamSeries *previousSeamSeries(precitec::storage::SeamSeries *seam) const;

    /**
     * @returns the next SeamSeries from reference @p seamSeries or @c nullptr if @p seamSeries is the last SeamSeries.
     **/
    Q_INVOKABLE precitec::storage::SeamSeries *nextSeamSeries(precitec::storage::SeamSeries *seamSeries) const;

    /**
     * All SeamSeries of this Product. This property is intended for interchange with QML and
     * thus returns a QVariantList instead of std::vector<SeamSeries*>. Do not use from C++ side, instead
     * use the direct accessor.
     **/
    QVariantList allSeamSeries() const;

    /**
     * Creates the first SeamSeries for this Product.
     * Does nothing if this Product already contains SeamSeries.
     **/
    void createFirstSeamSeries();

    /**
     * Creates a new SeamSeries and adds it to the product.
     * The number of the new SeamSeries is an increment to the current highest SeamSeries number.
     * @returns new created SeamSeries.
     **/
    precitec::storage::SeamSeries *createSeamSeries();

    /**
     * Creates a new SeamSeries and adds it to the product as a duplicate of the SeamSeries @p seriesToCopy.
     * The number of the new SeamSeries is an increment to the current highest SeamSeries number.
     * @param mode selects whether the duplicate gets a new, random Uuid or the same Uuid as the @p seriesToCopy.
     * @returns new created SeamSeries.
     **/
    precitec::storage::SeamSeries *createSeamSeriesCopy(CopyMode mode, SeamSeries *seriesToCopy);

    /**
     * @returns the currently loaded filter ParameterSet. Note: if a ParameterSet was discarded it is not included, use
     * @link{ensureAllFilterParameterSetsLoaded} to load the ParameterSets prior to access this method.
     **/
    const std::vector<ParameterSet*> &filterParameterSets() const
    {
        return m_filterParameterSets;
    }

    /**
     * Adds @p set to the filter parameters, if there is already a ParameterSet with
     * the same uuid, it gets replaced.
     **/
    void addFilterParameterSet(ParameterSet *set);

    /**
     * Removes and deletes the ParameterSet with the given @p id.
     **/
    void removeFilterParameterSet(const QUuid &id);

    /**
     * Discards the ParameterSet and marks it for lazy loading.
     * To load the ParameterSet again use @link{ensureFilterParameterSetLoaded} or @link{ensureAllFilterParameterSetsLoaded}/
     **/
    void discardFilterParameterSet(ParameterSet* parameterSet);

    /**
     * Whether the product contains a Filter ParameterSet with @p id. Also considers discarded ParameterSets.
     **/
    bool containsFilterParameterSet(const QUuid& id) const;

    /**
     * Ensures that the filter ParameterSet @link{id} gets loaded from Json and is no longer discarded.
     * This reverts @link{discardFilterParameterSet}.
     **/
    Q_INVOKABLE void ensureFilterParameterSetLoaded(const QUuid& id);

    /**
     * Ensures that all filter ParameterSet gets loaded from Json and are no longer discarded.
     * This reverts @link{discardFilterParameterSet} for all discarded filter ParameterSets.
     **/
    Q_INVOKABLE void ensureAllFilterParameterSetsLoaded();

    /**
     * @returns A QJsonObject representation for this object.
     * @see fromJson
     **/
    QJsonObject toJson() const;

    /**
     * Writes this object as json to the @p device.
     **/
    void toJson(QIODevice *device) const;

    /**
     * Saves this Product as Json to @p path or to @link{filePath} by default.
     * The method guarantees that an error does not destroy the already existing json file.
     * @returns @c false on error
     **/
    bool save(const QString &path = {}) const;

    /**
     * @returns whether change tracking is enabled
     **/
    bool isChangeTracking() const
    {
        return m_changeTracking;
    }
    /**
     * Enables or disables change tracking.
     * While change tracking is enabled every change is added to the ChangeTracker
     * and all changes can be retrieved as json array through @link{changes}.
     **/
    void setChangeTrackingEnabled(bool set);

    /**
     * @returns all tracked changes.
     **/
    QJsonArray changes() const;

    /**
     * Loads the Product from the file specified by @p path and returns a new created Product.
     * In case the parsing of the JSON fails, @c null is returned.
     * The @p parent is set as parent for the newly created Product.
     *
     * @returns New Product on success, @c null on failure
     **/
    static Product *fromJson(const QString &path, QObject *parent = nullptr);

    /**
     * Loads the Product from the provided Json @p object and returns a new created Product.
     * In case the @p object does not contain a valid Product JSON structure, @c null is returned.
     *
     * The @p parent is set as parent for the newly created Product.
     *
     * In case an element in the JSON structure is missing or has an invalid value, the
     * default value will be returned. The only required field is the uuid.
     *
     * @returns New Product on success, @c null on failure
     **/
    static Product *fromJson(const QJsonObject &object, QObject *parent = nullptr);

    /**
     * Imports the Product from legacy xml file provided at @p path.
     *
     * Only the structure (Product, SeamSeries, Seam, SeamInterval) are imported.
     * SumErrors, HardwareParameters and FilterParameters are not imported.
     *
     * @returns Imported Product on success, @c null on failure.
     *
     **/
    static Product *fromXml(const QString &path, QObject *parent = nullptr);

    ReferenceCurveData* referenceCurveData(const QUuid& id);
    void setReferenceCurveData(const QUuid& id, const std::forward_list<QVector2D>& samples, std::size_t count);
    void setReferenceCurveData(const QUuid& id, const std::vector<QVector2D>& samples);
    void copyReferenceCurveData(Product* sourceProduct, ReferenceCurve* sourceCurve, ReferenceCurve* newCurve);

    /**
     * Returns the ReferenceCurve of all the Seams of this Product.
     **/
    std::vector<ReferenceCurve*> allReferenceCurves() const;

    bool saveReferenceCurves(const QString &pathDir = {});

    friend QDataStream &operator<<(QDataStream &out, Product* product);
    friend QDataStream &operator>>(QDataStream &in, Product* product);

Q_SIGNALS:
    void nameChanged();
    void typeChanged();
    void endlessChanged();
    void triggerSourceChanged();
    void triggerModeChanged();
    void startPositionYAxisChanged();
    void hardwareParametersChanged();
    void defaultProductChanged();
    void lengthUnitChanged();
    void seamsChanged();
    void filePathChanged();
    void assemblyImageChanged();
    void seamSeriesChanged();
    void signalyQualityColorMapChanged();
    void errorLevelColorMapChanged();
    void qualityNormChanged();
    void laserControlPresetChanged();
    void referenceCurveDataChanged();
    void referenceCurveStorageDirChanged();
    void lwmTriggerSignalTypeChanged();
    void lwmTriggerSignalThresholdChanged();

private:
    void addChange(ChangeTracker &&change);
    void addSeamSeries(SeamSeries *seamSeries);
    void removeUnusedFilterParameterSets();
    void loadReferenceCurves();
    ReferenceCurveData* findOrCreateReferenceCurveData(const QUuid& id);
    bool ensureFilterParameterSetLoaded(const QJsonObject& object);

    QUuid m_uuid;
    QString m_name;
    int m_type = 0;
    bool m_endless = false;
    TriggerSource m_triggerSource = TriggerSource::Software;
    TriggerMode m_triggerMode = TriggerMode::Burst;
    int m_startPositionYAxis = 0;
    std::vector<ProductError*> m_overlyingErrors;
    std::vector<SeamSeries*> m_seamSeries;
    std::vector<ReferenceCurveData*> m_referenceCurveData;
    ParameterSet *m_hardwareParameters = nullptr;
    bool m_default = false;
    std::vector<ParameterSet*> m_filterParameterSets;
    std::vector<QUuid> m_discardedFilterParameterSets;
    QString m_filePath;
    QString m_referenceCurveStorageDir = QString();
    LengthUnit m_lengthUnit = LengthUnit::Millimeter;
    QString m_assemblyImage;
    QUuid m_qualityNorm;
    QUuid m_laserControlPreset;
    bool m_changeTracking = false;
    int m_lwmTriggerSignalType = -1;
    double m_lwmTriggerSignalThreshold = 0.0;
    std::vector<ChangeTracker> m_changeTracker;
    precitec::gui::components::plotter::ColorMap* m_signalyQualityColorMap = nullptr;
    QMetaObject::Connection m_signalyQualityColorMapDestroyed;
    precitec::gui::components::plotter::ColorMap* m_errorLevelColorMap = nullptr;
    QMetaObject::Connection m_errorLevelColorMapDestroyed;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::Product*)
