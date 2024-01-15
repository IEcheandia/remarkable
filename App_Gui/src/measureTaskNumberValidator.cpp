#include "measureTaskNumberValidator.h"

#include "abstractMeasureTask.h"

#include "seam.h"
#include "seamSeries.h"
#include "product.h"

namespace precitec
{

using storage::Seam;
using storage::SeamSeries;

namespace gui
{

MeasureTaskNumberValidator::MeasureTaskNumberValidator(QObject *parent)
    : QValidator(parent)
{
}

MeasureTaskNumberValidator::~MeasureTaskNumberValidator() = default;

void MeasureTaskNumberValidator::setMeasureTask(precitec::storage::AbstractMeasureTask *task)
{
    if (m_measureTask == task)
    {
        return;
    }
    disconnect(m_destroyConnection);
    m_destroyConnection = {};
    m_measureTask = task;
    if (m_measureTask)
    {
        m_destroyConnection = connect(m_measureTask, &QObject::destroyed, this, std::bind(&MeasureTaskNumberValidator::setMeasureTask, this, nullptr));
    }
    emit measureTaskChanged();
}

QValidator::State MeasureTaskNumberValidator::validate(QString& input, int& pos) const
{
    Q_UNUSED(pos)
    if (!m_measureTask)
    {
        return QValidator::Invalid;
    }
    if (input.isEmpty())
    {
        return QValidator::Intermediate;
    }
    bool ok = false;
    const int visualNumber = input.toInt(&ok);
    if (!ok)
    {
        return QValidator::Invalid;
    }
    const auto number = m_measureTask->numberFromVisualNumber(visualNumber);
    if (number > 255)
    {
        return QValidator::Intermediate;
    }
    if (auto *seam = dynamic_cast<Seam*>(m_measureTask))
    {
        const auto &seams = seam->seamSeries()->seams();
        const auto it = std::find_if(seams.begin(), seams.end(), [number] (auto s) { return s->number() == number; });
        if (it == seams.end() || *it == seam)
        {
            return QValidator::Acceptable;
        }
    }
    if (auto *seamSeries = dynamic_cast<SeamSeries*>(m_measureTask))
    {
        const auto &series = seamSeries->product()->seamSeries();
        const auto it = std::find_if(series.begin(), series.end(), [number] (auto s) { return s->number() == number; });
        if (it == series.end() || *it == seamSeries)
        {
            return QValidator::Acceptable;
        }
    }
    return QValidator::Intermediate;
}

}
}
