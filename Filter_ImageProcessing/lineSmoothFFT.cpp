#include "lineSmoothFFT.h"

#include "module/moduleLogger.h"
#include "common/bitmap.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include "filter/lineSmooth.h"

#define FILTER_ID                 "fc142509-0d5e-43ab-8663-071d78516fea"
#define VARIANT_ID                "c7f11d53-0f52-4fe7-91ef-2f4a05e0e100"

#define PIPE_ID_LINEIN            "0326c456-e0f8-4071-913c-546d70719ef7"
#define PIPE_ID_LINEOUT           "aeb59cae-3141-4eaf-9bfe-ba23c3100e04"

namespace precitec
{
namespace filter
{

LineSmoothFFT::LineSmoothFFT()
    : TransformFilter("LineSmoothFFT", Poco::UUID(FILTER_ID))
    , m_lineIn(nullptr)
    , m_lineOut(this, "LineOut")
    , m_maxFrequency(0)
    , m_ransacIteration(100)
    , m_ransacThreshold(0)
    , m_lineOutGeo()
{
    parameters_.add("MaxFrequency", fliplib::Parameter::TYPE_int, m_maxFrequency);
    parameters_.add("RansacIteration", fliplib::Parameter::TYPE_int, m_ransacIteration);
    parameters_.add("RansacThreshold", fliplib::Parameter::TYPE_double, m_ransacThreshold);

    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_LINEIN), m_lineIn, "LineIn", 1, "LineIn"},
    });

    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_LINEOUT), &m_lineOut, "LineOut", 1, "LineOut"},
    });

    setVariantID(Poco::UUID(VARIANT_ID));
}

void LineSmoothFFT::setParameter()
{
    TransformFilter::setParameter();

    m_maxFrequency = parameters_.getParameter("MaxFrequency").convert<int>();
    m_ransacIteration = parameters_.getParameter("RansacIteration").convert<int>();
    m_ransacThreshold = parameters_.getParameter("RansacThreshold").convert<double>();
}

void LineSmoothFFT::paint()
{
    if (m_oVerbosity == eNone || m_lineOutGeo.ref().empty())
    {
        return;
    }

    image::OverlayCanvas &rCanvas (canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer &lineLayer(rCanvas.getLayerContour());

    const auto& line = m_lineOutGeo.ref().front().getData();
    const auto& trafo = *(m_lineOutGeo.context().trafo());

    lineLayer.add<image::OverlayPointList>(trafo(geo2d::Point(0,0)), line, image::Color::Orange(), /*connected*/ true);
}

bool LineSmoothFFT::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "LineIn")
    {
        m_lineIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecDoublearray>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void LineSmoothFFT::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto &lineInGeo = m_lineIn->read(m_oCounter);

    m_lineOutGeo = lineInGeo; //copy lineIn

    for (auto &line : m_lineOutGeo.ref())
    {
        auto &lineVec = line.getData();
        lineVec = smoothLine(lineVec, m_maxFrequency, m_ransacIteration, m_ransacThreshold);
    }

    auto lineOutGeo = m_lineOutGeo;
    preSignalAction();

    m_lineOut.signal(lineOutGeo);
}

} //namespace filter
} //namespace precitec
