#pragma once

#include "overlyingError.h"

class TestSeamSeriesError;

namespace precitec
{
namespace interface
{

class FilterParameter;

}
namespace storage
{

class AbstractMeasureTask;
enum class CopyMode;

class SeamSeriesError : public OverlyingError
{
    Q_OBJECT

public:
    SeamSeriesError(QObject* parent = nullptr);
    SeamSeriesError(QUuid uuid, QObject* parent = nullptr);
    ~SeamSeriesError() override;

    /**
     * Duplicates this SeamSeriesError and assigns a new QUuid to the duplicated SeamSeriesError.
     * The @p parent is used as the measureTask of the duplicated SeamSeriesError.
     * @param mode selects whether the duplicate gets a new, random Uuid or the same Uuid as this object.
     * @param parent The parent for the new duplicated SeamSeriesError.
     **/
    SeamSeriesError *duplicate(CopyMode mode, AbstractMeasureTask *parent) const;

    AbstractMeasureTask *measureTask() const
    {
        return m_measureTask;
    }
    void setMeasureTask(AbstractMeasureTask *task);

    std::vector<std::shared_ptr<precitec::interface::FilterParameter> > toParameterList() const;

    static SeamSeriesError *fromJson(const QJsonObject &object, AbstractMeasureTask *parent);

    bool isChangeTracking() override;

Q_SIGNALS:
    void measureTaskChanged();

private:
    int getIntValue(const QString &name) const;
    std::string getStringValue(const QString& n) const;

    AbstractMeasureTask *m_measureTask = nullptr;

    friend TestSeamSeriesError;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::SeamSeriesError*)

