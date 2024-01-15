#pragma once

#include <QValidator>

namespace precitec
{

namespace storage
{
class AbstractMeasureTask;
}

namespace gui
{

/**
 * Validator for the number of an AbstractMeasureTask (either Seam or SeamSeries).
 * The provided input for the validator has to be a visual number and is converted to
 * an internal number.
 *
 * An already used number results in invalid, a not used number in acceptable. Everything
 * else is intermediate.
 **/
class MeasureTaskNumberValidator : public QValidator
{
    /**
     * The measure task to validate, either a Seam or SeamSeries.
     **/
    Q_PROPERTY(precitec::storage::AbstractMeasureTask *measureTask READ measureTask WRITE setMeasureTask NOTIFY measureTaskChanged)
    Q_OBJECT
public:
    explicit MeasureTaskNumberValidator(QObject *parent = nullptr);
    ~MeasureTaskNumberValidator() override;

    void setMeasureTask(precitec::storage::AbstractMeasureTask *task);
    precitec::storage::AbstractMeasureTask *measureTask() const
    {
        return m_measureTask;
    }

    QValidator::State validate(QString &input, int &pos) const override;

Q_SIGNALS:
    void measureTaskChanged();

private:
    precitec::storage::AbstractMeasureTask *m_measureTask = nullptr;
    QMetaObject::Connection m_destroyConnection;

};

}
}
