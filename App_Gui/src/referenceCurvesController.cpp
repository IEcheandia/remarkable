#include "referenceCurvesController.h"
#include "referenceCurve.h"
#include "referenceCurveData.h"
#include "product.h"
#include "guiConfiguration.h"
#include "precitec/dataSet.h"

using precitec::storage::Product;
using precitec::storage::ReferenceCurve;
using precitec::gui::components::plotter::DataSet;

namespace precitec
{
namespace gui
{

ReferenceCurvesController::ReferenceCurvesController(QObject* parent)
    : QObject(parent)
    , m_lower(new DataSet{this})
    , m_middle(new DataSet{this})
    , m_upper(new DataSet{this})
{
    const auto& oversamplingRate = GuiConfiguration::instance()->oversamplingRate();

    m_lower->setDrawingMode(DataSet::DrawingMode::Line);
    m_lower->setColor(Qt::magenta);
    m_lower->setName(QStringLiteral("Lower Boundary"));
    m_lower->setMaxElements(oversamplingRate * m_lower->maxElements());

    m_middle->setDrawingOrder(DataSet::DrawingOrder::OnTop);
    m_middle->setColor(Qt::darkMagenta);
    m_middle->setName(QStringLiteral("Reference Curve"));
    m_middle->setMaxElements(oversamplingRate * m_middle->maxElements());

    m_upper->setDrawingMode(DataSet::DrawingMode::Line);
    m_upper->setColor(Qt::magenta);
    m_upper->setName(QStringLiteral("Upper Boundary"));
    m_upper->setMaxElements(oversamplingRate * m_upper->maxElements());

    connect(this, &ReferenceCurvesController::currentProductChanged, this, &ReferenceCurvesController::setSamples);
    connect(this, &ReferenceCurvesController::referenceCurveChanged, this, &ReferenceCurvesController::setSamples);
}

ReferenceCurvesController::~ReferenceCurvesController() = default;

void ReferenceCurvesController::setCurrentProduct(Product* product)
{
    if (m_currentProduct == product)
    {
        return;
    }

    if (m_currentProduct)
    {
        disconnect(m_productDestroyConnection);
        disconnect(m_currentProduct, &Product::referenceCurveDataChanged, this, &ReferenceCurvesController::setSamples);
    }

    m_currentProduct = product;

    if (m_currentProduct)
    {
        m_productDestroyConnection = connect(m_currentProduct, &QObject::destroyed, this, std::bind(&ReferenceCurvesController::setCurrentProduct, this, nullptr));
        connect(m_currentProduct, &Product::referenceCurveDataChanged, this, &ReferenceCurvesController::setSamples);
    } else
    {
        m_productDestroyConnection = {};
    }

    emit currentProductChanged();
}

void ReferenceCurvesController::setReferenceCurve(ReferenceCurve* referenceCurve)
{
    if (m_referenceCurve == referenceCurve)
    {
        return;
    }

    if (m_referenceCurve)
    {
        disconnect(m_curveDestroyConnection);
    }

    m_referenceCurve = referenceCurve;

    if (m_referenceCurve)
    {
        m_curveDestroyConnection = connect(m_referenceCurve, &QObject::destroyed, this, std::bind(&ReferenceCurvesController::setReferenceCurve, this, nullptr));
    } else
    {
        m_curveDestroyConnection = {};
    }

    emit referenceCurveChanged();
}

void ReferenceCurvesController::setSamples()
{
    if (!m_currentProduct || !m_referenceCurve)
    {
        return;
    }

    m_upper->clear();
    m_middle->clear();
    m_lower->clear();

    if (auto upper = m_currentProduct->referenceCurveData(m_referenceCurve->upper()))
    {
        m_upper->addSamples(upper->samples());
    }

    if (auto middle = m_currentProduct->referenceCurveData(m_referenceCurve->middle()))
    {
        m_middle->addSamples(middle->samples());
    }

    if (auto lower = m_currentProduct->referenceCurveData(m_referenceCurve->lower()))
    {
        m_lower->addSamples(lower->samples());
    }
}

}
}
