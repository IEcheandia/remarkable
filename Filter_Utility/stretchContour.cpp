#include "stretchContour.h"
#include "module/moduleLogger.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

namespace precitec
{
namespace filter
{

StretchContour::StretchContour()
    : TransformFilter("StretchContour", Poco::UUID("11d2b55c-fd39-470d-bd48-77aef41bf682"))
    , m_pipeContourIn(nullptr)
    , m_pipeScaleX(nullptr)
    , m_pipeScaleY(nullptr)
    , m_pipeContourOut(this, "ContourOut")
{
    setInPipeConnectors({
        {Poco::UUID("934f5666-78e6-40e1-b72c-9258cccd28bc"), m_pipeContourIn, "ContourIn", 1, "ContourIn"},
        {Poco::UUID("9307da1d-4ed6-4658-b5de-9eb12eb7ac4b"), m_pipeScaleX, "ScaleX", 1, "ScaleX"},
        {Poco::UUID("ab5fb90e-2d2d-4b84-b697-283346d0be94"), m_pipeScaleY, "ScaleY", 1, "ScaleY"}
    });
    setOutPipeConnectors({
        {Poco::UUID("825125da-b211-4a11-b0f0-bf6b2b262940"), &m_pipeContourOut, "ContourOut", 1, "ContourOut"}
    });
    setVariantID(Poco::UUID("33a52583-2fdd-457b-8e45-a0e3b023f24f"));
}

bool StretchContour::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ContourIn")
    {
        m_pipeContourIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>*>(&pipe);
    }
    else if (pipe.tag() == "ScaleX")
    {
        m_pipeScaleX = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else if (pipe.tag() == "ScaleY")
    {
        m_pipeScaleY = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else
    {
        return false;
    }
    return BaseFilter::subscribe(pipe, group);
}

void StretchContour::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    interface::GeoVecAnnotatedDPointarray contourIn = m_pipeContourIn->read(m_oCounter);
    interface::GeoDoublearray scaleXArray = m_pipeScaleX->read(m_oCounter);
    interface::GeoDoublearray scaleYArray = m_pipeScaleY->read(m_oCounter);

    double scaleX = scaleXArray.ref().getData().at(0);
    double scaleY = scaleYArray.ref().getData().at(0);

    /* first make a copy of contourIn to contourOut, and then
     *scale each point of each contour in contour array */
    interface::GeoVecAnnotatedDPointarray contourOut = contourIn;

    std::vector<geo2d::TAnnotatedArray<geo2d::DPoint>> &contours = contourOut.ref();

    for (geo2d::TAnnotatedArray<geo2d::DPoint> &contour : contours)
    {
        std::vector<geo2d::DPoint> &contourPoints = contour.getData();
        for (geo2d::DPoint &point : contourPoints)
        {
            point = geo2d::DPoint(point.x * scaleX, point.y * scaleY);
        }
    }

    preSignalAction();
    m_pipeContourOut.signal(contourOut);
}

} //namespace filter
} //namespace precitec
