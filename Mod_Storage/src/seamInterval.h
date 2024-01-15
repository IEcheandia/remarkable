#pragma once
#include "abstractMeasureTask.h"

#include <QColor>

class QUuid;
class QXmlStreamReader;

namespace precitec
{
namespace storage
{

class Seam;
class Product;
enum class CopyMode;

/**
 * @brief Represents one SeamInterval in a Seam.
 **/
class SeamInterval : public AbstractMeasureTask
{
    Q_OBJECT
    /**
     * The Seam this SeamInterval belongs to.
     **/
    Q_PROPERTY(precitec::storage::Seam *seam READ seam CONSTANT)
    /**
     * Length of SeamInterval (internally [um], user level [mm]), negative = infinite
     **/
    Q_PROPERTY(qint64 length READ length WRITE setLength NOTIFY lengthChanged)
    /**
     * The Level of this SeamInterval
     **/
    Q_PROPERTY(int level READ level WRITE setLevel NOTIFY levelChanged)
    /**
     *  The color of this interval, determined by its @link{level}
     **/
    Q_PROPERTY(QColor color READ color NOTIFY levelChanged)
public:
    /**
     * Constructs a SeamInterval with the given @p uuid and the given @p parentSeam.
     * The @p parentSeam is also set as the QObject parent for this SeamInterval
     **/
    explicit SeamInterval(const QUuid &uuid, Seam *parentSeam);
    ~SeamInterval() override;

    /**
     * Duplicates this SeamInterval.
     * @param mode selects whether the duplicate gets a new, random Uuid or the same Uuid as this object.
     * @param parent The parent for the new duplicated SeamInterval.
     **/
    SeamInterval *duplicate(CopyMode mode, Seam *parent = nullptr) const;

    Seam *seam() const
    {
        return m_seam;
    }

    qint64 length() const
    {
        return m_length;
    }
    void setLength(qint64 length);

    int level() const
    {
        return m_level;
    }
    void setLevel(int level);

    QColor color() const;

    QJsonObject toJson() const override;

    bool isChangeTracking() const override;

    QUuid graph() const override;
    const GraphReference& graphReference() const override;
    QUuid graphParamSet() const override;
    const std::vector<QUuid> &subGraphs() const override;
    bool usesSubGraph() const override;
    ParameterSet *hardwareParameters() const override;

    /**
     * Creates a new SeamInterval from the provided json @p object.
     * If the @p object is empty @c null is returned.
     *
     * The uuid is taken from the @p object if provided. If no uuid is
     * provided a new uuid gets generated.
     *
     * The @p parent is set as the parent Seam for the created SeamInterval.
     *
     * If there are any valid hw or filter parameters, they are set to
     * the parent @p seam
     *
     * @returns New created SeamInterval or @c null if @p object is empty.
     **/
    static SeamInterval *fromJson(const QJsonObject &object, Seam *parent);

    /**
     * Imports SeamInterval from @p xml. The QXmlStreamReader must be at a
     * start element for a SeamInterval element.
     *
     * @returns the imported SeamInterval on succes or @c null on error.
     **/
    static SeamInterval *fromXml(QXmlStreamReader &xml, Seam *parent);

    Product *product() const override;

Q_SIGNALS:
    void lengthChanged();
    void levelChanged();

private:
    qint64 m_length = 100000;
    int m_level = 0;

    Seam* m_seam;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::SeamInterval*)
