/**
 * @file
 * @copyright   Precitec Vision GmbH & Co. KG
 * @author      Michelle Meier
 * @date        2021
 * @brief       Laserline tracking filter, based on lineTrackingXT with additional allowence the vertical distance to the starting point of the laserline.
 */

#include "lineTrackingDist.h"
#include "image/image.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "module/moduleLogger.h"
#include <filter/armStates.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec
{
namespace filter
{

const std::string LineTrackingDist::m_filterName = std::string("LineTrackingDist");

LineTrackingDist::LineTrackingDist() :
    TransformFilter( LineTrackingDist::m_filterName, Poco::UUID{"20e6e85c-5abf-42e5-99ac-833fa1453dc8"} ),
    m_pPipeInImageFrame(nullptr),
    m_pPipeInSearchThreshold(nullptr),
    m_pPipeInTrackStart(nullptr),
    m_pPipeInUpperMaxDiff(nullptr),
    m_pPipeInLowerMaxDiff(nullptr),
    m_pipeResY(nullptr),
    m_oLaserlineOutY(1), // work only with 1st line
    m_threshold(144),
    m_upperLower(1),
    m_pixelX(1),
    m_pixelY(10),
    m_averagingY(2),
    m_resolutionX(1),
    m_resolutionY(5),
    m_maxGapWidth(11),
    m_maxNumberOfGaps(4),
    m_maxLineJumpY(20),
    m_startAreaX(1),
    m_startAreaY(3)
{
    m_pipeResY = new fliplib::SynchronePipe<interface::GeoVecDoublearray>(this, "Line");

    parameters_.add("pixelx", fliplib::Parameter::TYPE_int, m_pixelX);
    parameters_.add("pixely", fliplib::Parameter::TYPE_int, m_pixelY);
    parameters_.add("averagingY", fliplib::Parameter::TYPE_int, m_averagingY);
    parameters_.add("resolutionX", fliplib::Parameter::TYPE_int, m_resolutionX);
    parameters_.add("resolutionY", fliplib::Parameter::TYPE_int, m_resolutionY);
    parameters_.add("maxGapWidth", fliplib::Parameter::TYPE_int, m_maxGapWidth);
    parameters_.add("maxNumberOfGaps", fliplib::Parameter::TYPE_int, m_maxNumberOfGaps);
    parameters_.add("maxLineJumpY", fliplib::Parameter::TYPE_int, m_maxLineJumpY);
    parameters_.add("startAreaX", fliplib::Parameter::TYPE_int, m_startAreaX);
    parameters_.add("startAreaY", fliplib::Parameter::TYPE_int, m_startAreaY);

    setInPipeConnectors({{Poco::UUID("e8b3d408-056b-48a2-a3f5-d254739c4c94"), m_pPipeInImageFrame, "ImageFrame", 1, "image"},
    {Poco::UUID("db0a4315-08f7-42d0-8caa-513e51db1b34"), m_pPipeInSearchThreshold, "SearchThreshold", 1, "searchThreshold"},
    {Poco::UUID("2173bc30-df11-4fd8-9a57-9bf3b8249235"), m_pPipeInTrackStart, "Trackstart", 1, "trackstart"},
    {Poco::UUID("8f6cde70-68c9-4d7e-81b4-132fa18d7242"), m_pPipeInUpperMaxDiff, "UpperMaxDiff", 1, "upperMaxDiff"},
    {Poco::UUID("48a0c3e3-2b45-49fd-ad33-f79db6c43d08"), m_pPipeInLowerMaxDiff, "LowerMaxDiff", 1, "lowerMaxDiff"}});
    setOutPipeConnectors({{Poco::UUID("1681982e-0d30-480e-b5ad-1d0f94cf4c9f"), m_pipeResY, "Line", 0, ""}});
    setVariantID(Poco::UUID("d3e2651b-4648-46d6-b12b-e59c2ab1b985"));
}


LineTrackingDist::~LineTrackingDist()
{
	delete m_pipeResY;
}

void LineTrackingDist::setParameter()
{
    TransformFilter::setParameter();

    m_laserlineTracker.m_par.iMaxBreiteUnterbruch = parameters_.getParameter("maxGapWidth");
    m_laserlineTracker.m_par.iMaxAnzahlUnterbrueche = parameters_.getParameter("maxNumberOfGaps");
    m_laserlineTracker.m_par.iMaxLinienSprungY = parameters_.getParameter("maxLineJumpY");
    m_laserlineTracker.m_par.iAufloesungX = parameters_.getParameter("resolutionX");
    m_laserlineTracker.m_par.iAufloesungY = parameters_.getParameter("resolutionY");
    m_laserlineTracker.m_par.iMittelungX = parameters_.getParameter("pixelx");
    m_laserlineTracker.m_par.iPixelY = parameters_.getParameter("pixely");
    m_laserlineTracker.m_par.iMittelungY = parameters_.getParameter("averagingY");
    m_laserlineTracker.m_par.startAreaX = parameters_.getParameter("startAreaX");
    m_laserlineTracker.m_par.startAreaY = parameters_.getParameter("startAreaY");
}


void LineTrackingDist::paint()
{
    if(m_oVerbosity < VerbosityType::eLow || m_oSpTrafo.isNull())
    {
        return;
    }

    const interface::Trafo  &rTrafo         (*m_oSpTrafo);
    OverlayCanvas           &rCanvas        (canvas<OverlayCanvas>(m_oCounter));
    OverlayLayer            &rLayerContour  (rCanvas.getLayerContour());
    OverlayLayer            &rLayerPosition (rCanvas.getLayerPosition());
    int yOld = 0, yNew = 0;

    if((m_laserlineTracker.result.firstValidIndex >= 0)
        && (m_laserlineTracker.result.lastValidIndex > 0)
        && (m_laserlineTracker.result.lastValidIndex > m_laserlineTracker.result.firstValidIndex)
        )
    {
        for (int x = m_laserlineTracker.result.firstValidIndex; x <= m_laserlineTracker.result.lastValidIndex; ++x)
        {
            yNew = m_laserlineTracker.result.Y[x];

            if (x == m_laserlineTracker.result.firstValidIndex)
            {
                yOld = yNew;
            }

            if (yNew == yOld)
            {
                // Mark this single point
                rLayerContour.add<OverlayPoint>(rTrafo(geo2d::Point(x,yNew)), Color(0x1E90FF)); // dodgerblue
            }
            else
            {
                // Draw a line from before point to actual
                rLayerContour.add<OverlayLine>(rTrafo(geo2d::Point(x - 1, yOld)), rTrafo(geo2d::Point(x, yNew)), Color(0x1E90FF)); // dodgerblue
                yOld = yNew;
            }
        }
    }
    else
    {
        wmLog(precitec::eDebug, "Filter '%s': no valid tracking points\n", m_filterName.c_str());
    }

    // draw tracking starting points
    if(m_oVerbosity > eLow)
    {
        if((m_laserlineTracker.result.laserLineStartPos.x >= 0) && (m_laserlineTracker.result.laserLineStartPos.y >= 0))
        {
            rLayerPosition.add<OverlayCross>(rTrafo(geo2d::Point(m_laserlineTracker.result.laserLineStartPos.x, m_laserlineTracker.result.laserLineStartPos.y)), Color::Yellow());
        } else
        {
            wmLog(precitec::eDebug, "Filter '%s': no valid tracking startpoint left\n", m_filterName.c_str());
        }

        if((m_laserlineTracker.result.laserLineEndPos.x >= 0) && (m_laserlineTracker.result.laserLineEndPos.y >= 0))
        {
            rLayerPosition.add<OverlayCross>(rTrafo(geo2d::Point(m_laserlineTracker.result.laserLineEndPos.x, m_laserlineTracker.result.laserLineEndPos.y)), Color::Yellow());
        } else
        {
            wmLog(precitec::eDebug, "Filter '%s': no valid tracking startpoint right\n", m_filterName.c_str());
        }
    }
}

bool LineTrackingDist::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if (p_rPipe.tag() == "image")
    {
        m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == "searchThreshold")
    {
        m_pPipeInSearchThreshold = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == "trackstart")
    {
        m_pPipeInTrackStart = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == "upperMaxDiff")
    {
        m_pPipeInUpperMaxDiff = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == "lowerMaxDiff")
    {
        m_pPipeInLowerMaxDiff = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
    }

    return BaseFilter::subscribe(p_rPipe, p_oGroup);
}


void LineTrackingDist::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg)
{
    poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInSearchThreshold != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInTrackStart != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInUpperMaxDiff != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInLowerMaxDiff != nullptr); // to be asserted by graph editor

    const auto frame = m_pPipeInImageFrame->read(m_oCounter);
    const auto& rGeoSearchThresholdIn = m_pPipeInSearchThreshold->read(m_oCounter).ref().getData();
    const auto& rGeoTrackStartIn = m_pPipeInTrackStart->read(m_oCounter).ref().getData();
    const auto& rGeoUpperIn = m_pPipeInUpperMaxDiff->read(m_oCounter).ref().getData();
    const auto& rGeoLowerIn = m_pPipeInLowerMaxDiff->read(m_oCounter).ref().getData();
    poco_assert_dbg(!rGeoSearchThresholdIn.empty()); // to be asserted by graph editor
    poco_assert_dbg(!rGeoTrackStartIn.empty()); // to be asserted by graph editor
    poco_assert_dbg(!rGeoUpperIn.empty()); // to be asserted by graph editor
    poco_assert_dbg(!rGeoLowerIn.empty()); // to be asserted by graph editor

    const BImage &rImageIn = frame.data();
    m_oSpTrafo	= frame.context().trafo();
    m_imageSize= rImageIn.size();
    const geo2d::Range validImageRangeX (0, m_imageSize.width - 1);

    if (rImageIn.isValid() == false)
    {
        const auto oAnalysisResult	= frame.analysisResult();
        const interface::GeoVecDoublearray &geoLaserlineOut = interface::GeoVecDoublearray(frame.context(), m_oLaserlineOutY, oAnalysisResult, interface::NotPresent);
        preSignalAction();
        m_pipeResY->signal(geoLaserlineOut); // send new image with old context

        return;
    }

    m_laserlineTracker.m_par.doubleTracking = false;
    m_laserlineTracker.m_par.lapJoin = true;
    m_laserlineTracker.m_par.iSuchSchwelle = int(rGeoSearchThresholdIn[0]);
    m_laserlineTracker.m_par.iTrackStart = int(rGeoTrackStartIn[0]);
    m_laserlineTracker.m_par.upperMaxLineDiff = int(rGeoUpperIn[0]);
    m_laserlineTracker.m_par.lowerMaxLineDiff = int(rGeoLowerIn[0]);

    m_laserlineTracker.alloc(rImageIn.width(), rImageIn.height());

    m_laserlineTracker.process(&frame); // the actual tracking

    double rank = 1.0;

    if(m_laserlineTracker.result.firstValidIndex == -1 || m_laserlineTracker.result.lastValidIndex == -1)
    {
        rank = 0.0;
    }

    int numberOfElements = m_laserlineTracker.result.getAllocated();

    if(m_pipeResY->linked())
    {
        std::vector<double>& data = m_oLaserlineOutY.front().getData(); // work only with 1st line
        data.assign(numberOfElements, -1);

        std::vector<int>& rankPerElement = m_oLaserlineOutY.front().getRank(); // work only with 1st line
        rankPerElement.assign(numberOfElements, 0);

        if(m_laserlineTracker.result.firstValidIndex > 0 && validImageRangeX.contains(m_laserlineTracker.result.laserLineStartPos.x))
        {
            for(int i = 0; i < m_laserlineTracker.result.laserLineStartPos.x; ++i) // until the first valid value
            {
                rankPerElement[i++] = 0;
            }
        }

        if(m_laserlineTracker.result.lastValidIndex > 0 && validImageRangeX.contains(m_laserlineTracker.result.laserLineEndPos.x))
        {
            for(int i = m_laserlineTracker.result.laserLineEndPos.x; i < numberOfElements; ++i) // until the end of the roi
            {
                rankPerElement[i++] = 0;
            }
        }

        if(validImageRangeX.contains(m_laserlineTracker.result.firstValidIndex) && validImageRangeX.contains(m_laserlineTracker.result.lastValidIndex))
        {
            int w;
            for(int i = m_laserlineTracker.result.firstValidIndex; i <= m_laserlineTracker.result.lastValidIndex; ++i)
            {
                w = m_laserlineTracker.result.Y[i];
                data[i] = w;
                if(w < 0)
                {
                    rankPerElement[i] = 0;
                } else
                {
                    rankPerElement[i] = 255;
                }
            }
        }

        const auto analysisResult = frame.analysisResult() == interface::AnalysisOK ? interface::AnalysisOK : frame.analysisResult();
        const interface::GeoVecDoublearray &geoLaserlineOut = interface::GeoVecDoublearray(frame.context(), m_oLaserlineOutY, analysisResult, rank);
        preSignalAction();
        m_pipeResY->signal(geoLaserlineOut);
    }
    else
    {
        preSignalAction();
    }
}

void LineTrackingDist::arm(const fliplib::ArmStateBase& state)
{
    if (state.getStateID() == eSeamStart)
    {
        m_oSpTrafo = nullptr;
    }
}

}
}
