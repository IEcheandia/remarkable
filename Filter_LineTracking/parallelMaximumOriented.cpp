/*!
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			LB
 *  @date			2018
 *  @file
 */

#include "parallelMaximumOriented.h"

#include <system/platform.h>			///< global and platform specific defines
#include <system/tools.h>				///< debug assert integrity assumptions
#include "module/moduleLogger.h"
#include "math/2D/LineEquation.h"

#include "overlay/overlayPrimitive.h"	///< paint overlay

#include <algorithm>					///< for_each
#include <functional>					///< bind
#include <tuple>						///< pair
#include <fliplib/TypeToDataTypeImpl.h>

namespace
{
//Utility class to hold the candidate points during the parallel maximum algorithm
class MaximumInSearchLine
{
public:
    MaximumInSearchLine(int pMaxLineWidth, unsigned int pThreshold)
        : mMaxLineWidth(pMaxLineWidth > 0 ? pMaxLineWidth : 0)
        , mThreshold(pThreshold)
    {
    }
    void resetSearchLine()
    {
        mIndexFirst = 0;
        mIndexLast = 0;
        mMaxFirst = std::numeric_limits<byte>::min();
    }
    void addPointOfSearchLine(byte pCurrentIntensity, int pIndex)
    {
        if ( pCurrentIntensity > mMaxFirst )
        {
            mMaxFirst = pCurrentIntensity;
            mIndexFirst = pIndex;
            mIndexLast = pIndex;
        }
        // second maximum found if value remains equal first maximum
        else if ( pCurrentIntensity == mMaxFirst )
        {
            // Falls dieser Pixel ausserdem in der nachbarschaft liegt
            if ( std::abs(mIndexFirst - pIndex) <= mMaxLineWidth )
            {
                mIndexLast = pIndex;
            }
        } // if
    }

    std::tuple<double, precitec::filter::ValueRankType> processSearchLine()
    {
        if ( mMaxFirst > mThreshold )
        {
            return std::make_tuple(static_cast<double> (mIndexFirst + mIndexLast) / 2.0,
                precitec::filter::ValueRankType::eRankMax);
        }
        return std::make_tuple(0.0, precitec::filter::ValueRankType::eRankMin);
    }
private:
    const int mMaxLineWidth;
    const unsigned int mThreshold;
    byte mMaxFirst;
    int mIndexFirst;
    int mIndexLast;
};
}


using namespace fliplib;
namespace precitec {
    using namespace interface;
    using namespace image;
    using namespace geo2d;
namespace filter {

const std::string ParallelMaximumOriented::m_oFilterName 	= std::string("ParallelMaximumOriented");
const std::string ParallelMaximumOriented::m_oPipeOutNameLine = std::string("Line");
const std::string ParallelMaximumOriented::m_oPipeOutNameLineModel = std::string("LineModel");


ParallelMaximumOriented::ParallelMaximumOriented() :
    TransformFilter( ParallelMaximumOriented::m_oFilterName, Poco::UUID{"996b9b2f-0dd9-46eb-bf3e-2d6340df9b4c"} ),
    m_pPipeInImageFrame	(nullptr),
    m_LinePipeOut(this, m_oPipeOutNameLine),
    m_LineEquationPipeOut(this, m_oPipeOutNameLineModel),
    m_oLineOut			(1), // 1 profile from 1 image
    m_oLineOutTransposed(1),
    m_oLineModelOut(1),
    m_oResX				(10),
    m_oResY				(1),
    m_oThreshold		(230),
    m_oFineTrackingDilation(5),
    m_oIsFineTracking	(true),
    m_oMaxLineWidth     (20),
    m_oMainDirectionHorizontal(true),
    m_oPaint            (false)
{
    // Defaultwerte der Parameter setzen
    parameters_.add("ResX",				Parameter::TYPE_UInt32, m_oResX);
    parameters_.add("ResY",				Parameter::TYPE_UInt32, m_oResY);
    parameters_.add("Threshold",		Parameter::TYPE_UInt32, m_oThreshold);
    parameters_.add("FineTrackingDilation", Parameter::TYPE_UInt32, m_oFineTrackingDilation);
    parameters_.add("IsFineTracking",	Parameter::TYPE_bool,	m_oIsFineTracking);
    parameters_.add("MaxLineWidth", 	Parameter::TYPE_UInt32,	m_oMaxLineWidth);
    parameters_.add("MainDirectionHorizontal", Parameter::TYPE_bool, m_oMainDirectionHorizontal);
    parameters_.add("HandleTransposeInContext", Parameter::TYPE_bool, m_oHandleTransposeInContext);

    setInPipeConnectors({{Poco::UUID("f0938461-f989-473a-88c5-926c9682a8fc"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("988cdf68-c573-421c-bd27-23283f3df7ff"), &m_LinePipeOut, m_oPipeOutNameLine, 0, ""},
    {Poco::UUID("03d0e0f7-9b48-4e11-bd1c-2314b5b581bd"), &m_LineEquationPipeOut, m_oPipeOutNameLineModel, 0, ""}});
    setVariantID(Poco::UUID("dadfd8ec-9a68-4566-819c-e07c110b6456"));
} // ParallelMaximumOriented



void ParallelMaximumOriented::setParameter() {
    TransformFilter::setParameter();
    m_oResX				= parameters_.getParameter("ResX").convert<std::uint32_t>();
    m_oResY				= parameters_.getParameter("ResY").convert<std::uint32_t>();
    m_oThreshold		= parameters_.getParameter("Threshold").convert<std::uint32_t>();
    m_oFineTrackingDilation = parameters_.getParameter("FineTrackingDilation").convert<std::uint32_t>();
    m_oIsFineTracking	= parameters_.getParameter("IsFineTracking").convert<bool>();
    m_oMaxLineWidth	    = parameters_.getParameter("MaxLineWidth");
    m_oMainDirectionHorizontal = parameters_.getParameter("MainDirectionHorizontal").convert<bool>();
    m_oHandleTransposeInContext = parameters_.getParameter("HandleTransposeInContext").convert<bool>();
} // setParameter



void ParallelMaximumOriented::paint()
{
    if(m_oVerbosity <= eNone)
    {
        return;
    }
    if (!m_oPaint)
    {
        return;
    }
    if (m_oSpTrafo.isNull())
    {
        return;
    }


    const Trafo		&rTrafo				( *m_oSpTrafo );
    OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
    OverlayLayer	&rLayerContour		( rCanvas.getLayerContour());
    OverlayLayer	&rLayerLine			( rCanvas.getLayerLine());
    OverlayLayer & rLayerImage (rCanvas.getLayerImage());

    auto numPoints = m_oLineOut.front().size();

    if (!m_oMainDirectionHorizontal && m_oHandleTransposeInContext)
    {
        const auto&	rLaserLineX(m_oLineOut.front().getData());
        for ( unsigned int i = 0; i != rLaserLineX.size(); ++i )
        {
            rLayerContour.add<OverlayPoint>(rTrafo(Point(int(rLaserLineX[i] + 0.5), i)), Color::Orange());
        }
    }
    else
    {
        const auto&	rLaserLineY(m_oLineOut.front().getData());
        rLayerContour.add<OverlayPointList>(Point{rTrafo.dx(), rTrafo.dy()}, rLaserLineY, Color(0x1E90FF)); // dodgerblue

        const auto&	rLaserLineX(m_oLineOutTransposed.front().getData());
        for ( unsigned int i = 0; i != rLaserLineX.size(); ++i )
        {
            rLayerContour.add<OverlayPoint>(rTrafo(Point(int(rLaserLineX[i] + 0.5), i)), Color::Orange());
        }
    }

    if (m_oPaintTransposedImage.isValid())
    {
        rLayerImage.add<OverlayImage>(rTrafo(Point(0,0)), m_oPaintTransposedImage,
                OverlayText{"Par Max Transposed Image", Font(), Rect(150, 18), Color::Orange()} );
    }

    if(m_oVerbosity < eHigh)
    {
        return;
    }
    auto fScreenPoint = [this] (const double & r_X, const double & r_Y)
    {
        //cast to int and apply trafo
        return m_oSpTrafo->apply(geo2d::Point(int(std::round(r_X)), int(std::round(r_Y))));
    };

    const auto & rLineModelOut = m_oLineModelOut.getData().front();
    double a, b, c;
    rLineModelOut.getCoefficients(a, b, c);
    math::LineEquation oLineEquation(a, b, c);
    if ( oLineEquation.isValid() )
    {
        geo2d::DPoint oSegmentCenter = rLineModelOut.getCenter();
        geo2d::DPoint oStart, oEnd;
        const double oPaintLineLength = static_cast<double>(numPoints);
        oLineEquation.get2Points(oStart.x, oStart.y, oEnd.x, oEnd.y, oPaintLineLength);
        rLayerContour.add<OverlayLine>(fScreenPoint(oStart.x, oStart.y), fScreenPoint(oEnd.x, oEnd.y), Color::Cyan());
    }

    // draw sub ROIs
    for(auto oIt = std::begin(m_oPatchRois); oIt != std::end(m_oPatchRois); ++oIt)
    {
        rLayerLine.add<OverlayRectangle>(rTrafo(*oIt), Color::Blue());
    } // for

} // paint



bool ParallelMaximumOriented::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    m_pPipeInImageFrame  = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);
    return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void ParallelMaximumOriented::proceed(const void* sender, PipeEventArgs& e)
{
    poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

    // get data from frame
    m_oPaint = false; // needs to be at the very top to make sure paint will not be called when errors occur!

    const auto oFrameIn = ( m_pPipeInImageFrame->read(m_oCounter) );
    m_oSpTrafo	= oFrameIn.context().trafo();
    const BImage &rImageIn = oFrameIn.data();

    if ( oFrameIn.analysisResult() != interface::AnalysisOK || !rImageIn.isValid() )
    {
        const double oNewRank = interface::NotPresent; // bad rank

        const GeoVecDoublearray geoLineOut(oFrameIn.context(), m_oLineOut, oFrameIn.analysisResult(), oNewRank);
        const GeoLineModelarray getOutLineModel(oFrameIn.context(), m_oLineModelOut, oFrameIn.analysisResult(), oNewRank);

        preSignalAction();
        m_LinePipeOut.signal(geoLineOut);
        m_LineEquationPipeOut.signal(getOutLineModel);
        return;
    }


    const int	oImgWidth	= rImageIn.width();
    const int   oImgHeight  = rImageIn.height();
    assert(m_oLineOut.size() == 1);
    assert(m_oLineOutTransposed.size() == 1);

    bool oIgnoreLineOutTransposed = (m_oMainDirectionHorizontal || m_oHandleTransposeInContext);

    auto oLineOutLength = oImgWidth;
    auto oLineOutTransposedLength = 0;
    if ( oIgnoreLineOutTransposed )
    {
        if ( m_oMainDirectionHorizontal)
        {
            //reinitialize m_oLineOut with img width as length, zeros and bad rank
            oLineOutLength = oImgWidth;
        }
        else
        {
            assert(!m_oMainDirectionHorizontal && m_oHandleTransposeInContext);
            oLineOutLength = oImgHeight;
        }
        oLineOutTransposedLength = 0;
    }
    else
    {
        //reinitialize m_oLineOutTransposed (laser line with vertical direction)
        oLineOutLength = oImgWidth;
        oLineOutTransposedLength = oImgHeight;
    }
    if (!m_oMainDirectionHorizontal && m_oHandleTransposeInContext && m_oVerbosity > eLow)
    {
        transposeImage(m_oPaintTransposedImage, rImageIn);
    }
    else
    {
        m_oPaintTransposedImage.clear();
    }

    m_oLineOut.front().assign(oLineOutLength, 0, eRankMin);
    if (!oIgnoreLineOutTransposed)
    {
        m_oLineOutTransposed.front().assign(oLineOutTransposedLength, 0, eRankMin);
    }

    //image processing: fill the laser line container with the parMax algorithm
    geo2d::VecDoublearray & r_lineParMax = oIgnoreLineOutTransposed ? m_oLineOut : m_oLineOutTransposed;
    const int oResolutionMainDirection = m_oMainDirectionHorizontal ? m_oResX : m_oResY;

    parMax(rImageIn, m_oResX, m_oResY, m_oThreshold, m_oMaxLineWidth, m_oMainDirectionHorizontal,
             r_lineParMax, Point{0, 0}, rImageIn.size());

    if ( (oResolutionMainDirection > 1) && m_oIsFineTracking )
    { // fill gaps with fine track if subsampled and fine track enabled
        m_oPatchRois = fineTrack(rImageIn, r_lineParMax, m_oFineTrackingDilation, m_oThreshold, m_oMaxLineWidth,
            oResolutionMainDirection, m_oMainDirectionHorizontal, m_oVerbosity >= eHigh);
    }


    // compute the output fitted line
    m_oLineModelOut.assign(1, fitLine(r_lineParMax.front(), m_oMainDirectionHorizontal), 1.0);

    auto oLineOutContext = oFrameIn.context();
    if ( !m_oMainDirectionHorizontal )
    {
        if ( oIgnoreLineOutTransposed )
        {
            assert(&r_lineParMax == & m_oLineOut);
            oLineOutContext.m_transposed = !oFrameIn.context().m_transposed;
        }
        else
        {
            //in case of vertical laser line, compute the output laser line container (horizontal, to be compatible with the other filters)
            assert(&r_lineParMax == & m_oLineOutTransposed);
            const auto & rData = m_oLineOutTransposed.front().getData();
            const auto & rRank = m_oLineOutTransposed.front().getRank();

            auto & rLaserVectorOut = m_oLineOut.front().getData();
            auto & rRankVectorOut = m_oLineOut.front().getRank();
            //fill out line only using the valid points (alternativately I could use the interpolate line)
            auto pLine = rData.begin();
            auto pRank = rRank.begin();
            for ( int  y = 0, yMax = m_oLineOutTransposed.front().size();
                y < yMax;
                ++y, ++pLine, ++pRank )
            {
                if ( (*pRank) != eRankMin )
                {
                    int x = static_cast<int>(std::round(*pLine));
                    rLaserVectorOut[x] = y;
                    rRankVectorOut[x] = (*pRank);
                }
            }
        }
    }

    const GeoVecDoublearray geoLineOut(oLineOutContext, m_oLineOut, oFrameIn.analysisResult(), interface::Limit); // full rank, detailed rank in Line
    const GeoLineModelarray geoLineModelOut(oFrameIn.context(), m_oLineModelOut, oFrameIn.analysisResult(), interface::Limit); // full rank, detailed rank in Line
    m_oPaint = true;
    preSignalAction();
    m_LinePipeOut.signal(geoLineOut);
    m_LineEquationPipeOut.signal(geoLineModelOut);
} // proceed

// actual signal processing
/*static*/
void ParallelMaximumOriented::parMax(
    const BImage &p_rImageIn,
    std::uint32_t p_oResX,
    std::uint32_t p_oResY,
    std::uint32_t p_oThreshold,
    std::uint32_t p_oMaxLineWidth,
    bool p_oMainDirectionHorizontal,
    VecDoublearray &p_rLineOut,
    geo2d::Point			p_oOffset,
    geo2d::Size				p_oSize
)
{
    const int oXMax = p_oOffset.x + p_oSize.width;
    const int oYMax = p_oOffset.y + p_oSize.height;
    assert(oXMax <= p_rImageIn.width());
    assert(oYMax <= p_rImageIn.height());

    auto &rLaserVectorOut = p_rLineOut.front().getData();
    auto &rRankVectorOut = p_rLineOut.front().getRank();
    assert(!p_oMainDirectionHorizontal || int(rLaserVectorOut.size()) == p_rImageIn.width());
    assert(p_oMainDirectionHorizontal || int(rLaserVectorOut.size()) == p_rImageIn.height());

    MaximumInSearchLine oSearchLine{static_cast<int>(p_oMaxLineWidth), p_oThreshold};

    if ( p_oMainDirectionHorizontal )
    {
        for ( int x = p_oOffset.x; x < oXMax; x += p_oResX )
        {
            oSearchLine.resetSearchLine();
            for ( int y = p_oOffset.y; y < oYMax; y += p_oResY )
            {
                const byte* pLine = p_rImageIn[y]; // get line pointer
                oSearchLine.addPointOfSearchLine(pLine[x], y);
            }
            std::tie(rLaserVectorOut[x], rRankVectorOut[x]) = oSearchLine.processSearchLine();
        }
    }
    else
    {
        for ( int y = p_oOffset.y; y < oYMax; y += p_oResY )
        {
            oSearchLine.resetSearchLine();
            const byte* pLine = p_rImageIn[y]; // get line pointer
            for ( int x = p_oOffset.x; x < oXMax; x += p_oResX )
            {
                oSearchLine.addPointOfSearchLine(pLine[x], x);
            }
            std::tie(rLaserVectorOut[y], rRankVectorOut[y]) = oSearchLine.processSearchLine();
        }
    }
} // parMax



std::vector<geo2d::Rect> ParallelMaximumOriented::fineTrack(
    const BImage &p_rImageIn,
    VecDoublearray &p_rLaserline,
    std::uint32_t p_oDilation,
    std::uint32_t p_oThreshold,
    std::uint32_t p_oMaxLineWidth,
    std::uint32_t p_oResolution,
    bool p_oHorizontal,
    bool p_returnPaintInfo)

{
    const unsigned int	oNbLines	= p_rLaserline.size();
    std::vector<geo2d::Rect> oPatchRois;

    for ( unsigned int lineN = 0; lineN < oNbLines; ++lineN )
    { // loop over N lines

        auto &rLaserVector	= p_rLaserline[lineN].getData();
        auto &rRankVector	= p_rLaserline[lineN].getRank();
        if ( p_returnPaintInfo )
        {
            oPatchRois.clear();
            oPatchRois.reserve(rLaserVector.size() / p_oResolution);
        } // if

        const Rect oImageIn(p_rImageIn.size());

        std::function<geo2d::Rect(int, int)> fCreatePatch;
        std::function<geo2d::Rect(int, int)> fComputePatch; //used internally by fCreatePatch

        if ( p_oHorizontal )
        {
            fComputePatch = [&p_oDilation, &rLaserVector] (int p_lastIndex, int p_Index)
            {
                Rect oPatchRoi { Point(p_lastIndex, int(rLaserVector[p_lastIndex])),
                                Point(p_Index, int(rLaserVector[p_Index]))};
                // dilate roi in y direction.
                oPatchRoi.y() = oPatchRoi.y().dilate(p_oDilation);
                return oPatchRoi;
            };
        }
        else
        {
            fComputePatch = [&p_oDilation, &rLaserVector] (int p_lastIndex, int p_Index)
            {
                Rect oPatchRoi{Point(int(rLaserVector[p_lastIndex]), p_lastIndex),
                    Point(int(rLaserVector[p_Index]), p_Index)};
                // dilate roi in x direction.
                oPatchRoi.x() = oPatchRoi.x().dilate(p_oDilation);
                return oPatchRoi;
            };
        }

        if ( p_returnPaintInfo )
        {
            fCreatePatch = [&oPatchRois, &fComputePatch, &oImageIn] (int p_lastIndex, int p_Index)
            {
                Rect oPatchRoi = fComputePatch(p_lastIndex, p_Index);
                oPatchRoi = intersect(oImageIn, oPatchRoi); // clip roi if bigger than image
                oPatchRois.push_back(oPatchRoi);
                return oPatchRoi;
            };
        }
        else
        {
            fCreatePatch = [&fComputePatch, &oImageIn] (int p_lastIndex, int p_Index)
            {
                Rect oPatchRoi = fComputePatch(p_lastIndex, p_Index);
                oPatchRoi = intersect(oImageIn, oPatchRoi); // clip roi if bigger than image
                return oPatchRoi;
            };

        }

        //fine tracking inside windows of invalid values
        // get position of first good value - usually the first value from filter ParallelMaximum should be valid
        const auto oItFirstGood = std::find_if(
            rRankVector.begin(),
            rRankVector.end(),
            std::bind2nd(std::not_equal_to<int>(), eRankMin)
        );

        if ( oItFirstGood == rRankVector.end() )
        {
            //all bad rank;
            return oPatchRois;
        }

        int oPreviousValidPointIndex = std::distance(std::begin(rRankVector), oItFirstGood);

        for ( auto itRank = oItFirstGood + 1, itRankEnd = rRankVector.end(); itRank != itRankEnd; ++itRank )
        {
            if ( (*itRank) == eRankMin )
            {
                continue;
            }
            int oCurrentValidPointIndex = itRank - rRankVector.begin();

            assert(oPreviousValidPointIndex < 0 || rRankVector[oPreviousValidPointIndex] == eRankMax);
            assert(rRankVector[oCurrentValidPointIndex] == eRankMax);

            // create subroi between tracked points, exclude borders. If we use it with an image, we'd need +1 to compenasate for -1 from subroi constructor
            Rect oPatchRoi = fCreatePatch(oPreviousValidPointIndex, oCurrentValidPointIndex);
            // track on subimage with full resolution
            ParallelMaximumOriented::parMax(p_rImageIn, 1, 1, p_oThreshold, p_oMaxLineWidth, p_oHorizontal, p_rLaserline,
                {oPatchRoi.x().start(), oPatchRoi.y().start()},
                {oPatchRoi.width(), oPatchRoi.height()}
            );

            oPreviousValidPointIndex = oCurrentValidPointIndex;
        } // for

        //process last subroi: in this case the last point is not defined
        int oLastIndex = rRankVector.size() - 1;
        if ( rRankVector[oLastIndex] == eRankMin )
        {
            //compute the last value in order to be able to call fCreatePatch
            ParallelMaximumOriented::parMax(p_rImageIn, 1, 1, p_oThreshold, p_oMaxLineWidth, p_oHorizontal, p_rLaserline,
                p_oHorizontal ? geo2d::Point{oLastIndex, 0} : geo2d::Point{0, oLastIndex},
                {1, 1});
        }
        Rect oPatchRoi = fCreatePatch(oPreviousValidPointIndex, oLastIndex);

        // track on subimage with full resolution
        ParallelMaximumOriented::parMax(p_rImageIn, 1, 1, p_oThreshold, p_oMaxLineWidth, p_oHorizontal, p_rLaserline,
            {oPatchRoi.x().start(), oPatchRoi.y().start()},
            {oPatchRoi.width(), oPatchRoi.height()}
        );


    } // for

    return oPatchRois;
} // fineTrack


geo2d::LineModel ParallelMaximumOriented::fitLine(geo2d::Doublearray	&p_rLaserline, bool p_oMainDirectionHorizontal)
{
    //u is the index of the geoDoubleArray, v is the corresponding value
    std::vector<double> u, v;
    u.reserve(p_rLaserline.size());
    v.reserve(p_rLaserline.size());

    auto pLine = p_rLaserline.getData().begin();
    auto pRank = p_rLaserline.getRank().begin();

    //fit line only using the valid points
    for ( int i = 0, iMax = p_rLaserline.size();
        i < iMax;
        ++i, ++pLine, ++pRank )
    {
        if ( (*pRank) != eRankMin )
        {
            u.push_back(i);
            v.push_back((*pLine));
        }
    }
    //special case: with less than 2 valid points, we can't fit a line
    //we can't fit a line also when all the points are coincident, but here we don't need to consider it because u has always unique values (by construction)
    if ( u.size() < 2 )
    {
        return{0, 0, 0, 0, 0};
    }

    double oA, oB, oC; //line equation coefficients
    double xc, yc; //center of the segment used to compute the line

    if ( p_oMainDirectionHorizontal )
    {
        math::LineEquation oComputedLine(u, v, p_oMainDirectionHorizontal);
        assert(oComputedLine.isValid());
        oComputedLine.getCoefficients(oA, oB, oC);
        //take the median point as center of the segment (u has at least 2 elements, see above)
        xc = u[u.size()/2];
        yc = oComputedLine.getY(xc);
    }
    else
    {
        //the laser line vector is transposed: use v as x and u as y
        //the flag horizontal in LineEquation it's just a hint for the computation, it does not swap the input
        math::LineEquation oComputedLine(v, u, p_oMainDirectionHorizontal);
        assert(oComputedLine.isValid());
        oComputedLine.getCoefficients(oA, oB, oC);
        //take the median point as center of the segment (u has at least 2 elements, see above)
        yc = u[u.size() / 2];
        xc = oComputedLine.getX(yc);
    }
    return{xc, yc, oA, oB, oC};

}


} // namespace filter
} // namespace precitec
