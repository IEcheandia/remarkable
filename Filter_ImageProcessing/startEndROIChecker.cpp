/**
*  @copyright: Precitec Vision GmbH & Co. KG
*  @author     MM
*  @date       2022
*  @file
*  @brief      Checks if ROI is on blank or (partly) on background. Output: 1 on blank, 2: not completely on blank
*/

#include "startEndROIChecker.h"

#include "module/moduleLogger.h"
#include "filter/algoArray.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <fliplib/TypeToDataTypeImpl.h>
#include <overlay/overlayCanvas.h>


namespace precitec
{
namespace filter
{

StartEndROIChecker::StartEndROIChecker()
    : TransformFilter("StartEndROIChecker", Poco::UUID("2FEE99CD-3F01-4529-A0CC-B9D5225B00B0"))
    , m_startEndInfoIn(nullptr)
    , m_imageIn(nullptr)
    , m_onBlankOut(this, "OnBlank")
    , m_edge{}
{
    setInPipeConnectors({
        {Poco::UUID("D0E38722-313F-4365-9F40-69FFB3103A10"), m_startEndInfoIn, "StartEndInfoIn", 1, "StartEndInfoIn"},
        {Poco::UUID("4705FEBE-7534-4C0E-8120-050015D8628B"), m_imageIn, "ImageIn", 1, "ImageIn"},
    });

    setOutPipeConnectors({
        {Poco::UUID("BDB3B92C-913A-4897-84EB-43B5905329B6"), &m_onBlankOut, "OnBlank", 1, "OnBlank"},
    });

    setVariantID(Poco::UUID("36B30D79-95A6-400B-B7F6-49C17426B188"));
}

StartEndROIChecker::~StartEndROIChecker() = default;

void StartEndROIChecker::setParameter()
{
    TransformFilter::setParameter();
}

void StartEndROIChecker::paint()
{
    if (m_oVerbosity == eNone || (m_oVerbosity < eMedium && m_onBlank))
    {
        return;
    }

    image::OverlayCanvas& overlayCanvas(canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer& layerContour(overlayCanvas.getLayerContour());

    // Draw roi rectangle. ROI on blank: red, ROI on background: green
    bool unicolor = false;
    if (m_edge.m != 0 && !m_onBlank)
    {
        const auto xIntersectionTop = (m_roiStart.y - m_edge.q) / m_edge.m;
        if (xIntersectionTop > m_roiStart.x && xIntersectionTop < m_roiEnd.x)
        {
            if ((m_topMaterial && m_edge.m < 0) || (!m_topMaterial && m_edge.m > 0))
            {
                layerContour.add<image::OverlayLine>(m_roiStart.x, m_roiStart.y, xIntersectionTop, m_roiStart.y, image::Color::Green());
                layerContour.add<image::OverlayLine>(xIntersectionTop, m_roiStart.y, m_roiEnd.x, m_roiStart.y, image::Color::Red());
            }
            else if ((m_topMaterial && m_edge.m > 0) || (!m_topMaterial && m_edge.m < 0))
            {
                layerContour.add<image::OverlayLine>(m_roiStart.x, m_roiStart.y, xIntersectionTop, m_roiStart.y, image::Color::Red());
                layerContour.add<image::OverlayLine>(xIntersectionTop, m_roiStart.y, m_roiEnd.x, m_roiStart.y, image::Color::Green());
            }
        }
        else
        {
            unicolor = true;
        }
    }
    if (m_edge.m == 0 || unicolor || m_onBlank)
    {
        if (m_onBlank || (m_topMaterial && (m_edge.getY(m_roiStart.x + m_startEndTrafoX) > m_roiStart.y || m_edge.getY(m_roiEnd.x + m_startEndTrafoX) > m_roiEnd.y)))
        {
            layerContour.add<image::OverlayLine>(m_roiStart.x, m_roiStart.y, m_roiEnd.x, m_roiStart.y, image::Color::Green());
        }
        else
        {
            layerContour.add<image::OverlayLine>(m_roiStart.x, m_roiStart.y, m_roiEnd.x, m_roiStart.y, image::Color::Red());
        }
    }

    unicolor = false;
    if (m_edge.m != 0 && !m_onBlank)
    {
        const auto xIntersectionBottom = (m_roiEnd.y - m_edge.q) / m_edge.m;
        if (xIntersectionBottom > m_roiStart.x && xIntersectionBottom < m_roiEnd.x)
        {
            if ((m_topMaterial && m_edge.m < 0) || (!m_topMaterial && m_edge.m > 0))
            {
                layerContour.add<image::OverlayLine>(m_roiStart.x, m_roiEnd.y, xIntersectionBottom, m_roiEnd.y, image::Color::Green());
                layerContour.add<image::OverlayLine>(xIntersectionBottom, m_roiEnd.y, m_roiEnd.x, m_roiEnd.y, image::Color::Red());
            }
            else if ((m_topMaterial && m_edge.m > 0) || (!m_topMaterial && m_edge.m < 0))
            {
                layerContour.add<image::OverlayLine>(m_roiStart.x, m_roiEnd.y, xIntersectionBottom, m_roiEnd.y, image::Color::Red());
                layerContour.add<image::OverlayLine>(xIntersectionBottom, m_roiEnd.y, m_roiEnd.x, m_roiEnd.y, image::Color::Green());
            }
        }
        else
        {
            unicolor = true;
        }
    }
    if (m_edge.m == 0 || unicolor || m_onBlank)
    {
        if (m_onBlank || (!m_topMaterial && (m_edge.getY(m_roiStart.x + m_startEndTrafoX) < m_roiStart.y || m_edge.getY(m_roiEnd.x + m_startEndTrafoX) < m_roiEnd.y)))
        {
            layerContour.add<image::OverlayLine>(m_roiStart.x, m_roiEnd.y, m_roiEnd.x, m_roiEnd.y, image::Color::Green());
        }
        else
        {
            layerContour.add<image::OverlayLine>(m_roiStart.x, m_roiEnd.y, m_roiEnd.x, m_roiEnd.y, image::Color::Red());
        }
    }

    const auto yIntersectionLeft = m_edge.getY(m_roiStart.x + m_startEndTrafoX);
    if (!m_onBlank && yIntersectionLeft > m_roiStart.y && yIntersectionLeft < m_roiEnd.y)
    {
        if (m_topMaterial)
        {
            layerContour.add<image::OverlayLine>(m_roiStart.x, m_roiStart.y, m_roiStart.x, yIntersectionLeft, image::Color::Green());
            layerContour.add<image::OverlayLine>(m_roiStart.x, yIntersectionLeft, m_roiStart.x, m_roiEnd.y, image::Color::Red());
        }
        else
        {
            layerContour.add<image::OverlayLine>(m_roiStart.x, m_roiStart.y, m_roiStart.x, yIntersectionLeft, image::Color::Red());
            layerContour.add<image::OverlayLine>(m_roiStart.x, yIntersectionLeft, m_roiStart.x, m_roiEnd.y, image::Color::Green());
        }
    }
    else if (m_onBlank || (m_topMaterial && m_edge.getY(m_roiStart.x + m_startEndTrafoX) > m_roiEnd.y) || (!m_topMaterial && m_edge.getY(m_roiStart.x + m_startEndTrafoX) < m_roiEnd.y))
    {
        layerContour.add<image::OverlayLine>(m_roiStart.x, m_roiStart.y, m_roiStart.x, m_roiEnd.y, image::Color::Green());
    }
    else
    {
        layerContour.add<image::OverlayLine>(m_roiStart.x, m_roiStart.y, m_roiStart.x, m_roiEnd.y, image::Color::Red());
    }

    const auto yIntersectionRight = m_edge.getY(m_roiEnd.x + m_startEndTrafoX);
    if (!m_onBlank && yIntersectionRight > m_roiStart.y && yIntersectionRight < m_roiEnd.y)
    {
        if (m_topMaterial)
        {
            layerContour.add<image::OverlayLine>(m_roiEnd.x, m_roiStart.y, m_roiEnd.x, yIntersectionRight, image::Color::Green());
            layerContour.add<image::OverlayLine>(m_roiEnd.x, yIntersectionRight, m_roiEnd.x, m_roiEnd.y, image::Color::Red());
        }
        else
        {
            layerContour.add<image::OverlayLine>(m_roiEnd.x, m_roiStart.y, m_roiEnd.x, yIntersectionRight, image::Color::Red());
            layerContour.add<image::OverlayLine>(m_roiEnd.x, yIntersectionRight, m_roiEnd.x, m_roiEnd.y, image::Color::Green());
        }
    }
    else if (m_onBlank || (m_topMaterial && m_edge.getY(m_roiEnd.x + m_startEndTrafoX) > m_roiEnd.y) || (!m_topMaterial && m_edge.getY(m_roiEnd.x + m_startEndTrafoX) < m_roiEnd.y))
    {
        layerContour.add<image::OverlayLine>(m_roiEnd.x, m_roiStart.y, m_roiEnd.x, m_roiEnd.y, image::Color::Green());
    }
    else
    {
        layerContour.add<image::OverlayLine>(m_roiEnd.x, m_roiStart.y, m_roiEnd.x, m_roiEnd.y, image::Color::Red());
    }
}

bool StartEndROIChecker::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "StartEndInfoIn")
    {
        m_startEndInfoIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoStartEndInfoarray>*>(&pipe);
    }
    else if (pipe.tag() == "ImageIn")
    {
        m_imageIn = dynamic_cast<fliplib::SynchronePipe<interface::ImageFrame>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void StartEndROIChecker::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto& startEndInfoIn = m_startEndInfoIn->read(m_oCounter);
    const auto& imageIn = m_imageIn->read(m_oCounter);
    const auto& startEndInfoData = startEndInfoIn.ref().getData();
    const auto& startEndInfoContext = startEndInfoIn.context();

    if(startEndInfoData.empty() || !imageIn.data().isValid())
    {
        interface::GeoDoublearray geoOnBlankOut(imageIn.context(), geo2d::Doublearray{1, 0, eRankMin}, imageIn.analysisResult(), interface::NotPresent);

        preSignalAction();
        m_onBlankOut.signal(geoOnBlankOut);
        return;
    }

    m_startEndTrafoX = startEndInfoContext.getTrafoX();
    m_startEndTrafoY = startEndInfoContext.getTrafoY();

    const size_t startEndInfoDataSize{startEndInfoData.size()};
    geo2d::Doublearray isOnBlankOut{};
    isOnBlankOut.resize(startEndInfoDataSize);
    isOnBlankOut.getRank().assign(startEndInfoDataSize, eRankMax);

    // Transform everything to coordinates of the source image. The in pipes have usually different trafos.
    for (auto i = 0u; i < startEndInfoDataSize; i++)
    {
        const auto& startEndInfoDataN = startEndInfoData[i];

        const Point roiStart{imageIn.context().getTrafoX(), imageIn.context().getTrafoY()};
        const Point roiEnd{roiStart.x + imageIn.data().width(), roiStart.y + imageIn.data().height()};


        m_onBlank = true;

        // no background visible
        if(startEndInfoDataN.m_oImageState == geo2d::StartEndInfo::ImageState::OnlyMaterial)
        {
            m_roiStart = roiStart;
            m_roiEnd = roiEnd;

            isOnBlankOut.getData()[i] = 1;
            continue;
        }
        // no material visible
        if(startEndInfoDataN.m_oImageState == geo2d::StartEndInfo::ImageState::OnlyBackground || startEndInfoDataN.m_oImageState == geo2d::StartEndInfo::ImageState::Unknown
            || startEndInfoDataN.m_oImageState == geo2d::StartEndInfo::ImageState::Invalid)
        {
            m_onBlank = false;
            m_roiStart = roiStart;
            m_roiEnd = roiEnd;
            m_topMaterial = false;
            m_edge.q = roiStart.x;
            m_edge.m = 0;

            isOnBlankOut.getData()[i] = 0;
            continue;
        }

        geo2d::StartEndInfo::FittedLine edge{0, 0};
        bool topMaterial{!startEndInfoDataN.isTopDark};
        const bool oneSideFullBackground{startEndInfoDataN.isTopDark && startEndInfoDataN.isBottomDark};
        const bool oneSideFullMaterial{startEndInfoDataN.isTopMaterial && startEndInfoDataN.isBottomMaterial};
        if (startEndInfoDataN.m_oImageState == geo2d::StartEndInfo::ImageState::OnlyLeftEdgeVisible)
        {
            edge = startEndInfoDataN.leftEdge;
            if (startEndInfoDataN.isTopDark && startEndInfoDataN.isBottomDark)
            {
                topMaterial = startEndInfoDataN.isTopMaterial;
            }
            // Roi is located on the right image side. There is either full material or full background.
            if (roiStart.x > m_startEndTrafoX + startEndInfoDataN.imageWidth * 0.5 && (oneSideFullBackground || oneSideFullMaterial))
            {
                edge.m = 0;
                edge.q = 0;
                topMaterial = oneSideFullBackground;
            }
        }
        else if (startEndInfoDataN.m_oImageState == geo2d::StartEndInfo::ImageState::OnlyRightEdgeVisible)
        {
            edge = startEndInfoDataN.rightEdge;
            if (startEndInfoDataN.isTopDark && startEndInfoDataN.isBottomDark)
            {
                topMaterial = startEndInfoDataN.isTopMaterial;
            }
            else if (roiEnd.x < m_startEndTrafoX + startEndInfoDataN.imageWidth * 0.5 && (oneSideFullBackground || oneSideFullMaterial))
            {
                // on the left side is only material and roi is located on this side
                edge.m = 0;
                edge.q = 0;
                topMaterial = oneSideFullBackground;
            }
        }
        else if (startEndInfoDataN.m_oImageState == geo2d::StartEndInfo::ImageState::FullEdgeVisible)
        {
            // if both edges are visible take the edge of the side of the roi. If roi is on both sides, take edge with smaller space on blank to minimize a faulty result "fully on blank".
            if (roiEnd.x < m_startEndTrafoX + startEndInfoDataN.imageWidth * 0.5)
            {
                edge = startEndInfoDataN.leftEdge;
            }
            else if (roiStart.x > m_startEndTrafoX + startEndInfoDataN.imageWidth * 0.5)
            {
                edge = startEndInfoDataN.rightEdge;
            }
            else
            {
                auto greaterEdge = startEndInfoDataN.rightEdge.getY(roiStart.x) > startEndInfoDataN.leftEdge.getY((roiStart.x)) ? startEndInfoDataN.rightEdge : startEndInfoDataN.leftEdge;
                auto smallerEdge = startEndInfoDataN.rightEdge.getY(roiStart.x) < startEndInfoDataN.leftEdge.getY((roiStart.x)) ? startEndInfoDataN.rightEdge : startEndInfoDataN.leftEdge;
                edge = topMaterial ? smallerEdge : greaterEdge;
            }
        }
        else
        {
            // should never reach this, just in case e.g. someone added a new image state
            m_topMaterial = true;
        }

        edge.q += m_startEndTrafoY;
        // get the corner which will first intersect with the edge
        auto x = topMaterial ? (edge.m > 0 ? roiStart.x : roiEnd.x) : (edge.m > 0 ? roiEnd.x : roiStart.x);
        x += m_startEndTrafoX;
        auto y = topMaterial ? roiEnd.y : roiStart.y;

        if ((topMaterial && y > edge.getY(x)) || (!topMaterial && y < edge.getY(x)))
        {
            isOnBlankOut.getData()[i] = 0;
            m_onBlank = false;
        }
        else
        {
            isOnBlankOut.getData()[i] = 1;
        }

        m_edge = edge;
        m_roiStart = roiStart;
        m_roiEnd = roiEnd;
        m_topMaterial = topMaterial;
    }

    interface::GeoDoublearray geoOnBlankOut(imageIn.context(), isOnBlankOut, imageIn.analysisResult(), interface::Limit);

    preSignalAction();
    m_onBlankOut.signal(geoOnBlankOut);
}

}
}

