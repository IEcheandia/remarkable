#include "staticErrorConfigController.h"
#include "seam.h"
#include "seamError.h"
#include "precitec/dataSet.h"
#include "precitec/infiniteSet.h"

using precitec::storage::SeamError;
using precitec::gui::components::plotter::DataSet;
using precitec::gui::components::plotter::InfiniteSet;

namespace precitec
{
namespace gui
{

StaticErrorConfigController::StaticErrorConfigController(QObject *parent)
    : QObject(parent)
    , m_visualReference(new DataSet(this))
    , m_lowerBoundary(new InfiniteSet(this))
    , m_upperBoundary(new InfiniteSet(this))
    , m_shiftedLowerBoundary(new InfiniteSet(this))
    , m_shiftedUpperBoundary(new InfiniteSet(this))
{
    m_visualReference->setDrawingOrder(DataSet::DrawingOrder::OnTop);

    m_lowerBoundary->setColor(QColor("magenta"));
    m_lowerBoundary->setName(QStringLiteral("Min"));
    m_upperBoundary->setColor(QColor("magenta"));
    m_upperBoundary->setName(QStringLiteral("Max"));
    m_shiftedLowerBoundary->setColor(QColor("magenta"));
    m_shiftedLowerBoundary->setName(QStringLiteral("Min Shift"));
    m_shiftedUpperBoundary->setColor(QColor("magenta"));
    m_shiftedUpperBoundary->setName(QStringLiteral("Max Shift"));

    connect(this, &StaticErrorConfigController::seamErrorChanged, this, &StaticErrorConfigController::updateLowerBoundary);
    connect(this, &StaticErrorConfigController::seamErrorChanged, this, &StaticErrorConfigController::updateUpperBoundary);
}

StaticErrorConfigController::~StaticErrorConfigController() = default;

void StaticErrorConfigController::setSeamError(SeamError *seamError)
{
    if (m_seamError == seamError)
    {
        return;
    }
    if (m_seamError)
    {
        disconnect(m_seamError, &SeamError::minChanged, this, &StaticErrorConfigController::updateLowerBoundary);
        disconnect(m_seamError, &SeamError::maxChanged, this, &StaticErrorConfigController::updateUpperBoundary);
        disconnect(m_seamError, &SeamError::shiftChanged, this, &StaticErrorConfigController::updateLowerBoundary);
        disconnect(m_seamError, &SeamError::shiftChanged, this, &StaticErrorConfigController::updateUpperBoundary);
        disconnect(m_destroyedConnection);
        disconnect(m_resultValueChangedConnection);
    }
    m_seamError = seamError;
    if (m_seamError)
    {
        m_destroyedConnection = connect(m_seamError, &QObject::destroyed, this, std::bind(&StaticErrorConfigController::setSeamError, this, nullptr));
        m_resultValueChangedConnection = connect(m_seamError, &SeamError::resultValueChanged, this, std::bind(&StaticErrorConfigController::setVisualReference, this, nullptr));
        connect(m_seamError, &SeamError::minChanged, this, &StaticErrorConfigController::updateLowerBoundary);
        connect(m_seamError, &SeamError::maxChanged, this, &StaticErrorConfigController::updateUpperBoundary);
        connect(m_seamError, &SeamError::shiftChanged, this, &StaticErrorConfigController::updateLowerBoundary);
        connect(m_seamError, &SeamError::shiftChanged, this, &StaticErrorConfigController::updateUpperBoundary);
    } else
    {
        m_destroyedConnection = {};
        m_resultValueChangedConnection = {};
        setVisualReference(nullptr);
    }

    emit seamErrorChanged();
}

void StaticErrorConfigController::setVisualReference(DataSet *ds)
{
    m_visualReference->clear();
    m_visualReference->setColor("white");
    m_visualReference->setName(QStringLiteral(""));
    if (ds)
    {
        m_visualReference->addSamples(ds->samples());
        m_visualReference->setColor(ds->color());
        m_visualReference->setName(ds->name());
    }
    emit visualReferenceChanged();
}

void StaticErrorConfigController::updateLowerBoundary()
{
    if (!m_seamError)
    {
        m_lowerBoundary->setEnabled(false);
        m_shiftedLowerBoundary->setEnabled(false);
        return;
    }

    m_lowerBoundary->setEnabled(true);
    m_lowerBoundary->setValue(m_seamError->min());
    m_shiftedLowerBoundary->setValue(m_seamError->min() + m_seamError->shift());

    if (qFuzzyIsNull(m_seamError->shift()))
    {
        m_lowerBoundary->setAlpha(255);
        m_shiftedLowerBoundary->setEnabled(false);
    } else
    {
        m_lowerBoundary->setAlpha(50);
        m_shiftedLowerBoundary->setEnabled(true);
    }
}

void StaticErrorConfigController::updateUpperBoundary()
{
    if (!m_seamError)
    {
        m_upperBoundary->setEnabled(false);
        m_shiftedUpperBoundary->setEnabled(false);
        return;
    }

    m_upperBoundary->setEnabled(true);
    m_upperBoundary->setValue(m_seamError->max());
    m_shiftedUpperBoundary->setValue(m_seamError->max() + m_seamError->shift());

    if (qFuzzyIsNull(m_seamError->shift()))
    {
        m_upperBoundary->setAlpha(255);
        m_shiftedUpperBoundary->setEnabled(false);
    } else
    {
        m_upperBoundary->setAlpha(50);
        m_shiftedUpperBoundary->setEnabled(true);
    }
}

void StaticErrorConfigController::setMinFromReference()
{
    if (m_visualReference->isEmpty() || !m_seamError)
    {
        return;
    }
    m_seamError->setMin(m_visualReference->minY());
}

void StaticErrorConfigController::setMaxFromReference()
{
    if (m_visualReference->isEmpty() || !m_seamError)
    {
        return;
    }
    m_seamError->setMax(m_visualReference->maxY());
}

}
}



