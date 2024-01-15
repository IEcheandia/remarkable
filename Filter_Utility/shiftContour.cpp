#include "shiftContour.h"
#include "module/moduleLogger.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

namespace precitec
{
namespace filter
{

ShiftContour::ShiftContour()
    : TransformFilter("ShiftContour", Poco::UUID("240961cd-531c-4427-a814-4b8951858328"))
    , m_pipeContourIn(nullptr)
    , m_pipeShiftX(nullptr)
    , m_pipeShiftY(nullptr)
    , m_pipeContourOut(this, "ContourOut")
{
    setInPipeConnectors({
        {Poco::UUID("dca2a8f1-757b-4aed-a1f9-4472a12cd3f4"), m_pipeContourIn, "ContourIn", 1, "ContourIn"},
        {Poco::UUID("95c90fc7-e0ba-461c-8460-d783ed41317c"), m_pipeShiftX, "ShiftX", 1, "ShiftX"},
        {Poco::UUID("af1e2f1f-79f1-472f-b675-8b9df6666825"), m_pipeShiftY, "ShiftY", 1, "ShiftY"}
    });
    setOutPipeConnectors({
        {Poco::UUID("660bcda6-b1f9-484d-be12-88a8cebf37c7"), &m_pipeContourOut, "ContourOut", 1, "ContourOut"}
    });
    setVariantID(Poco::UUID("93c33c8b-df46-4c90-b07f-79c059282d95"));
}

bool ShiftContour::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ContourIn")
    {
        m_pipeContourIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>*>(&pipe);
    }
    else if (pipe.tag() == "ShiftX")
    {
        m_pipeShiftX = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else if (pipe.tag() == "ShiftY")
    {
        m_pipeShiftY = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else
    {
        return false;
    }
    return BaseFilter::subscribe(pipe, group);
}

void ShiftContour::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    interface::GeoVecAnnotatedDPointarray contourIn = m_pipeContourIn->read(m_oCounter);
    interface::GeoDoublearray shiftXArray = m_pipeShiftX->read(m_oCounter);
    interface::GeoDoublearray shiftYArray = m_pipeShiftY->read(m_oCounter);

    double shiftX = shiftXArray.ref().getData().at(0);
    double shiftY = shiftYArray.ref().getData().at(0);

    /* first make a copy of contourIn to contourOut, and then
     *shift each point of each contour in contour array */
    interface::GeoVecAnnotatedDPointarray contourOut = contourIn;

    std::vector<geo2d::TAnnotatedArray<geo2d::DPoint>> &contours = contourOut.ref();

    for (geo2d::TAnnotatedArray<geo2d::DPoint> &contour : contours)
    {
        std::vector<geo2d::DPoint> &contourPoints = contour.getData();
        for (geo2d::DPoint &point : contourPoints)
        {
            point = geo2d::DPoint(point.x + shiftX, point.y + shiftY);
        }
    }

    preSignalAction();
    m_pipeContourOut.signal(contourOut);
}

} //namespace filter
} //namespace precitec
