#pragma once
#include "abstractMeasureTask.h"

#include <QObject>
#include <QUuid>

#include <vector>

class QXmlStreamReader;

namespace precitec
{
namespace storage
{
class Product;
class Seam;
class SeamError;
class SeamSeriesError;
class IntervalError;
class ReferenceCurve;

/**
 * @brief Represents one SeamSeries in a Product.
 *
 **/
class SeamSeries : public AbstractMeasureTask
{
    Q_OBJECT
    /**
     * All Seams of this SeamSeries. This property is intended for interchange with QML and
     * thus returns a QVariantList instead of std::vector<Seam*>. Do not use from C++ side, instead
     * iterate over seamSeries.
     **/
    Q_PROPERTY(QVariantList seams READ allSeams NOTIFY seamsChanged)

public:
    /**
     * Constructs a SeamSeries with the given @p uuid and the given @p parentProduct.
     * The @p parentProduct is also set as the QObject parent for this SeamSeries
     **/
    explicit SeamSeries(const QUuid &uuid, Product *parentProduct);
    ~SeamSeries() override;

    /**
     * Duplicates this SeamSeries.
     * @param mode selects whether the duplicate gets a new, random Uuid or the same Uuid as this object.
     * @param parent The parent for the new duplicated SeamSeries.
     **/
    SeamSeries *duplicate(CopyMode mode, Product *parent = nullptr) const;

    Product *product() const override
    {
        return m_product;
    }

    /**
     * @returns All @link{Seam}s in this SeamSeries
     **/
    const std::vector<Seam*> &seams() const
    {
        return m_seams;
    }

    /**
     * @returns the previous Seam from reference @p seam or @c nullptr if @p seam is the first Seam.
     **/
    Seam *previousSeam(Seam *seam) const;

    /**
     * @returns the next Seam from reference @p seam or @c nullptr if @p seam is the last Seam.
     **/
    Seam *nextSeam(Seam *seam) const;

    /**
     * @returns all Seams from this SeamSeries. This method is intended for interchange with QML and
     * thus returns a QVariantList instead of std::vector<Seam*>. Do not use from C++ side, instead
     * iterate over seamSeries.
     **/
    QVariantList allSeams() const;

    /**
     * @returns the Seam in this SeamSeries with the given @p id, if there is no such Seam @c null is returned.
     **/
    Seam *findSeam(const QUuid &id) const;

    /**
     * @returns the Seam in this SeamSeries with the given @p number, if there is no such Seam @c null is returned.
     **/
    Seam *findSeam(quint32 number) const;

    /**
     * Removes the @p seam from this SeamSeries and deletes @p seam if it was a Seam for this SeamSeries.
     **/
    Q_INVOKABLE void destroySeam(precitec::storage::Seam *seam);

    /**
     * Creates a new Seam and adds it to this SeamSeries.
     * The number of the new Seam is an increment to the current highest Seam number in this SeamSeries.
     * @returns new created Seam.
     **/
    Seam *createSeam();

    /**
     * Creates a new Seam as a copy of @p seamToCopy and adds it to this SeamSeries.
     * The number of the new Seam is an increment to the current highest Seam number in this SeamSeries.
     * Uuids may be recreated, all other information is taken from the @p seamToCopy.
     * @param mode selects whether the duplicate gets a new, random Uuid or the same Uuid as their source.
     * @returns new created Seam.
     **/
    Seam *createSeamCopy(CopyMode mode, Seam *seamToCopy);

    /**
     * Creates a link to @p seamToLink with the @p label and adds it to the Seams.
     * May return @c nullptr.
     **/
    Seam *createSeamLink(Seam *seamToLink, const QString &label);

    /**
     * @returns the ParameterSet with the given @p id, evaluates the hardware parameters
     * of this SeamSeries and all contained Seams and their SeamIntervals.
     * If there is no such ParameterSet @c null is returned.
     **/
    ParameterSet *findHardwareParameterSet(const QUuid &id) const;

    QJsonObject toJson() const override;

    bool isChangeTracking() const override;

    QJsonArray changes() const override;

    /**
     * Returns the SeamErrors of this SeamSeries.
     **/
    std::vector<SeamError*> allSeamErrors() const;

    /**
     * Returns the IntervalErrors of all of this SeamSeries' SeamIntervals.
     **/
    std::vector<IntervalError*> allIntervalErrors() const;
    int intervalErrorCount();

    const std::vector<SeamSeriesError*> overlyingErrors() const
    {
        return m_overlyingErrors;
    }

    SeamSeriesError *addOverlyingError(const QUuid &variantId);
    void removeOverlyingError(int index);

    /**
     * Returns the ReferenceCurves of this SeamSeries and its Seams.
     **/
    std::vector<ReferenceCurve*> allReferenceCurves() const;

    /**
     * Creates a new SeamSeries from the provided json @p object.
     * If the @p object is empty @c null is returned.
     *
     * The uuid is taken from the @p object if provided. If no uuid is
     * provided a new uuid gets generated.
     *
     * The @p parent is set as the parent Product for the created SeamSeries.
     *
     * @returns New created SeamSeries or @c null if @p object is empty.
     **/
    static SeamSeries *fromJson(const QJsonObject &object, Product *parent);

    /**
     * Imports SeamSeries from @p xml. The QXmlStreamReader must be at a
     * start element for a SeamSeries element.
     *
     * @returns the imported SeamSeries on succes or @c null on error.
     **/
    static SeamSeries *fromXml(QXmlStreamReader &xml, Product *parent);

Q_SIGNALS:
    /**
     * Emitted whenever the Seams change.
     **/
    void seamsChanged();


private:
    int maxSeamNumber() const;
    /**
     * Creates copies of all linked seams of all seams and stores them in m_seams.
     **/
    void copyLinkedSeamsToSeams();
    Product *m_product;
    std::vector<Seam*> m_seams;
    std::vector<SeamSeriesError*> m_overlyingErrors;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::SeamSeries*)
