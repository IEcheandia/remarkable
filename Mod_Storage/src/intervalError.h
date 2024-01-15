#pragma once

#include "simpleError.h"

#include <QObject>
#include <QUuid>
#include <memory>

class TestIntervalError;

namespace precitec
{
namespace interface
{

class FilterParameter;

}
namespace storage
{

class ChangeTracker;
class SeamInterval;
class LevelConfig;
enum class CopyMode;

class IntervalError : public SimpleError
{
    Q_OBJECT

    Q_PROPERTY(double lowestMin READ lowestMin NOTIFY minChanged)

    Q_PROPERTY(double highestMax READ highestMax NOTIFY maxChanged)

    Q_PROPERTY(bool showSecondThreshold READ showSecondThreshold CONSTANT)

public:
    IntervalError(QObject* parent = nullptr);
    IntervalError(QUuid uuid, QObject* parent = nullptr);
    ~IntervalError() override;

    /**
     * Duplicates this IntervalError.
     * The @p parent is used as the measureTask of the duplicated IntervalError.
     * @param mode selects whether the duplicate gets a new, random Uuid or the same Uuid as this object.
     * @param parent The parent for the new duplicated IntervalError.
     **/
    IntervalError* duplicate(CopyMode mode, Seam* parent) const;

    double threshold(uint index);
    void setThreshold(uint index, double threshold);

    double secondThreshold(uint index);
    void setSecondThreshold(uint index, double secondThreshold);

    double min(uint index);
    void setMin(uint index, double min);

    double max(uint index);
    void setMax(uint index, double max);

    void initFromAttributes(const AttributeModel *attributeModel) override;

    QJsonObject toJson() const override;
    static IntervalError *fromJson(const QJsonObject &object, AbstractMeasureTask *parent);

    std::vector<std::shared_ptr<precitec::interface::FilterParameter> > toParameterList() const;

    void addInterval(SeamInterval *interval);
    void removeInterval(SeamInterval *interval);

    double lowestMin() const;
    double highestMax() const;
    bool showSecondThreshold();

Q_SIGNALS:
    void thresholdChanged();
    void secondThresholdChanged();
    void minChanged();
    void maxChanged();

protected:
    double getDoubleValue(const QString &name, int level) const;
    std::string getStringValue(const QString& name) const override;

private:
    void updateLowerBounds();
    void updateUpperBounds();

    std::map<QUuid, QUuid> m_errorIds;
    std::vector<LevelConfig*> m_levels;

    friend TestIntervalError;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::IntervalError*)
