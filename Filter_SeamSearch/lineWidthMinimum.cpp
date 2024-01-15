/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     OS
 *  @date       2015
 *  @brief      This filter gets a seam width and looks for a minimum in the laserline width curve
 */

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>
// local includes
#include "lineWidthMinimum.h"

using namespace fliplib;
namespace precitec {
    using namespace interface;
    using namespace image;
    using namespace geo2d;
namespace filter {

const std::string LineWidthMinimum::m_oFilterName             = std::string("LineWidthMinimum");
const std::string LineWidthMinimum::PIPENAME_SEAMPOS_OUT      = std::string("SeamPositionOut");
const std::string LineWidthMinimum::PIPENAME_SEAMLEFT_OUT     = std::string("SeamLeftOut");
const std::string LineWidthMinimum::PIPENAME_SEAMRIGHT_OUT    = std::string("SeamRightOut");
const std::string LineWidthMinimum::PIPENAME_SEAMFINDINGS_OUT = std::string("SeamFindingOut");


LineWidthMinimum::LineWidthMinimum() :
    TransformFilter( LineWidthMinimum::m_oFilterName, Poco::UUID{"24CE7A6A-08C0-49D4-ACEC-BE7081766D6D"} ),
    m_pPipeInLaserLine    ( nullptr ),
    m_pPipeInSeamWidth    ( nullptr ),
    m_pPipeInSeamFindings ( nullptr ),
    m_oFilterLength       ( 30 ),         // Filter for checking of laserline width/intensity
    m_oMinYDistance       ( 10 ),         // Min distance between y min and y max
    m_oLaserlineWidth     ( 1 )           // usually one line
{
    m_pPipeOutSeamPos = new SynchronePipe< interface::GeoDoublearray > ( this, LineWidthMinimum::PIPENAME_SEAMPOS_OUT );
    m_pPipeOutSeamLeft = new SynchronePipe< interface::GeoDoublearray > ( this, LineWidthMinimum::PIPENAME_SEAMLEFT_OUT );
    m_pPipeOutSeamRight = new SynchronePipe< interface::GeoDoublearray >(this, LineWidthMinimum::PIPENAME_SEAMRIGHT_OUT);
    m_pPipeOutSeamFindings = new SynchronePipe< interface::GeoSeamFindingarray >(this, LineWidthMinimum::PIPENAME_SEAMFINDINGS_OUT);

    // Set default values of the parameters of the filter
    parameters_.add("Mode", Parameter::TYPE_int, m_oMode);
    parameters_.add("FilterLength", Parameter::TYPE_int, m_oFilterLength);
    parameters_.add("MinYDistance", Parameter::TYPE_int, m_oMinYDistance);

    setInPipeConnectors({{Poco::UUID("4B631385-C9E7-4D90-A870-F9EE49D5121B"), m_pPipeInLaserLine, "LineWidthIn", 1, "LineWidth"},{Poco::UUID("F1E3A3C1-C1FE-472D-86D3-B3A92137290B"), m_pPipeInSeamWidth, "SeamWidthIn", 1, "SeamWidth"},{Poco::UUID("765946C0-466A-4A83-A6B4-B73548A833A1"), m_pPipeInSeamFindings, "SeamFindingIn", 1, "SeamFinding"}});
    setOutPipeConnectors({{Poco::UUID("D13343BE-2A2A-4D93-9849-BEB2E6503D7D"), m_pPipeOutSeamPos, PIPENAME_SEAMPOS_OUT, 0, ""},
    {Poco::UUID("CFB1C37A-B89A-42B9-AF19-873A8F1A4043"), m_pPipeOutSeamLeft, PIPENAME_SEAMLEFT_OUT, 0, ""},
    {Poco::UUID("D1A7BC63-0621-4AD1-B592-B2595E5B94D7"), m_pPipeOutSeamRight, PIPENAME_SEAMRIGHT_OUT, 0, ""},
    {Poco::UUID("19D37443-C93A-485E-86DD-6B588F276CE1"), m_pPipeOutSeamFindings, PIPENAME_SEAMFINDINGS_OUT, 0, ""}});
    setVariantID(Poco::UUID("EBA3C09D-9545-4652-9180-0CBC3C4ACFA4"));
} // LineProfile


LineWidthMinimum::~LineWidthMinimum()
{
    delete m_pPipeOutSeamPos;
    delete m_pPipeOutSeamLeft;
    delete m_pPipeOutSeamRight;
    delete m_pPipeOutSeamFindings;
} // ~LineProfile


void LineWidthMinimum::setParameter()
{
    TransformFilter::setParameter();
    m_oMode         = parameters_.getParameter("Mode").convert<int>();
    m_oFilterLength = parameters_.getParameter("FilterLength").convert<int>();
    m_oMinYDistance = parameters_.getParameter("MinYDistance").convert<int>();
} // setParameter


bool LineWidthMinimum::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    m_pPipeInSeamFindings = nullptr;
    if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
        m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
    if (p_rPipe.type() == typeid(GeoDoublearray))
        m_pPipeInSeamWidth = dynamic_cast< SynchronePipe < GeoDoublearray > * >(&p_rPipe);
    if (p_rPipe.type() == typeid(GeoSeamFindingarray))
        m_pPipeInSeamFindings = dynamic_cast< SynchronePipe < GeoSeamFindingarray > * >(&p_rPipe);

    return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


void LineWidthMinimum::paint()
{
    if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
    {
        return;
    } // if

    try
    {
        const Trafo	    &rTrafo(*m_oSpTrafo);
        OverlayCanvas   &rCanvas(canvas<OverlayCanvas>(m_oCounter));
        OverlayLayer    &rLayerContour(rCanvas.getLayerContour());

        const auto offset = rTrafo(Point(0,0));    // Upper left corner of 'Lasrline ROI'
        const auto &rWidthVectorOut	= m_oLaserlineWidth.front().getData();
        //const auto &rRankVectorOut  = m_oLaserlineWidth.front().getRank();
        int yOld, yNew;

        if (m_oVerbosity > eLow)
        {
            // Draw the filtered laserline width/intensity values
            for (unsigned int xPos = 0; xPos != rWidthVectorOut.size(); ++xPos)
            {
                // Upper border of laserline ROI as 'reference' = offset.y
                yNew = (int) (0.5 + offset.y - rWidthVectorOut[xPos]);

                if (xPos == 0)
                    yOld = yNew;

                if (yNew == yOld)
                {
                    // Mark this single point
                    rLayerContour.add<OverlayPoint>(Point(offset.x + xPos, yNew), Color::Orange());
                }
                else
                {
                    // Draw a line from before point to actual
                    rLayerContour.add<OverlayLine>(Point(offset.x + xPos - 1, yOld), Point(offset.x + xPos, yNew), Color::Orange());
                    yOld = yNew;
                }
            }

            // Draw the found min position
            if ((m_resultSeamLeft != 0) && (m_resultSeamRight != 0) && (m_resultSeamPos != 0))
            {
                if (m_oMode == 1)   // Search 1. and 2. min
                {
                    // Upper border of laserline ROI as 'reference' = offset.y
                    yNew = (int) (0.5 + offset.y - rWidthVectorOut[m_minX_1]);

                    // Draw a cross for each found min
                    if (m_IsMinOk)
                    {
                        // Good 'Min' found! Mark with big crosses
                        rLayerContour.add<OverlayCross>(Point(offset.x + m_minX_1,      yNew), 20, Color::Green());
                        rLayerContour.add<OverlayCross>(Point(offset.x + m_minX_1 - 1,  yNew), 20, Color::Green());
                    }
                    else
                    {
                        // Only a small 'Min' found! Mark with small cross
                        rLayerContour.add<OverlayCross>(Point(offset.x + m_minX_1, yNew), 10, Color::Red());
                    }

                    // Also 2nd min found?
                    if (m_minX_2 > 0)
                    {
                        // Mark with small cross
                        rLayerContour.add<OverlayCross>(Point(offset.x + m_minX_2, yNew), 10, Color::Red());
                    }
                }
                else   // Search 1 min
                {
                    // Draw a cross for left/right found seam positions
                    if (m_IsMinOk)
                    {
                        // Good 'Min' found! Mark with big crosses
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamLeft,    int(30))), 20, Color::Green());
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamLeft-1,  int(30))), 20, Color::Green());
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamRight,   int(30))), 20, Color::Green());
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamRight+1, int(30))), 20, Color::Green());
                    }
                    else
                    {
                        // Only a small 'Min' found! Mark with small crosses
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamLeft,  int(30))), 10, Color::Red());
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamRight, int(30))), 10, Color::Red());
                    }
                }
            }
        }
        else   // m_oVerbosity = eLow
        {
            // Draw the found min position
            if ((m_resultSeamLeft != 0) && (m_resultSeamRight != 0) && (m_resultSeamPos != 0))
            {
                if (m_oMode == 1)   // Search 1. and 2. min
                {
                    // Draw a cross for each found min
                    if (m_IsMinOk)
                    {
                        // Good 'Min' found! Mark with big cross
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamPos,     int(30))), 20, Color::Green());
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamPos - 1, int(30))), 20, Color::Green());
                    }
                    else
                    {
                        // Only a small 'Min' found! Mark with small cross
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamPos,     int(30))), 10, Color::Red());
                    }
                }
                else   // Search 1 min
                {
                    // Draw a cross for left/right found seam positions
                    if (m_IsMinOk)
                    {
                        // Good 'Min' found! Mark with big crosses
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamLeft,    int(30))), 20, Color::Green());
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamLeft-1,  int(30))), 20, Color::Green());
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamRight,   int(30))), 20, Color::Green());
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamRight+1, int(30))), 20, Color::Green());
                    }
                    else
                    {
                        // Only a small 'Min' found! Mark with small crosses
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamLeft,  int(30))), 10, Color::Red());
                        rLayerContour.add<OverlayCross>(rTrafo(Point(m_resultSeamRight, int(30))), 10, Color::Red());
                    }
                }
            }
        }
    }
    catch(...)
    {
        return;
    }
} // paint


void LineWidthMinimum::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
    poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInSeamWidth != nullptr); // to be asserted by graph editor
    //poco_assert_dbg(m_pPipeInSeamFindings != nullptr); // to be asserted by graph editor

    m_resultSeamLeft = m_resultSeamRight = m_resultSeamPos = 0;
    m_IsMinOk = false;

    const GeoDoublearray& rSeamWidthIn = m_pPipeInSeamWidth->read(m_oCounter);
    const GeoVecDoublearray& rLaserLineIn = m_pPipeInLaserLine->read(m_oCounter);

    SeamFindingarray seamFindingArray;
    if (m_pPipeInSeamFindings != nullptr)
    {
        const GeoSeamFindingarray& rSeamFindingsIn = m_pPipeInSeamFindings->read(m_oCounter);
        seamFindingArray = rSeamFindingsIn.ref();
    }
    else
    {
        seamFindingArray.getData().push_back(SeamFinding(0, 0, 0, 0));
        seamFindingArray.getRank().push_back(0);
    }

    m_oSpTrafo	= rLaserLineIn.context().trafo();

    // And extract byte-array
    const VecDoublearray& rLaserarray = rLaserLineIn.ref();

    geo2d::Doublearray oOutSeamPos;
    geo2d::Doublearray oOutSeamLeft;
    geo2d::Doublearray oOutSeamRight;
    geo2d::SeamFindingarray oOutSeamFindings = seamFindingArray;

    // (re)initialization of laserline width structure
    resetFromInput(rLaserLineIn.ref(), m_oLaserlineWidth);

    // input validity check
    if ( inputIsInvalid(rLaserLineIn) )
    {
        //initialize the output with minimum rank
        oOutSeamPos = geo2d::Doublearray{1, 0, ValueRankType::eRankMin};
        oOutSeamLeft = geo2d::Doublearray{1, 0, ValueRankType::eRankMin};
        oOutSeamRight = geo2d::Doublearray{1, 0, ValueRankType::eRankMin};
        assert(inputIsInvalid(oOutSeamPos));
        const GeoDoublearray &rSeamPos = GeoDoublearray(rLaserLineIn.context(), oOutSeamPos, rLaserLineIn.analysisResult(), interface::NotPresent);
        const GeoDoublearray &rSeamLeft = GeoDoublearray(rLaserLineIn.context(), oOutSeamLeft, rLaserLineIn.analysisResult(), interface::NotPresent);
        const GeoDoublearray &rSeamRight = GeoDoublearray(rLaserLineIn.context(), oOutSeamRight, rLaserLineIn.analysisResult(), interface::NotPresent);
        const GeoSeamFindingarray &rSeamFindings = GeoSeamFindingarray(rLaserLineIn.context(), oOutSeamFindings, rLaserLineIn.analysisResult(), interface::NotPresent);
        preSignalAction();
        m_pPipeOutSeamPos->signal( rSeamPos );
        m_pPipeOutSeamLeft->signal( rSeamLeft );
        m_pPipeOutSeamRight->signal(rSeamRight);
        m_pPipeOutSeamFindings->signal(rSeamFindings);

        return; // RETURN
    }

    // Now do the actual image processing
    if (m_oMode == 1)
    {
        calcLineWidthDoubleMinimum(rLaserarray, rSeamWidthIn.ref(), oOutSeamPos, oOutSeamLeft, oOutSeamRight, oOutSeamFindings, m_oLaserlineWidth);
    }
    else
    {
        calcLineWidthMinimum(rLaserarray, rSeamWidthIn.ref(), oOutSeamPos, oOutSeamLeft, oOutSeamRight, oOutSeamFindings, m_oLaserlineWidth);
    }

    // Create a new byte array, and put the global context into the resulting profile
    const auto oAnalysisResult	= rLaserLineIn.analysisResult() == AnalysisOK ? AnalysisOK : rLaserLineIn.analysisResult(); // replace 2nd AnalysisOK by your result type
    const GeoDoublearray &rGeoSeamPos = GeoDoublearray(rLaserLineIn.context(), oOutSeamPos, oAnalysisResult, filter::eRankMax );
    const GeoDoublearray &rGeoSeamLeft = GeoDoublearray(rLaserLineIn.context(), oOutSeamLeft, oAnalysisResult, filter::eRankMax );
    const GeoDoublearray &rGeoSeamRight = GeoDoublearray(rLaserLineIn.context(), oOutSeamRight, oAnalysisResult, filter::eRankMax );
    const GeoSeamFindingarray &rSeamFindings = GeoSeamFindingarray(rLaserLineIn.context(), oOutSeamFindings, rLaserLineIn.analysisResult(), interface::NotPresent);

    preSignalAction();
    m_pPipeOutSeamPos->signal( rGeoSeamPos );
    m_pPipeOutSeamLeft->signal( rGeoSeamLeft );
    m_pPipeOutSeamRight->signal(rGeoSeamRight);
    m_pPipeOutSeamFindings->signal(rSeamFindings);

} // proceedGroup


// Searches for a min. If it has at least a vertical difference of
// "MinYDistance" pixel to the max, the found "min" is a good min.
void LineWidthMinimum::calcLineWidthMinimum( const geo2d::VecDoublearray & p_rLaserLineIn,
                                             const Doublearray           & p_rSeamWidth,
                                             Doublearray                 & p_rSeamPosOut,
                                             Doublearray                 & p_rSeamLeftOut,
                                             Doublearray                 & p_rSeamRightOut,
                                             SeamFindingarray            seamFindingArrayOut,
                                             geo2d::VecDoublearray       & p_oLaserlineWidth
                                           )
{
    const unsigned int oNbLines	= p_rLaserLineIn.size();
    p_oLaserlineWidth.resize(p_rLaserLineIn.size());

    try
    {
        for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
        {
            // loop over N lines
            int index = lineN > p_rSeamWidth.size() - 1 ? p_rSeamWidth.size() - 1 : lineN;
            int curSeamWidth = (int) (0.5 + p_rSeamWidth.getData()[index]);

            const auto& rLaserLineIn_Data = p_rLaserLineIn[lineN].getData();
            //const auto& rLaserLineIn_Rank = p_rLaserLineIn[lineN].getRank();

            int minX = 0;
            int minSum = 1000000;
            int maxSum = 0;
            int curWindowSum = 0;

            // Calculate start value for filter
            {
                for (int index = 0; index < m_oFilterLength; index++)
                {
                    curWindowSum += (int) rLaserLineIn_Data[index];
                }

                if (curWindowSum < minSum)
                {
                    minSum = curWindowSum;
                }
                if (curWindowSum > maxSum)
                {
                    maxSum = curWindowSum;
                }
            }

            // Save 'start value' for paint routine
            auto &rWidthVectorOut = p_oLaserlineWidth[lineN].getData();
            auto &rRankVectorOut  = p_oLaserlineWidth[lineN].getRank();
            int indexOutVector = 0;

            for (; indexOutVector < m_oFilterLength / 2; indexOutVector++)
            {
                rWidthVectorOut[indexOutVector] = curWindowSum / m_oFilterLength;
                rRankVectorOut[indexOutVector]  = eRankMax;
            }

            // Now check the laser line
            for (int x = 0; x < (int) rLaserLineIn_Data.size() - m_oFilterLength; x++)
            {
                curWindowSum -= (int) rLaserLineIn_Data[x];                 // Remove first filter element
                curWindowSum += (int) rLaserLineIn_Data[x + m_oFilterLength];  // Next 'new' laser line point

                if (curWindowSum < minSum)
                {
                    // Actual 'x' is left border of filter!
                    minX = x + m_oFilterLength / 2;
                    minSum = curWindowSum;
                }

                if (curWindowSum > maxSum)
                {
                    maxSum = curWindowSum;
                }

                rWidthVectorOut[indexOutVector] = curWindowSum / m_oFilterLength;
                rRankVectorOut[indexOutVector]  = eRankMax;
                indexOutVector++;
            }

            // Fill rest of result vector for paint routine
            for (; indexOutVector < (int) rLaserLineIn_Data.size(); indexOutVector++)
            {
                rWidthVectorOut[indexOutVector] = curWindowSum / m_oFilterLength;
                rRankVectorOut[indexOutVector]  = eRankMax;
            }

            if (    (minX != 0)                                                  // Min found ?
                 && ( (maxSum - minSum) / m_oFilterLength >= m_oMinYDistance )   // y distance big enough?
               )
            {
                m_resultSeamLeft = minX - curSeamWidth / 2;   // Use 'Default SeamWidth'!
                m_resultSeamPos = minX;
                m_resultSeamRight = minX + curSeamWidth / 2;  // Use 'Default SeamWidth'!
                m_IsMinOk = true;
                seamFindingArrayOut.getData().push_back(SeamFinding(m_resultSeamLeft, m_resultSeamRight, 255, 1));
                seamFindingArrayOut.getRank().push_back(255);
                p_rSeamPosOut.getData().push_back(minX);
                p_rSeamPosOut.getRank().push_back(255);
                p_rSeamLeftOut.getData().push_back(minX - curSeamWidth / 2);   // Use 'Default SeamWidth'!
                p_rSeamLeftOut.getRank().push_back(255);
                p_rSeamRightOut.getData().push_back(minX + curSeamWidth / 2);  // Use 'Default SeamWidth'!
                p_rSeamRightOut.getRank().push_back(255);
            }
            else if (minX != 0)    // Min found ?
            {
                // y distance is too small! Set 'bad rank'
                m_resultSeamLeft = minX - curSeamWidth / 2;   // Use 'Default SeamWidth'!
                m_resultSeamPos = minX;
                m_resultSeamRight = minX + curSeamWidth / 2;  // Use 'Default SeamWidth'!
                m_IsMinOk = false;
                seamFindingArrayOut.getData().push_back(SeamFinding(m_resultSeamLeft, m_resultSeamRight, 0, 1));
                seamFindingArrayOut.getRank().push_back(0);
                p_rSeamPosOut.getData().push_back(minX);
                p_rSeamPosOut.getRank().push_back(0);
                p_rSeamLeftOut.getData().push_back(minX - curSeamWidth / 2);   // Use 'Default SeamWidth'!
                p_rSeamLeftOut.getRank().push_back(0);
                p_rSeamRightOut.getData().push_back(minX + curSeamWidth / 2);  // Use 'Default SeamWidth'!
                p_rSeamRightOut.getRank().push_back(0);
            }
            else // No min found
            {
                m_resultSeamLeft = 0;
                m_resultSeamPos = 0;
                m_resultSeamRight = 0;
                m_IsMinOk = false;

                seamFindingArrayOut.getData().push_back(SeamFinding(m_resultSeamLeft, m_resultSeamRight, 0, 1));
                seamFindingArrayOut.getRank().push_back(0);

                p_rSeamPosOut.getData().push_back(0);
                p_rSeamPosOut.getRank().push_back(0);
                p_rSeamLeftOut.getData().push_back(0);
                p_rSeamLeftOut.getRank().push_back(0);
                p_rSeamRightOut.getData().push_back(0);
                p_rSeamRightOut.getRank().push_back(0);
            }

        } // for
    }
    catch(...)
    {
        seamFindingArrayOut = geo2d::SeamFindingarray{1, SeamFinding(0, 0, 0, 1), ValueRankType::eRankMin};
        p_rSeamPosOut = geo2d::Doublearray{1, 0, ValueRankType::eRankMin};
        p_rSeamLeftOut = geo2d::Doublearray{1, 0, ValueRankType::eRankMin};
        p_rSeamRightOut = geo2d::Doublearray{1, 0, ValueRankType::eRankMin};
    }
} // calcLineWidthMinimum


// Searches for lowest and 2nd lowest min. If they have at least a vertical difference of
// "MinYDistance" pixel, the found "lowest min" is a good min.
void LineWidthMinimum::calcLineWidthDoubleMinimum( const geo2d::VecDoublearray & p_rLaserLineIn,
                                                   const Doublearray           & p_rSeamWidth,
                                                   Doublearray                 & p_rSeamPosOut,
                                                   Doublearray                 & p_rSeamLeftOut,
                                                   Doublearray                 & p_rSeamRightOut,
                                                   SeamFindingarray            seamFindingArrayOut,
                                                   geo2d::VecDoublearray       & p_oLaserlineWidth
                                                 )
{
    const unsigned int oNbLines	= p_rLaserLineIn.size();
    p_oLaserlineWidth.resize(p_rLaserLineIn.size());

    try
    {
        for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
        {
            // loop over N lines
            int index = lineN > p_rSeamWidth.size() - 1 ? p_rSeamWidth.size() - 1 : lineN;
            int curSeamWidth = (int) (0.5 + p_rSeamWidth.getData()[index]);

            const auto& rLaserLineIn_Data = p_rLaserLineIn[lineN].getData();
            //const auto& rLaserLineIn_Rank = p_rLaserLineIn[lineN].getRank();

            int minX = 0;
            m_minX_1 = 0;
            m_minX_2 = 0;
            int minSum   = 1000000;
            m_minSum_1 = 1000000;
            m_minSum_2 = 1000000;
            int minFoundSum = 0;
            int maxSum = 0;
            int curWindowSum = 0;

            // Variables for min search
            enum MinSearchState
            {
                eStartMin   = 0,
                eCheckMin1  = 1,
                eMin1Found  = 2,
                eCheckMin2  = 3,
                eMin2Found  = 4,
                eCheckMin3  = 5
            };

            MinSearchState eActualMinState = eStartMin;

            // Calculate start value for filter
            {
                for (int index = 0; index < m_oFilterLength; index++)
                {
                    curWindowSum += (int) rLaserLineIn_Data[index];
                }

                if (curWindowSum < minSum)
                {
                    minSum = curWindowSum;
                }
                if (curWindowSum > maxSum)
                {
                    maxSum = curWindowSum;
                }
            }

            // Save 'start value' for paint routine
            auto &rWidthVectorOut = p_oLaserlineWidth[lineN].getData();
            auto &rRankVectorOut  = p_oLaserlineWidth[lineN].getRank();
            int indexOutVector = 0;
            const int checkGoodMinDistance = m_oFilterLength * m_oMinYDistance;

            for (; indexOutVector < m_oFilterLength / 2; indexOutVector++)
            {
                rWidthVectorOut[indexOutVector] = curWindowSum / m_oFilterLength;
                rRankVectorOut[indexOutVector]  = eRankMax;
            }

            // Now check the laser line
            for (int x = 0; x < (int) rLaserLineIn_Data.size() - m_oFilterLength; x++)
            {
                curWindowSum -= (int) rLaserLineIn_Data[x];                    // Remove first filter element
                curWindowSum += (int) rLaserLineIn_Data[x + m_oFilterLength];  // Next 'new' laser line point

                if (eActualMinState == eStartMin)
                {
                    if ( curWindowSum < (maxSum - checkGoodMinDistance) )   // Good 'start min' found
                    {
                        // Actual 'x' is left border of filter!
                        minX = x + m_oFilterLength / 2;
                        minSum = curWindowSum;
                        eActualMinState = eCheckMin1;
                    }
                }
                else if (eActualMinState == eCheckMin1)
                {
                    if (curWindowSum < minSum)   // New 'min' found
                    {
                        // Actual 'x' is left border of filter!
                        minX = x + m_oFilterLength / 2;
                        minSum = curWindowSum;
                    }
                    else if ( (curWindowSum - minSum) >= checkGoodMinDistance )
                    {
                        // Act value is 'big'  =>  min is a 'good' min
                        m_minX_1 = minX;
                        m_minSum_1 = minSum;
                        minFoundSum = minSum + checkGoodMinDistance;
                        eActualMinState = eMin1Found;
                    }
                }
                else if (eActualMinState == eMin1Found)
                {
                    if (curWindowSum < minFoundSum)   // Start of new 'min' found
                    {
                        // Actual 'x' is left border of filter!
                        minX = x + m_oFilterLength / 2;
                        minSum = curWindowSum;
                        eActualMinState = eCheckMin2;
                    }
                    else if (curWindowSum > minFoundSum)   // New 'reference' value
                    {
                        minFoundSum = curWindowSum;
                    }
                }
                else if (eActualMinState == eCheckMin2)
                {
                    if (curWindowSum < minSum)   // New 'min' found
                    {
                        // Actual 'x' is left border of filter!
                        minX = x + m_oFilterLength / 2;
                        minSum = curWindowSum;
                    }
                    else if ( (curWindowSum - minSum) >= checkGoodMinDistance )
                    {
                        // Act value is 'big'  =>  min is a 'good' min
                        m_minX_2 = minX;
                        m_minSum_2 = minSum;
                        minFoundSum = minSum + checkGoodMinDistance;
                        eActualMinState = eMin2Found;
                    }
                }
                else if (eActualMinState == eMin2Found)
                {
                    if (curWindowSum < minFoundSum)   // Start of new 'min' found
                    {
                        // Actual 'x' is left border of filter!
                        minX = x + m_oFilterLength / 2;
                        minSum = curWindowSum;
                        eActualMinState = eCheckMin3;
                    }
                    else if (curWindowSum > minFoundSum)   // New 'reference' value
                    {
                        minFoundSum = curWindowSum;
                    }
                }
                else if (eActualMinState == eCheckMin3)
                {
                    if (curWindowSum < minSum)   // New 'min' found
                    {
                        // Actual 'x' is left border of filter!
                        minX = x + m_oFilterLength / 2;
                        minSum = curWindowSum;
                    }
                    else if ( (curWindowSum - minSum) >= checkGoodMinDistance )
                    {
                        // Act value is 'big'  =>  min is a 'good' min

                        // Check if this actual min is lower than min1 or min2
                        if ( (minSum < m_minSum_1) || (minSum < m_minSum_2) )
                        {
                            // Replace the worse min  =>  make better min as min1
                            if (m_minSum_1 >= m_minSum_2)
                            {
                                // Replace min1 by min2
                                m_minX_1 = m_minX_2;
                                m_minSum_1 = m_minSum_2;
                            }

                            // Set actual min as new 'min2'
                            m_minX_2 = minX;
                            m_minSum_2 = minSum;
                            minFoundSum = minSum + checkGoodMinDistance;
                            eActualMinState = eMin2Found;
                        }
                        else   // New 'min' is not a better min  =>  forget it !!
                        {
                            minFoundSum = minSum + checkGoodMinDistance;
                            eActualMinState = eMin2Found;
                        }
                    }
                }

                if (curWindowSum > maxSum)
                {
                    maxSum = curWindowSum;
                }

                rWidthVectorOut[indexOutVector] = curWindowSum / m_oFilterLength;
                rRankVectorOut[indexOutVector]  = eRankMax;
                indexOutVector++;
            }

            // Fill rest of result vector for paint routine
            for (; indexOutVector < (int) rLaserLineIn_Data.size(); indexOutVector++)
            {
                rWidthVectorOut[indexOutVector] = curWindowSum / m_oFilterLength;
                rRankVectorOut[indexOutVector]  = eRankMax;
            }

            // Check end state
            if (eActualMinState == eStartMin)
            {
                m_resultSeamLeft = 0;
                m_resultSeamPos = 0;
                m_resultSeamRight = 0;
                m_IsMinOk = false;
                m_minX_1 = 0;
                m_minX_2 = 0;

                seamFindingArrayOut.getData().push_back(SeamFinding(m_resultSeamLeft, m_resultSeamRight, 0, 1));
                seamFindingArrayOut.getRank().push_back(0);

                p_rSeamPosOut.getData().push_back(0);
                p_rSeamPosOut.getRank().push_back(0);
                p_rSeamLeftOut.getData().push_back(0);
                p_rSeamLeftOut.getRank().push_back(0);
                p_rSeamRightOut.getData().push_back(0);
                p_rSeamRightOut.getRank().push_back(0);
            }
            else if (eActualMinState == eCheckMin1)
            {
                m_resultSeamLeft = minX - curSeamWidth / 2;   // Use 'Default SeamWidth'!
                m_resultSeamPos = minX;
                m_resultSeamRight = minX + curSeamWidth / 2;  // Use 'Default SeamWidth'!
                m_IsMinOk = false;
                m_minX_1 = minX;
                m_minX_2 = 0;
                seamFindingArrayOut.getData().push_back(SeamFinding(m_resultSeamLeft, m_resultSeamRight, 0, 1));
                seamFindingArrayOut.getRank().push_back(0);
                p_rSeamPosOut.getData().push_back(minX);
                p_rSeamPosOut.getRank().push_back(0);
                p_rSeamLeftOut.getData().push_back(minX - curSeamWidth / 2);   // Use 'Default SeamWidth'!
                p_rSeamLeftOut.getRank().push_back(0);
                p_rSeamRightOut.getData().push_back(minX + curSeamWidth / 2);  // Use 'Default SeamWidth'!
                p_rSeamRightOut.getRank().push_back(0);
            }
            else if (eActualMinState == eMin1Found)
            {
                m_resultSeamLeft = m_minX_1 - curSeamWidth / 2;   // Use 'Default SeamWidth'!
                m_resultSeamPos = m_minX_1;
                m_resultSeamRight = m_minX_1 + curSeamWidth / 2;  // Use 'Default SeamWidth'!
                m_IsMinOk = true;
                m_minX_2 = 0;
                seamFindingArrayOut.getData().push_back(SeamFinding(m_resultSeamLeft, m_resultSeamRight, 255, 1));
                seamFindingArrayOut.getRank().push_back(255);
                p_rSeamPosOut.getData().push_back(m_minX_1);
                p_rSeamPosOut.getRank().push_back(255);
                p_rSeamLeftOut.getData().push_back(m_minX_1 - curSeamWidth / 2);   // Use 'Default SeamWidth'!
                p_rSeamLeftOut.getRank().push_back(255);
                p_rSeamRightOut.getData().push_back(m_minX_1 + curSeamWidth / 2);  // Use 'Default SeamWidth'!
                p_rSeamRightOut.getRank().push_back(255);
            }
            else if (eActualMinState == eCheckMin2)
            {
                m_resultSeamLeft = m_minX_1 - curSeamWidth / 2;   // Use 'Default SeamWidth'!
                m_resultSeamPos = m_minX_1;
                m_resultSeamRight = m_minX_1 + curSeamWidth / 2;  // Use 'Default SeamWidth'!
                m_IsMinOk = true;
                m_minX_2 = 0;
                seamFindingArrayOut.getData().push_back(SeamFinding(m_resultSeamLeft, m_resultSeamRight, 255, 1));
                seamFindingArrayOut.getRank().push_back(255);
                p_rSeamPosOut.getData().push_back(m_minX_1);
                p_rSeamPosOut.getRank().push_back(255);
                p_rSeamLeftOut.getData().push_back(m_minX_1 - curSeamWidth / 2);   // Use 'Default SeamWidth'!
                p_rSeamLeftOut.getRank().push_back(255);
                p_rSeamRightOut.getData().push_back(m_minX_1 + curSeamWidth / 2);  // Use 'Default SeamWidth'!
                p_rSeamRightOut.getRank().push_back(255);
            }
            else if (eActualMinState == eMin2Found)
            {
                // Check size of minima, make min_1 the 'better' one
                if (m_minSum_2 < m_minSum_1)
                {
                    minSum = m_minSum_1;
                    m_minSum_1 = m_minSum_2;
                    m_minSum_2 = minSum;
                    minSum = m_minX_1;
                    m_minX_1 = m_minX_2;
                    m_minX_2 = minSum;
                }
                m_resultSeamLeft = m_minX_1 - curSeamWidth / 2;   // Use 'Default SeamWidth'!
                m_resultSeamPos = m_minX_1;
                m_resultSeamRight = m_minX_1 + curSeamWidth / 2;  // Use 'Default SeamWidth'!
                // Distance between the 2 min's big enough?
                if ( m_minSum_1 <= (m_minSum_2 - m_oMinYDistance) )
                {
                    m_IsMinOk = true;
                    seamFindingArrayOut.getData().push_back(SeamFinding(m_resultSeamLeft, m_resultSeamRight, 255, 1));
                    seamFindingArrayOut.getRank().push_back(255);
                    p_rSeamPosOut.getData().push_back(m_minX_1);
                    p_rSeamPosOut.getRank().push_back(255);
                    p_rSeamLeftOut.getData().push_back(m_minX_1 - curSeamWidth / 2);   // Use 'Default SeamWidth'!
                    p_rSeamLeftOut.getRank().push_back(255);
                    p_rSeamRightOut.getData().push_back(m_minX_1 + curSeamWidth / 2);  // Use 'Default SeamWidth'!
                    p_rSeamRightOut.getRank().push_back(255);
                }
                else
                {
                    m_IsMinOk = false;
                    seamFindingArrayOut.getData().push_back(SeamFinding(m_resultSeamLeft, m_resultSeamRight, 255, 1));
                    seamFindingArrayOut.getRank().push_back(0);
                    p_rSeamPosOut.getData().push_back(m_minX_1);
                    p_rSeamPosOut.getRank().push_back(0);
                    p_rSeamLeftOut.getData().push_back(m_minX_1 - curSeamWidth / 2);   // Use 'Default SeamWidth'!
                    p_rSeamLeftOut.getRank().push_back(0);
                    p_rSeamRightOut.getData().push_back(m_minX_1 + curSeamWidth / 2);  // Use 'Default SeamWidth'!
                    p_rSeamRightOut.getRank().push_back(0);
                }
            }
            else if (eActualMinState == eCheckMin3)
            {
                // Check size of minima, make min_1 the 'better' one
                if (m_minSum_2 < m_minSum_1)
                {
                    minSum = m_minSum_1;
                    m_minSum_1 = m_minSum_2;
                    m_minSum_2 = minSum;
                    minSum = m_minX_1;
                    m_minX_1 = m_minX_2;
                    m_minX_2 = minSum;
                }
                m_resultSeamLeft = m_minX_1 - curSeamWidth / 2;   // Use 'Default SeamWidth'!
                m_resultSeamPos = m_minX_1;
                m_resultSeamRight = m_minX_1 + curSeamWidth / 2;  // Use 'Default SeamWidth'!
                // Distance between the 2 min's big enough?
                if ( m_minSum_1 <= (m_minSum_2 - m_oMinYDistance) )
                {
                    m_IsMinOk = true;
                    seamFindingArrayOut.getData().push_back(SeamFinding(m_resultSeamLeft, m_resultSeamRight, 255, 1));
                    seamFindingArrayOut.getRank().push_back(255);
                    p_rSeamPosOut.getData().push_back(m_minX_1);
                    p_rSeamPosOut.getRank().push_back(255);
                    p_rSeamLeftOut.getData().push_back(m_minX_1 - curSeamWidth / 2);   // Use 'Default SeamWidth'!
                    p_rSeamLeftOut.getRank().push_back(255);
                    p_rSeamRightOut.getData().push_back(m_minX_1 + curSeamWidth / 2);  // Use 'Default SeamWidth'!
                    p_rSeamRightOut.getRank().push_back(255);
                }
                else
                {
                    m_IsMinOk = false;
                    seamFindingArrayOut.getData().push_back(SeamFinding(m_resultSeamLeft, m_resultSeamRight, 255, 1));
                    seamFindingArrayOut.getRank().push_back(0);
                    p_rSeamPosOut.getData().push_back(m_minX_1);
                    p_rSeamPosOut.getRank().push_back(0);
                    p_rSeamLeftOut.getData().push_back(m_minX_1 - curSeamWidth / 2);   // Use 'Default SeamWidth'!
                    p_rSeamLeftOut.getRank().push_back(0);
                    p_rSeamRightOut.getData().push_back(m_minX_1 + curSeamWidth / 2);  // Use 'Default SeamWidth'!
                    p_rSeamRightOut.getRank().push_back(0);
                }
            }
            else
            {
                wmLog(eWarning, "Filter 'lineWidthMinimum' > 'Mode' 1 > eActualMinState %d: bad state!\n", eActualMinState);
            }

        } // for
    }
    catch(...)
    {
        seamFindingArrayOut = geo2d::SeamFindingarray{1, SeamFinding(0, 0, 0, 1), ValueRankType::eRankMin};
        p_rSeamPosOut = geo2d::Doublearray{1, 0, ValueRankType::eRankMin};
        p_rSeamLeftOut = geo2d::Doublearray{1, 0, ValueRankType::eRankMin};
        p_rSeamRightOut = geo2d::Doublearray{1, 0, ValueRankType::eRankMin};
    }
} // calcLineWidthDoubleMinimum


} // namespace precitec
} // namespace filter
