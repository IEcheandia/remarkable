#include "referenceErrorConfigController.h"
#include "product.h"
#include "seam.h"
#include "seamError.h"
#include "referenceCurve.h"
#include "referenceCurveData.h"
#include "guiConfiguration.h"
#include "precitec/dataSet.h"

using precitec::storage::Product;
using precitec::storage::SeamError;
using precitec::gui::components::plotter::DataSet;

namespace precitec
{
namespace gui
{

ReferenceErrorConfigController::ReferenceErrorConfigController(QObject *parent)
    : QObject(parent)
    , m_visualReference(new DataSet(this))
    , m_lowerShadow(new DataSet(this))
    , m_upperShadow(new DataSet(this))
    , m_lowerReference(new DataSet(this))
    , m_middleReference(new DataSet(this))
    , m_upperReference(new DataSet(this))
{
    const auto& oversamplingRate = GuiConfiguration::instance()->oversamplingRate();;
    m_visualReference->setDrawingOrder(DataSet::DrawingOrder::OnTop);
    m_visualReference->setMaxElements(oversamplingRate * m_visualReference->maxElements());

    m_lowerShadow->setColor(QColor("darkturquoise"));
    m_lowerShadow->setName(QStringLiteral("Min"));
    m_lowerShadow->setMaxElements(oversamplingRate * m_lowerShadow->maxElements());
    m_upperShadow->setColor(QColor("darkturquoise"));
    m_upperShadow->setName(QStringLiteral("Max"));
    m_upperShadow->setMaxElements(oversamplingRate * m_upperShadow->maxElements());
    m_lowerReference->setColor(QColor("magenta"));
    m_lowerReference->setName(QStringLiteral("Lower Reference"));
    m_lowerReference->setMaxElements(oversamplingRate * m_lowerReference->maxElements());
    m_middleReference->setColor(QColor("darkmagenta"));
    m_middleReference->setName(QStringLiteral("Middle Reference"));
    m_middleReference->setMaxElements(oversamplingRate * m_middleReference->maxElements());
    m_upperReference->setColor(QColor("magenta"));
    m_upperReference->setName(QStringLiteral("Upper Reference"));
    m_upperReference->setMaxElements(oversamplingRate * m_upperReference->maxElements());

    connect(this, &ReferenceErrorConfigController::currentProductChanged, this, &ReferenceErrorConfigController::loadReferenceCurve);

    connect(this, &ReferenceErrorConfigController::seamErrorChanged, this, &ReferenceErrorConfigController::updateLowerBoundary);
    connect(this, &ReferenceErrorConfigController::seamErrorChanged, this, &ReferenceErrorConfigController::updateUpperBoundary);
    connect(this, &ReferenceErrorConfigController::seamErrorChanged, this, &ReferenceErrorConfigController::loadReferenceCurve);
    connect(this, &ReferenceErrorConfigController::seamErrorChanged, this, &ReferenceErrorConfigController::updateReferenceCurve);

    connect(this, &ReferenceErrorConfigController::referenceChanged, this, &ReferenceErrorConfigController::updateLowerBoundary);
    connect(this, &ReferenceErrorConfigController::referenceChanged, this, &ReferenceErrorConfigController::updateUpperBoundary);
    connect(this, &ReferenceErrorConfigController::referenceChanged, this, &ReferenceErrorConfigController::updateReferenceCurve);
}

ReferenceErrorConfigController::~ReferenceErrorConfigController() = default;

void ReferenceErrorConfigController::setSeamError(SeamError *seamError)
{
    if (m_seamError == seamError)
    {
        return;
    }
    if (m_seamError)
    {
        disconnect(m_seamError, &SeamError::minChanged, this, &ReferenceErrorConfigController::updateLowerBoundary);
        disconnect(m_seamError, &SeamError::maxChanged, this, &ReferenceErrorConfigController::updateUpperBoundary);
        disconnect(m_seamError, &SeamError::envelopeChanged, this, &ReferenceErrorConfigController::loadReferenceCurve);
        disconnect(m_seamError, &SeamError::useMiddleCurveAsReferenceChanged, this, &ReferenceErrorConfigController::updateReferenceCurve);
        disconnect(m_seamError, &SeamError::useMiddleCurveAsReferenceChanged, this, &ReferenceErrorConfigController::updateLowerBoundary);
        disconnect(m_seamError, &SeamError::useMiddleCurveAsReferenceChanged, this, &ReferenceErrorConfigController::updateUpperBoundary);
        disconnect(m_destroyedConnection);
        disconnect(m_resultValueChangedConnection);
    }
    m_seamError = seamError;
    if (m_seamError)
    {
        m_destroyedConnection = connect(m_seamError, &QObject::destroyed, this, std::bind(&ReferenceErrorConfigController::setSeamError, this, nullptr));
        m_resultValueChangedConnection = connect(m_seamError, &SeamError::resultValueChanged, this, std::bind(&ReferenceErrorConfigController::setVisualReference, this, nullptr));
        connect(m_seamError, &SeamError::minChanged, this, &ReferenceErrorConfigController::updateLowerBoundary);
        connect(m_seamError, &SeamError::maxChanged, this, &ReferenceErrorConfigController::updateUpperBoundary);
        connect(m_seamError, &SeamError::envelopeChanged, this, &ReferenceErrorConfigController::loadReferenceCurve);
        connect(m_seamError, &SeamError::useMiddleCurveAsReferenceChanged, this, &ReferenceErrorConfigController::updateReferenceCurve);
        connect(m_seamError, &SeamError::useMiddleCurveAsReferenceChanged, this, &ReferenceErrorConfigController::updateLowerBoundary);
        connect(m_seamError, &SeamError::useMiddleCurveAsReferenceChanged, this, &ReferenceErrorConfigController::updateUpperBoundary);
    } else
    {
        m_destroyedConnection = {};
        m_resultValueChangedConnection = {};
        setVisualReference(nullptr);
    }

    emit seamErrorChanged();
}

void ReferenceErrorConfigController::setVisualReference(DataSet *ds)
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

void ReferenceErrorConfigController::updateLowerBoundary()
{
    m_lowerShadow->clear();

    if (!m_seamError)
    {
        return;
    }

    if (m_seamError->useMiddleCurveAsReference())
    {
        m_lowerShadow->addSamples(m_middleReference->samples());
    } else
    {
        m_lowerShadow->addSamples(m_lowerReference->samples());
    }
    m_lowerShadow->setOffset({float(m_offset), float(-m_seamError->min())});
}

void ReferenceErrorConfigController::updateUpperBoundary()
{
    m_upperShadow->clear();

    if (!m_seamError)
    {
        return;
    }

    if (m_seamError->useMiddleCurveAsReference())
    {
        m_upperShadow->addSamples(m_middleReference->samples());
    } else
    {
        m_upperShadow->addSamples(m_upperReference->samples());
    }
    m_upperShadow->setOffset({float(m_offset), float(m_seamError->max())});
}

void ReferenceErrorConfigController::setCurrentProduct(Product* product)
{
    if (m_currentProduct == product)
    {
        return;
    }

    if (m_currentProduct)
    {
        disconnect(m_productDestroyConnection);
    }

    m_currentProduct = product;

    if (m_currentProduct)
    {
        m_productDestroyConnection = connect(m_currentProduct, &QObject::destroyed, this, std::bind(&ReferenceErrorConfigController::setCurrentProduct, this, nullptr));
    } else
    {
        m_productDestroyConnection = {};
    }

    emit currentProductChanged();
}

void ReferenceErrorConfigController::loadReferenceCurve()
{
    m_lowerReference->clear();
    m_middleReference->clear();
    m_upperReference->clear();

    if (!m_seamError || !m_seamError->measureTask() || !m_currentProduct)
    {
        return;
    }

    if (auto reference = m_seamError->measureTask()->findReferenceCurve(m_seamError->envelope()))
    {
        if (auto upperData = m_currentProduct->referenceCurveData(reference->upper()))
        {
            m_upperReference->addSamples(upperData->samples());
        }
        if (auto middleData = m_currentProduct->referenceCurveData(reference->middle()))
        {
            m_middleReference->addSamples(middleData->samples());
        }
        if (auto lowerData = m_currentProduct->referenceCurveData(reference->lower()))
        {
            m_lowerReference->addSamples(lowerData->samples());
        }
    }

    emit referenceChanged();
}

void ReferenceErrorConfigController::updateReferenceCurve()
{
    if (!m_seamError)
    {
        return;
    }
    if (m_seamError->useMiddleCurveAsReference())
    {
        m_lowerReference->setEnabled(false);
        m_middleReference->setEnabled(true);
        m_upperReference->setEnabled(false);
    } else
    {
        m_lowerReference->setEnabled(true);
        m_middleReference->setEnabled(false);
        m_upperReference->setEnabled(true);
    }
}

void ReferenceErrorConfigController::setOffset(int offset)
{
    if (m_offset == offset)
    {
        return;
    }
    m_offset = offset;

    m_lowerShadow->setOffset({float(m_offset), m_lowerShadow->offset().y()});
    m_upperShadow->setOffset({float(m_offset), m_upperShadow->offset().y()});
    m_lowerReference->setOffset({float(m_offset), m_lowerReference->offset().y()});
    m_middleReference->setOffset({float(m_offset), m_middleReference->offset().y()});
    m_upperReference->setOffset({float(m_offset), m_upperReference->offset().y()});

    emit offsetChanged();
}

}
}




