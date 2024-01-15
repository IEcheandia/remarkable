#include "startEndDetectionInImage.h"

#include <algorithm> 
#include <numeric>

#include <image/image.h>
#include "math/2D/LineEquation.h"
#include "filter/algoImage.h"
#include "module/moduleLogger.h"

//helper functions
namespace 
{
using namespace precitec::filter::start_end_detection;

bool isEdgeVisible(Appearance value)
{
    return value == Appearance::BackgroundOnTop || value == Appearance::BackgroundOnBottom;
}


class StartEndDetectionHelper
{
    friend class precitec::filter::start_end_detection::StartEndDetectionInImage;

    enum findDirection {
        fromTop, fromBottom
    };

    template <findDirection t_direction, ImageStripeEvaluation t_value>
    static StripePositioning::Index findStripeIndexInROI(const StripesResult & rTube, unsigned int minNumberStripes);
    
    static ROIValidRange updateValidRange(const StripesResult & rTube, const StripePositioning & rStripePositioning, int minStripesMaterial);

};

} // end anonymous namespace

namespace precitec 
{
namespace filter 
{
namespace start_end_detection
{
    
void ImageStripeCalculator::examineStripes(StripesResult & rStripeResult, const precitec::image::BImage & rImage, 
    const StripePositioning & rStripePositioning) const
{    
    rStripeResult.clear();
    int n = rStripePositioning.getNumberOfStripes();
    if (n==0)
    {
        return;
    }
    
    int SchwelleRohrBG = getThreshold();
    
    for ( StripePositioning::Index stripeIndex = 0; stripeIndex <n; stripeIndex++ )
    {
        auto oStripeValues = computeSingleStripe(rImage, stripeIndex, rStripePositioning);
        //examine stripe
        bool isTubeStripe = false;
        if ( oStripeValues.valid )
        {
            // 3 Indizien: Max-Min gross, Mean gross, viele Pixel zu Rohr oder BG
            int counter = 
                int(oStripeValues.max - oStripeValues.min > ThreshMaxMinDiff) //MMgross
                + int(oStripeValues.mean > SchwelleRohrBG) //Mgross
                + int(oStripeValues.m_AnzRohr > 0.2* (oStripeValues.m_AnzRohr + oStripeValues.m_AnzBG)  // AnzRohr > 20%
                );
            if ( counter > 1 ) // at least 2 clues
            {
                isTubeStripe = true;
            }
        }
        rStripeResult[stripeIndex] = isTubeStripe ? ImageStripeEvaluation::Tube : ImageStripeEvaluation::NotTube;
    }
}


}}} //end namespace



//helper functions
namespace 
{
using namespace precitec::filter::start_end_detection;

template <StartEndDetectionHelper::findDirection t_direction, ImageStripeEvaluation t_value>
StripePositioning::Index StartEndDetectionHelper::findStripeIndexInROI(const StripesResult & rTube, unsigned int minNumberStripes)
{
    if (minNumberStripes == 0)
    {
        minNumberStripes = 1;
    }
    if ( rTube.size() == 0 )
    {
        return -1;
    }
    

    auto fEqual = [] (StripesResult::const_reference & element)
    {
        return element.second == t_value;
    };

    StripePositioning::Index index = -1;
    bool found = false;
    switch(t_direction)
    {
        case(findDirection::fromTop):
        {
            auto it = rTube.begin();
            while (!found)
            {
                it = std::find_if(it, rTube.end(), fEqual);
                if (std::distance(it, rTube.end())  < minNumberStripes)
                {
                    index = -1;
                    break;
                }
                found = std::all_of(it, next(it, minNumberStripes), fEqual);
                if (found)
                {
                    index = it->first;
                    break;
                }
                else
                {
                    it++;
                }
            }
        }
        break;
        case(findDirection::fromBottom):
        {            
            auto it = rTube.rbegin();
            while (!found)
            {
                it = std::find_if(it, rTube.rend(), fEqual);
                if (std::distance(it, rTube.rend())  < minNumberStripes)
                {
                    index = -1;
                    break;
                }
                found = std::all_of(it, next(it, minNumberStripes), fEqual);
                if (found)
                {
                    index = it->first;
                    break;
                }
                else
                {
                    it++;
                }
            }

        }
        break;
    }
    assert(found || index == -1);
    return index;
}


ROIValidRange StartEndDetectionHelper::updateValidRange(const StripesResult& rTube, const StripePositioning& rStripePositioning, int minStripesMaterial)
{
    typedef StripePositioning::StripeClip StripeClip;
    
    ROIValidRange oValidRange;
    oValidRange.mStripePositioning = rStripePositioning;
    StripePositioning::Index indexFirstMaterialFromTop = findStripeIndexInROI<fromTop, ImageStripeEvaluation::Tube>(rTube, minStripesMaterial);
    oValidRange.m_oStartValidRangeY = indexFirstMaterialFromTop == -1 ? -1 : (int) rStripePositioning.computeStripeCoordinateInImage(indexFirstMaterialFromTop, StripeClip::clipFirstIndex).y;

    StripePositioning::Index indexFirstMaterialFromBottom = findStripeIndexInROI<fromBottom, ImageStripeEvaluation::Tube>(rTube, minStripesMaterial);
    oValidRange.m_oEndValidRangeY = indexFirstMaterialFromBottom == -1 ? -1 : (int) rStripePositioning.computeStripeCoordinateInImage(indexFirstMaterialFromBottom, StripeClip::clipLastIndex).y;

    
    int numberOfStripes = rStripePositioning.getNumberOfStripes();

    if (numberOfStripes == 0)
    {
        oValidRange.mAppearance = Appearance::NotAvailable;
        return oValidRange;
    }

    //TODO: check special cases
    bool hasMaterialOnTop = indexFirstMaterialFromTop == 0;
    bool hasMaterialOnBottom = indexFirstMaterialFromBottom == numberOfStripes - 1;

    if ( hasMaterialOnTop && hasMaterialOnBottom )
    {
        //TODO: here could fall also the case of a black stripe in the middle
        oValidRange.mAppearance = Appearance::AllMaterial;
        assert (oValidRange.borderBackgroundStripe == -1);
        assert( oValidRange.m_oStartValidRangeY == 0);
        assert( oValidRange.m_oEndValidRangeY == rStripePositioning.computeStripeCoordinateInImage(numberOfStripes, StripeClip::clipBoth).y);  //clip to image height
        oValidRange.m_oEndValidRangeY -= 1;
        return oValidRange;
    }
    assert((indexFirstMaterialFromTop == -1) == (indexFirstMaterialFromBottom == -1));
    if (indexFirstMaterialFromTop == -1)
    {
        oValidRange.mAppearance = Appearance::AllBackground;
        assert (oValidRange.borderBackgroundStripe == -1);
        assert(oValidRange.m_oStartValidRangeY == -1);
        assert(oValidRange.m_oEndValidRangeY == -1);
        return oValidRange;
    }

    if (!hasMaterialOnTop)  //isTopDark
    {
        StripePositioning::Index indexBgStripeFromBottom = findStripeIndexInROI<fromBottom, ImageStripeEvaluation::NotTube>(rTube,0);
        if ( indexBgStripeFromBottom != indexFirstMaterialFromTop - 1 )
        {
            //black stripes on top and bottom
            oValidRange.mAppearance = Appearance::Unsupported;
            return oValidRange;
        }
        assert(indexFirstMaterialFromBottom == (numberOfStripes - 1));
        if (indexFirstMaterialFromBottom == (numberOfStripes -1))
        {
            oValidRange.mAppearance = Appearance::BackgroundOnTop;
            oValidRange.borderBackgroundStripe = indexBgStripeFromBottom;
            return oValidRange;
        }
        else
        {
            oValidRange.mAppearance = Appearance::Unsupported;
            return oValidRange;
        }
    }

    //isBottomDark
    assert(indexFirstMaterialFromTop == 0 && indexFirstMaterialFromBottom < (numberOfStripes -1));
    assert(hasMaterialOnTop && !hasMaterialOnBottom);
    
    StripePositioning::Index indexFirstBgStripeFromTop = findStripeIndexInROI<findDirection::fromTop, ImageStripeEvaluation::NotTube>(rTube, 0);
    if ( indexFirstBgStripeFromTop != indexFirstMaterialFromBottom + 1 )
    {
        //not contiguos material stripes
        oValidRange.mAppearance = Appearance::Unsupported;
        return oValidRange;

    }
    oValidRange.mAppearance = Appearance::BackgroundOnBottom;
    oValidRange.borderBackgroundStripe = indexFirstBgStripeFromTop;
    assert(oValidRange.m_oStartValidRangeY != -1);
    assert(oValidRange.m_oEndValidRangeY != -1);
    return oValidRange;
}


std::string to_string(Appearance p_EdgeROIContent)
{
    switch ( p_EdgeROIContent)
        {
            case Appearance::Unknown:
                return "Unknown";
            case Appearance::AllBackground:
                return "AllBackground";
            case Appearance::BackgroundOnTop:
                return "BackgroundOnTop";
            case Appearance::BackgroundOnBottom:
                return "BackgroundOnBottom";
            case Appearance::AllMaterial:
                return "AllMaterial";
            case Appearance::Unsupported:
                return "Unsupported";
        case Appearance::NotAvailable:
            return "NotAvailable";
        }
        assert(false && "unreacheable");
        return "?";
}

} //end anonymous namespace

namespace precitec
{
namespace filter
{
    
using precitec::image::BImage;
namespace start_end_detection
{
    
    
    

ROIValidRange::operator std::string() const
{
    switch ( mAppearance )
    {
    case Appearance::BackgroundOnTop:
    case Appearance::BackgroundOnBottom:
        return to_string(mAppearance) + " stripe=" + std::to_string(borderBackgroundStripe);
    default:
        return to_string(mAppearance);
    }
}


StripePositioning::StripePositioning(const image::Size2d & p_rImageSize, const Parameters & p_ROI)
:
m_oImageSize(p_rImageSize),
    m_parameters(p_ROI)
{
    if ( p_rImageSize.area() == 0 || m_parameters.width == 0 )
    {
        m_numStripes = 0;
        return ;
    }
        
    if ( m_parameters.x >= p_rImageSize.width
        || p_rImageSize.width < m_parameters.x + m_parameters.width)
    {
        m_numStripes = 0;
        return ;
    }

    assert(m_parameters.m_StripePositionStrategy == StripePositioning::Parameters::StripePositionStrategy::AlignTopOnlyFullStripes);
    //exclude last y if stripe smaller than m_stripe_height
    m_numStripes = getStripeIndexFromROI(0, m_oImageSize.height - m_parameters.m_stripe_height / 2 - 1) + 1;

}


StripePositioning::StripePositioning()
    :StripePositioning(image::Size2d{0,0},{}) 
{
    assert(m_numStripes == 0);
}


int StripePositioning::getNumberOfStripes() const
{
    return m_numStripes;
}


const StripePositioning::Parameters& StripePositioning::viewParameters() const
{
    return m_parameters;
}


const image::Size2d& StripePositioning::viewImageSize() const
{
    return m_oImageSize;
}

/*static*/ geo2d::DPoint StripePositioning::computeStripeCenterOnROI(int index, const Parameters & rROI)
{
    assert(rROI.m_StripePositionStrategy == StripePositioning::Parameters::StripePositionStrategy::AlignTopOnlyFullStripes);
    double pointX = rROI.width / 2;
    double pointY = index*rROI.m_stripe_height + rROI.m_stripe_height / 2;
    return{pointX, pointY};
}


StripePositioning::Index StripePositioning::computeStripeIndexFromImage(double xImage, double yImage) const
{
    if (m_numStripes  == 0)
    {
        return -1;
    }
    assert(m_parameters.m_StripePositionStrategy == StripePositioning::Parameters::StripePositionStrategy::AlignTopOnlyFullStripes);
    
    double xROI = xImage - m_parameters.x;
    double yROI = yImage;
    if ( yROI < 0 || yROI >= m_oImageSize.height )
    {
        return -1;
    }    
    return getStripeIndexFromROI(xROI, yROI);
    
}

//assumes input already checked
StripePositioning::Index StripePositioning::getStripeIndexFromROI(double xROI, double yROI) const
{
    (void) (xROI);
    assert(yROI >= 0 && yROI < m_oImageSize.height);
    assert(m_parameters.m_StripePositionStrategy == StripePositioning::Parameters::StripePositionStrategy::AlignTopOnlyFullStripes);
    return static_cast<int>(std::floor(yROI / m_parameters.m_stripe_height));
}

geo2d::DPoint StripePositioning::computeStripeCenterOnROIFromIndex(StripePositioning::Index index) const
{
    if (index < 0 || index >= m_numStripes)
    {
        return {-1,-1};
    }
    return computeStripeCenterOnROI(index, m_parameters);
}

geo2d::DPoint StripePositioning::computeStripeCoordinateInImage(StripePositioning::Index index, StripeClip p_direction)const
{

    if (index < 0 
        || (index == 0 && (p_direction==clipBoth || p_direction == clipFirstIndex)))
    {
        return {m_parameters.x, 0.0};
    }
    if (index >= m_numStripes
        || (index == m_numStripes-1 && (p_direction==clipBoth || p_direction == clipLastIndex)))
    {
        return{m_parameters.x + m_parameters.width - 1, double(m_oImageSize.height - 1)};
    }
    geo2d::DPoint roiPosition = computeStripeCenterOnROIFromIndex(index);
    assert(roiPosition.x >= 0 && roiPosition.x < m_parameters.width && roiPosition.y >= 0 && roiPosition.y < m_oImageSize.height);
    return {m_parameters.x + roiPosition.x, roiPosition.y};
}


geo2d::StartEndInfo::FittedLine StripePositioning::computeStripeMidlineInImage(StripePositioning::Index index) const
{
    auto midPoint = computeStripeCoordinateInImage(index, StripeClip::noClip);
    double m = m_parameters.m_stripe_inclination;
    return { m , midPoint.y - m * midPoint.x};
    
}

ImageStripeCalculator::ImageStripe ImageStripeCalculator::computeSingleStripe(const image::BImage & p_rImage, StripePositioning::Index index, const StripePositioning & rStripePositioning) const
{

    const auto & rPar = rStripePositioning.viewParameters();
    const auto & rImageSize = rStripePositioning.viewImageSize();
    const auto & rROIWidth = rPar.width;

    ImageStripe ret;
    ret.valid = false;
    if ( p_rImage.size() != rImageSize )
    {
        assert(false && "only the image used to build the ImageStripeCalculator can be used");
        return ret;
    }
    if ( index < 0 || index >= rStripePositioning.getNumberOfStripes() )
    {
        assert(false && "input should have been checked by the caller");
        return ret;
    }
    auto oCenterPoint = rStripePositioning.computeStripeCenterOnROIFromIndex(index);
    //no need to use checkValid(), by construction at least a point (oCenterPoint) is valid
    
    double m = rPar.m_stripe_inclination;
    double q = oCenterPoint.y - m * (oCenterPoint.x + rPar.x); //  image coordinates
    
    assert(oCenterPoint.x >= 0 && oCenterPoint.x < rROIWidth && oCenterPoint.y >= 0 && oCenterPoint.y < rImageSize.height);

    int  counter = 0, sum = 0;
    ret.min = 300;
    ret.max = -1;

    ret.m_AnzRohr = 0;
    ret.m_AnzBG = 0;

    //possible optimizations: m =0 should be handled separately.  iterate first on j 
    
    //x,y : onImage
    const int offsetX = speederX / 2;
    const int offsetY = rPar.m_stripe_height % speederY;
    const int halfRangeY = (rPar.m_stripe_height - 2 * offsetY) / 2 - 1;

    for ( int x = (int) (rPar.x + offsetX), xMax = (int) (rPar.x + rROIWidth - speederX / 2);
         x < xMax; 
        x += speederX )
        {
            assert(x >= 0 && x <  p_rImage.width());

            const int y0 = static_cast<int>(std::round (m * x + q)); // on image
            
            for ( int y = std::max(y0 - halfRangeY, 0), yMax = std::min(y0 + halfRangeY, rImageSize.height);
                            y < yMax; y += speederY )
            {
                assert(y >= 0 && y < p_rImage.height()); 	//Pixel liegt im Bild
                
                int greyVal = static_cast<int>(p_rImage[y][x]);
                counter++;
                sum += greyVal;

                if ( greyVal >  ret.max )              ret.max = greyVal;
                if ( greyVal <  ret.min )              ret.min = greyVal;
                if ( greyVal >= thresholdMaterial )   ret.m_AnzRohr++;
                if ( greyVal <= thresholdBackground )  ret.m_AnzBG++;

            }
        }

        ret.mean = counter ? sum / counter : 1;
        ret.valid = true;
        return ret;
}

    


const EdgePositionInImage & EdgeCalculator::viewlastEdgePositionInImage() const
{
    return m_lastEdgePositionInImage;
}
const std::vector<geo2d::Point> & EdgeCalculator::viewlastEdgePoints() const
{
    return m_lastPoints;
}
    
template<BackgroundPosition tBackgroundPosition>
void EdgeCalculator::calcEdgePosition(const image::BImage & rImage, FittedLine midLine,
                                      int searchAbove, int searchBelow, int minX, int maxX)
{    
    //define a comparison function: in the algorithms we want to sort by growing y
    const auto fSmallerY = [] (const geo2d::Point & r1, const geo2d::Point & r2)
        { return r1.y < r2.y; };
    
    // reverse order
    const auto fBiggerY = [] (const geo2d::Point & r1, const geo2d::Point & r2)
        { return r1.y > r2.y; };
    
    const auto fSumY = [] (int sum, const geo2d::Point & p)
        { return sum += p.y; };
        
    m_lastEdgePositionInImage.mAppearance = Appearance::Unknown;
    m_lastPoints.clear();

    const int numPixels = m_windowWidth*m_windowHeight; 
    const int thresholdSum = m_threshMaterial * numPixels;
    assert(numPixels > 0);

    int midYLeft = midLine.getY(minX);
    int midYRight = midLine.getY(maxX);
    
    auto minYLeft =  std::max(midYLeft - searchAbove,0);
    auto maxYLeft =  std::min(midYLeft + searchBelow+1, rImage.height());
    auto minYRight = std::max(midYRight - searchAbove,0);
    auto maxYRight = std::min(midYRight + searchBelow+1, rImage.height());
    
    
    geo2d::Point oTableStart{minX, std::min(minYLeft, minYRight)}; 
    geo2d::Point oTableEnd{ maxX - 1, std::max(maxYLeft, maxYRight)-1}; 
    
    BImage oROI(rImage, geo2d::Rect{oTableStart, geo2d::Size {oTableEnd.x+1 -oTableStart.x,oTableEnd.y +1 - oTableStart.y}}, true);
    assert(oROI[0] + 0 == rImage[oTableStart.y] + oTableStart.x);
    assert(oROI[oROI.height() - 1] + oROI.width() - 1 == rImage[oTableEnd.y] + oTableEnd.x);
    
    filter::SummedAreaTable<byte, int> oSummedAreaTable(oROI,filter::SummedAreaTableOperation::sumValues);

    //decide iteration direction in y
#ifdef _WIN32
    const bool forward = tBackgroundPosition == BackgroundPosition::Top ? true: false;
#else
    constexpr const bool forward = tBackgroundPosition == BackgroundPosition::Top ? true: false;
#endif
    m_lastPoints.reserve((maxX-minX)/m_windowWidth);
    
    //speeder unused
    for (int x = minX; x <= maxX-m_windowWidth; x += m_windowWidth)
    {
        int xROI = x - oTableStart.x;
        int minY = std::max(std::min( int(midLine.getY(x) - searchAbove), rImage.height() - 1),0);
        int maxY = std::max(std::min( int(midLine.getY(x) + searchBelow), rImage.height() - 1),0);
        
        const int firstY = forward ? minY: maxY;
        const int lastValidY = forward ? maxY - m_windowHeight-1 :  minY + m_windowHeight+1;
    
        // approximation wiht a rectangular window
        for (int y = firstY; 
             forward? y <= lastValidY : y >= lastValidY; 
             forward ? y++: y--)
        {
            int yROI = y - oTableStart.y;
            int sum = oSummedAreaTable.calcSum(xROI, 
                                                xROI+m_windowWidth,
                                                forward ? yROI : yROI - m_windowHeight,
                                                forward ? yROI + m_windowHeight : yROI);
            
            if ( sum > thresholdSum )
            {  
                m_lastPoints.push_back({x,y}); 
                break;
            }

            if (y == lastValidY)
            {   
                m_lastPoints.push_back({x,y}); 
                break;
            }
            
        }
    }
    
    unsigned int perCentKill = 15;
    unsigned int perCentKillNumber = (int) (0.5 + (perCentKill / 100.0) * m_lastPoints.size());

    if ( perCentKillNumber > 0 )
    {
        //reorder edgePoints so that the elements [0:perCentKillNumber] are the smallest,
        // the elements [perCentKillNumber: 2*perCentKillNumber] are the biggest
    
        std::nth_element(m_lastPoints.begin(),
                         m_lastPoints.begin() + perCentKillNumber,
                         m_lastPoints.end(),
                        fSmallerY);

        std::nth_element(m_lastPoints.begin() + perCentKillNumber + 1,
                         m_lastPoints.begin() + 2 * perCentKillNumber,
                         m_lastPoints.end(),
                        fBiggerY);
#ifndef NDEBUG        
        int valueMin = m_lastPoints.begin()->y;
        int value15Perc = (m_lastPoints.begin() + perCentKillNumber)->y;
        int valueMax = (m_lastPoints.begin() + perCentKillNumber+1)->y;
        int value85Perc = (m_lastPoints.begin() + 2 * perCentKillNumber)->y;
        std::cout <<"calcEdgePosition point distribution "
        << " input " << minX << " " << maxX << " " << midLine.getY((minX + maxX)/2) << " edgePoints " << valueMin << " " << value15Perc << " " << value85Perc << " " << valueMax 
        << " num points " << m_lastPoints.size() << std::endl;
#endif
    }

    auto itStart = m_lastPoints.cbegin() + 2 * perCentKillNumber + 1;
    auto itEnd = m_lastPoints.cend();
    int numPoints = itEnd - itStart;
    int sumY = std::accumulate(itStart, itEnd, 0, fSumY);
    int sumX = std::accumulate(itStart, itEnd, 0, [] (int sum, const geo2d::Point & p) { return sum += p.x; });
    
    //TODO  clean the points
    //m_lastEdgePositionInImage.xFittedLineCenter = (minX + maxX)/2;
    m_lastEdgePositionInImage.line.m = midLine.m; 
    m_lastEdgePositionInImage.line.q = (sumY - midLine.m * sumX)/ double(numPoints);
    m_lastEdgePositionInImage.mAppearance = tBackgroundPosition == BackgroundPosition::Top ? Appearance::BackgroundOnTop : Appearance::BackgroundOnBottom;

}

void EdgeCalculator::calcEdgePositionFromThreshold(const image::BImage & rImage, int minY, int maxY, int minX, int maxX, BackgroundPosition p_BackgroundPosition)
{
    //define a comparison function: in the algorithms we want to sort by growing y
    const auto fSmallerY = [] (const geo2d::Point & r1, const geo2d::Point & r2)
        { return r1.y < r2.y; };
    
    // reverse order
    const auto fBiggerY = [] (const geo2d::Point & r1, const geo2d::Point & r2)
        { return r1.y > r2.y; };
        
    m_lastEdgePositionInImage.mAppearance = Appearance::Unknown;
    m_lastPoints.clear();    
    

    BImage oROI(rImage, geo2d::Rect(minX, minY, maxX - minX, maxY - minY), true);
    assert(oROI[0] + 0 == rImage[minY] + minX);
    assert(oROI[oROI.height() - 1] + oROI.width() - 1 == rImage[maxY - 1] + maxX - 1);

    filter::SummedAreaTable<byte, int> oSummedAreaTable(oROI, filter::SummedAreaTableOperation::sumValues);
    
    EdgePositionInImage m_lastEdgePositionInImage;
    
    double numPixels = m_windowWidth*m_windowHeight;
    assert(numPixels > 0);

    for ( int yROI = 0, lastyROI = maxY - m_windowHeight - minY; yROI < lastyROI; yROI += m_windowHeight )
    {
        for ( int xROI = 0, lastxROI = maxX - m_windowWidth - minX; xROI < lastxROI; xROI += m_windowWidth )
        {
            double sum = oSummedAreaTable.calcSum(xROI, xROI + m_windowWidth, yROI, yROI + m_windowHeight);
            auto mean = std::round(sum / (double)(numPixels));
            if ( mean > m_threshMaterial )
            {
                //image coordinates 
                int x = xROI + minX;
                int y = yROI + minY;
                m_lastPoints.push_back(geo2d::Point{x + m_windowWidth/2 , y + m_windowHeight/2});
            }
        }
    }
    
    if ( m_lastPoints.size() == 0 )
    {
        assert(!m_lastEdgePositionInImage.valid());
        return;
    }
    
    //m_lastEdgePositionInImage.xFittedLineCenter = (minX + maxX)/2;
    switch(p_BackgroundPosition)
    {
        case( BackgroundPosition::Top):
        {
            int perCentKillNumber = static_cast<int>(0.01 * m_lastPoints.size());
            //reorder, so that the first perCentKillNumber are the smallest
            std::nth_element(m_lastPoints.begin(),  m_lastPoints.begin() + perCentKillNumber, m_lastPoints.end(),
                            fSmallerY);
            m_lastEdgePositionInImage.line = {0.0, double(std::min_element(m_lastPoints.cbegin() + perCentKillNumber + 1, m_lastPoints.cend(), fSmallerY)->y)};
            m_lastEdgePositionInImage.mAppearance = Appearance::BackgroundOnTop;
            break;
        }
        case(BackgroundPosition::Bottom):
        {
            int perCentKillNumber = static_cast<int>(0.01 * m_lastPoints.size());
            //reorder, so that the first perCentKillNumber are the biggest
            std::nth_element(m_lastPoints.begin(),  m_lastPoints.begin() + perCentKillNumber, m_lastPoints.end(), fBiggerY);
            m_lastEdgePositionInImage.line = {0.0, double(std::max_element(m_lastPoints.cbegin() + perCentKillNumber + 1, m_lastPoints.cend(), fSmallerY)->y)};
            m_lastEdgePositionInImage.mAppearance = Appearance::BackgroundOnBottom;
            break;
        }
    }
}

void EdgeCalculator::calcEdgePosition(const image::BImage & rImage, const ROIValidRange & rValidRange,
    int x, int width)
{
    m_lastEdgePositionInImage.mAppearance = Appearance::Unknown;
    m_lastPoints.clear();
    assert(!m_lastEdgePositionInImage.valid());
    
    if (!rImage.isValid())
    {
        assert(false);
        return;
    }
    if (rValidRange.mAppearance != Appearance::BackgroundOnBottom && rValidRange.mAppearance != Appearance::BackgroundOnTop)
    {
        assert(false);
        return;
    }
    int minX = x;
    int maxX = x + width;
    
    assert( minX >= 0 && minX < rImage.size().width && maxX >= 0 && maxX <= rImage.size().width);
    
    assert(rValidRange.borderBackgroundStripe != -1);
    
    auto oStripeMidline = rValidRange.mStripePositioning.computeStripeMidlineInImage(rValidRange.borderBackgroundStripe);
    
    if (rValidRange.mAppearance == Appearance::BackgroundOnTop)
    {        
        int searchBelow = rImage.height()  - std::min(oStripeMidline.getY(minX), oStripeMidline.getY(maxX));        
        calcEdgePosition<BackgroundPosition::Top>(rImage, oStripeMidline, m_STRIPE_OFFSET, searchBelow  , minX, maxX);
    }
    else
    {
        assert(rValidRange.mAppearance == Appearance::BackgroundOnBottom);
        
        int searchAbove = std::max(oStripeMidline.getY(minX), oStripeMidline.getY(maxX));
        calcEdgePosition<BackgroundPosition::Bottom>(rImage, oStripeMidline, searchAbove, m_STRIPE_OFFSET, minX, maxX);
    }
}


void StartEndDetectionInImage::resetState()
{
    for (auto && result: m_EdgeSearchResults)
    {
        result.points.clear();
    }
    m_lastResult.reset();
}


void StartEndDetectionInImage::initializeSearchAreaStripes(const precitec::image::BImage & rImage)
{ 
    const auto & rPar = m_parameters;
    assert(rImage.isValid());
    
    auto oImageSize = rImage.size();
    
    m_StripesSearchResult.clear();
    if ( !rPar.m_searchWholeImageForBackground)
    {
        int oSearchWidthLeft = std::min(m_parameters.m_searchWidthLeft, static_cast<int>(rImage.width() *0.5));
        int oSearchWidthRight = std::min(m_parameters.m_searchWidthRight, static_cast<int>(rImage.width() *0.5));
        StripePositioning::Parameters oROILeft{m_parameters.m_offsetLeftRight, oSearchWidthLeft, rPar.m};
        const double startRight = rImage.width() - oSearchWidthRight - m_parameters.m_offsetLeftRight;
        StripePositioning::Parameters oROIRight{startRight, oSearchWidthRight, rPar.m};
        m_StripesSearchResult.push_back({{oImageSize, oROILeft},{}});
        m_StripesSearchResult.push_back({{oImageSize, oROIRight},{}});
    }
    else
    {
    
        StripePositioning::Parameters oROIWholeImage{0.0, rImage.width(), rPar.m};
        m_StripesSearchResult.push_back({{oImageSize, oROIWholeImage},{}}); 
    }
}

void StartEndDetectionInImage::initializeSearchAreaEdge(const precitec::image::BImage & rImage)
{ 

    assert(rImage.isValid());
    const auto & rPar = m_parameters;

    auto oSearchWidthLeft = rPar.m_searchWidthLeft > rImage.width() ? rImage.width() : rPar.m_searchWidthLeft;
    auto oSearchWidthRight = rPar.m_searchWidthRight > rImage.width() ? rImage.width() : rPar.m_searchWidthRight;
    const auto oOffsetLeft = rPar.m_offsetLeftRight;
    const auto oOffsetRight = rPar.m_offsetLeftRight;
    
    auto & rEdgeSearchResultLeft = m_EdgeSearchResults[0];
    rEdgeSearchResultLeft.x = oOffsetLeft;
    rEdgeSearchResultLeft.width = oSearchWidthLeft;
    rEdgeSearchResultLeft.points.clear();
    
    auto & rEdgeSearchResultRight = m_EdgeSearchResults[1];
    rEdgeSearchResultRight.x = rImage.width() - oSearchWidthRight - oOffsetRight;
    rEdgeSearchResultRight.width = oSearchWidthRight;
    rEdgeSearchResultRight.points.clear();
    
}

StartEndDetectionInImage::Result::Result() 
: mImageValidRange(ROIValidRange{}, ROIValidRange{})
{
    reset();
}


void StartEndDetectionInImage::Result::reset()
{
    mImageValidRange.reset();
    mLeftEdgePosition.mAppearance = Appearance::Unknown;
    mRightEdgePosition.mAppearance = Appearance::Unknown;
}


StartEndDetectionInImage::Result::Result(ImageValidRange p_oImageValidRange, EdgePositionInImage p_oLeftEdgePosition, EdgePositionInImage p_oRightEdgePosition)
:  mImageValidRange(p_oImageValidRange), 
                mLeftEdgePosition(p_oLeftEdgePosition), 
                mRightEdgePosition(p_oRightEdgePosition)
{
}

bool StartEndDetectionInImage::hasEdge() const {
    return m_lastResult.mImageValidRange.isEdgeVisible();
}

void StartEndDetectionInImage::process(const image::BImage& image, EdgeSearch p_EdgeSearch, int minStripesMaterial)
{
    resetState();

    initializeSearchAreaStripes(image);
    detectValidRange(image, p_EdgeSearch, minStripesMaterial);

    assert(m_lastResult.mLeftEdgePosition.mAppearance == Appearance::Unknown);
    assert(m_lastResult.mRightEdgePosition.mAppearance == Appearance::Unknown);

    if ( m_lastResult.mImageValidRange.isEdgeVisible())
    {
        detectEdgePosition(image, p_EdgeSearch);
        //update the roi position given the actual edge (especially important when the edge is oblique)
        updateValidRangeFromEdges(image.size());
    }

    if (m_parameters.m_searchWidthLeft == 0)
    {
        m_lastResult.mLeftEdgePosition.mAppearance = Appearance::NotAvailable;
    }
    if (m_parameters.m_searchWidthRight == 0)
    {
        m_lastResult.mRightEdgePosition.mAppearance = Appearance::NotAvailable;
    }

    assert(m_lastResult.mImageValidRange.isEdgeVisible() ==
        (m_lastResult.mLeftEdgePosition.valid() || m_lastResult.mRightEdgePosition.valid()));
}

void StartEndDetectionInImage::processAsFullImage(image::Size2d imageSize)
{
    resetState();
    m_StripesSearchResult.clear();
    StripePositioning dummyStripes{imageSize, {0.0, 0, 0.0}};
    //dummy left and right stripes
    m_StripesSearchResult.push_back({dummyStripes,{}});
    m_StripesSearchResult.push_back({dummyStripes,{}});

    m_lastResult.reset();

    ROIValidRange fullValidRange;
    fullValidRange.mAppearance = Appearance::AllMaterial;
    fullValidRange.borderBackgroundStripe = -1;
    fullValidRange.mStripePositioning = dummyStripes;
    fullValidRange.m_oStartValidRangeY = 0;
    fullValidRange.m_oEndValidRangeY = imageSize.height - 2;

    m_lastResult.mImageValidRange = ImageValidRange{fullValidRange, fullValidRange};

}


void StartEndDetectionInImage::detectEdgePosition(const precitec::image::BImage & image, EdgeSearch p_EdgeSearch)
{    
    assert(m_lastResult.mLeftEdgePosition.mAppearance == Appearance::Unknown);
    assert(m_lastResult.mRightEdgePosition.mAppearance == Appearance::Unknown);
    
    const auto & rImageValidRange = m_lastResult.mImageValidRange;
    if (!rImageValidRange.isEdgeVisible())
    {
        return;
    }
    
    initializeSearchAreaEdge(image);
    
    EdgeCalculator edgeCalculator;
    const auto & rPar = m_parameters;
    edgeCalculator.m_threshMaterial = rPar.m_threshMaterialForEdgeRecognition;
    edgeCalculator.m_windowWidth = rPar.m_resolutionForEdgeRecognition;
    edgeCalculator.m_windowHeight = rPar.m_resolutionForEdgeRecognition;
    
    int updatedEdges = 0;
    for (bool useLeft : {true, false})
    { 
        auto & rPartialSearchResult = useLeft ? m_EdgeSearchResults[0]: m_EdgeSearchResults[1];        
        auto & rEdgeDescription = rImageValidRange.viewSubRange(useLeft);
        
        auto & rEdgePosition =  useLeft ? m_lastResult.mLeftEdgePosition : m_lastResult.mRightEdgePosition;
        auto & rResultPoints = rPartialSearchResult.points;

        rResultPoints.clear();

        //compute edge if the image is not completely background or completely material
        if (!isEdgeVisible(rEdgeDescription.mAppearance))
        {
            rEdgePosition.mAppearance = rEdgeDescription.mAppearance;
            //rEdgePosition.xFittedLineCenter = -1;
            rEdgePosition.line = {0.0, -1.0};
            continue;
        }
        
        switch (p_EdgeSearch)
        {
            case EdgeSearch::BothDirections: 
                break;
            case EdgeSearch::OnlyBackgroundOnTop: 
                assert(rEdgeDescription.mAppearance == Appearance::BackgroundOnTop); 
                break;
            case EdgeSearch::OnlyBackgroundOnBottom: 
                assert(rEdgeDescription.mAppearance == Appearance::BackgroundOnBottom);
                break;
        }
        
        edgeCalculator.calcEdgePosition(image, rEdgeDescription, rPartialSearchResult.x, rPartialSearchResult.width);
        rEdgePosition = edgeCalculator.viewlastEdgePositionInImage();
        rResultPoints = edgeCalculator.viewlastEdgePoints();
        updatedEdges++;
    }
    assert(updatedEdges == int(m_lastResult.mLeftEdgePosition.valid()) + int(m_lastResult.mRightEdgePosition.valid()));
    assert(updatedEdges > 0 && "input was not checked");

}

void StartEndDetectionInImage::detectValidRange(const precitec::image::BImage & image, EdgeSearch p_EdgeSearch, int minStripesMaterial) 
{
    auto & rSearchParameters = m_parameters.m_stripeParameters;
    
    assert(image.isValid());

    bool searchWholeImageForBackground = (m_StripesSearchResult.size() == 1);  //set by initializeSearchArea...
    
    assert((searchWholeImageForBackground && m_StripesSearchResult.size()==1)
        || (!searchWholeImageForBackground && m_StripesSearchResult.size()==2));

    ROIValidRange leftValidRange;
    ROIValidRange rightValidRange;

    for ( unsigned int i = 0; i < m_StripesSearchResult.size(); i++ )
    {
        assert(i < 2);
        const auto & rStripePositioning = m_StripesSearchResult[i].first;
        auto & rStripes = m_StripesSearchResult[i].second;
        rStripes.clear();
        auto & rValidRange = (i == 0) ? leftValidRange : rightValidRange;
        
        
        rSearchParameters.examineStripes(rStripes, image, rStripePositioning);
        rValidRange = StartEndDetectionHelper::updateValidRange(rStripes, rStripePositioning, minStripesMaterial);
        switch ( p_EdgeSearch )
        {
            case EdgeSearch::BothDirections:
                break;
            case EdgeSearch::OnlyBackgroundOnBottom:
                if ( rValidRange.mAppearance == Appearance::BackgroundOnTop )
                {
                    rValidRange.mAppearance = Appearance::Unsupported;
                }
                break;
            case EdgeSearch::OnlyBackgroundOnTop:
                if ( rValidRange.mAppearance == Appearance::BackgroundOnBottom )
                {
                    rValidRange.mAppearance = Appearance::Unsupported;
                }
                break;
        }
    }

    
    if ( searchWholeImageForBackground )
    {
        assert(m_StripesSearchResult.size() == 1);
        rightValidRange = leftValidRange;
    }
    else
    {
        //by construction, could change if width is different and line is not horizontal
        assert(m_StripesSearchResult.size() == 2 && (m_StripesSearchResult[0].second.size() == m_StripesSearchResult[1].second.size() || m_StripesSearchResult[0].second.size() == 0 || m_StripesSearchResult[1].second.size() == 0));
    }
    m_lastResult.mImageValidRange = ImageValidRange{leftValidRange, rightValidRange};

}


bool ImageValidRange::isEdgeVisible() const
{
    typedef geo2d::StartEndInfo::ImageState ImageState;
    return mImageState == ImageState::OnlyRightEdgeVisible
        || mImageState == ImageState::OnlyLeftEdgeVisible
        || mImageState == ImageState::FullEdgeVisible;
}



void StartEndDetectionInImage::updateValidRangeFromEdges(const precitec::image::Size2d & imageSize)
{
    double midX = imageSize.width / 2.0;

    for (bool useLeft : {true, false})
    {
        auto newEdgeDescription = m_lastResult.mImageValidRange.viewSubRange(useLeft);
        if (!isEdgeVisible(newEdgeDescription.mAppearance))
        {
            continue;
        }
        auto oStripeMidline = newEdgeDescription.mStripePositioning.computeStripeMidlineInImage(newEdgeDescription.borderBackgroundStripe);
        auto newRoiY = std::min(std::max<int>(std::round(oStripeMidline.getY(midX)),0), imageSize.height);
        if (newEdgeDescription.mAppearance == Appearance::BackgroundOnTop)
        {
            newEdgeDescription.m_oStartValidRangeY = newRoiY;
        }
        else
        {
            assert(newEdgeDescription.mAppearance == Appearance::BackgroundOnBottom && "edge not visible");
            newEdgeDescription.m_oEndValidRangeY = newRoiY;
        }

        if (useLeft)
        {
            m_lastResult.mImageValidRange.setLeftValidRange(newEdgeDescription);
        }
        else
        {
            m_lastResult.mImageValidRange.setRightValidRange(newEdgeDescription);
        }
    }
}


bool ImageValidRange::hasBackgroundOnTop() const
{
    if ( !isEdgeVisible() )
    {
        return false;
    }
    return mLeftValidRange.mAppearance == Appearance::BackgroundOnTop || mRightValidRange.mAppearance == Appearance::BackgroundOnTop;
}

bool ImageValidRange::hasBackgroundOnBottom() const
{
    if ( !isEdgeVisible() )
    {
        return false;
    }
    return mLeftValidRange.mAppearance == Appearance::BackgroundOnBottom || mRightValidRange.mAppearance == Appearance::BackgroundOnBottom;
}

void ImageValidRange::reset()
{
    mImageState = geo2d::StartEndInfo::ImageState::Unknown;
    mLeftValidRange.mAppearance = Appearance::Unknown;
    mRightValidRange.mAppearance = Appearance::Unknown;
}


ImageValidRange::ImageValidRange(ROIValidRange p_LeftValidRange, ROIValidRange p_RightValidRange)
    : mLeftValidRange(std::move(p_LeftValidRange)), mRightValidRange(std::move(p_RightValidRange))
{
    mImageState = computeImageState();
}


const ROIValidRange& ImageValidRange::viewSubRange(bool useLeft) const
{
    return useLeft ? mLeftValidRange : mRightValidRange;
}


void ImageValidRange::setLeftValidRange(ROIValidRange range)
{
    mLeftValidRange = range;
}


void ImageValidRange::setRightValidRange(ROIValidRange range)
{
    mRightValidRange = range;
}



const geo2d::StartEndInfo::ImageState& ImageValidRange::viewImageState() const
{
    return mImageState;
}


geo2d::StartEndInfo::ImageState ImageValidRange::computeImageState() const
{
    typedef geo2d::StartEndInfo::ImageState ImageState;

    const auto & rLeftContent = mLeftValidRange.mAppearance;
    const auto & rRightContent = mRightValidRange.mAppearance;

    // make sure that both contens are valid
    std::array<Appearance,2> rContents = {rLeftContent, rRightContent};

    if ( std::find(rContents.begin(), rContents.end(), Appearance::Unsupported) != rContents.end() )
    {
        return ImageState::Invalid;
    }
    if ( std::find(rContents.begin(), rContents.end(), Appearance::Unknown) != rContents.end())
    {
        return ImageState::Unknown;
    }


    if ( rLeftContent == rRightContent )
    {
        switch ( rLeftContent )
        {
            case Appearance::AllBackground:
                return ImageState::OnlyBackground;

            case Appearance::AllMaterial:
                return ImageState::OnlyMaterial;

            case Appearance::BackgroundOnTop:
            case Appearance::BackgroundOnBottom:
                return ImageState::FullEdgeVisible;

            case Appearance::Unknown:
            case Appearance::Unsupported:
            case Appearance::NotAvailable:
                assert(false && "case already handled");
                return ImageState::Invalid;
        }
        assert(false && "not reachable if all cases handled");
        return ImageState::Invalid;
    }
    else
    {
        //mixed cases: e.g edge+ background, material + edge, material + background ...
        bool leftEdgeVisible = rLeftContent == Appearance::BackgroundOnTop || rLeftContent == Appearance::BackgroundOnBottom;
        bool rightEdgeVisible = rRightContent == Appearance::BackgroundOnTop || rRightContent == Appearance::BackgroundOnBottom;

        bool leftROIUniform = rLeftContent == Appearance::AllBackground || rLeftContent == Appearance::AllMaterial || rLeftContent == Appearance::NotAvailable;
        bool rightROIUniform = rRightContent == Appearance::AllBackground || rRightContent == Appearance::AllMaterial || rRightContent == Appearance::NotAvailable;
        
        if ( leftEdgeVisible && rightROIUniform )
        {
            return ImageState::OnlyLeftEdgeVisible;
        }
        if ( rightEdgeVisible && leftROIUniform )
        {
            return ImageState::OnlyRightEdgeVisible;
        }

        if (rLeftContent == Appearance::NotAvailable || rRightContent == Appearance::NotAvailable)
        {
            auto& content = rRightContent == Appearance::NotAvailable ? rLeftContent : rRightContent;
            if (content == Appearance::AllBackground)
            {
                return ImageState::OnlyBackground;
            }
            if (content == Appearance::AllMaterial)
            {
                return ImageState::OnlyMaterial;
            }
        }

        //all the other cases (background top + background bottom, edge + invalid, all abckground + all material etc) are not valid
        return ImageState::Invalid;
    }
   
}
geo2d::StartEndInfo StartEndDetectionInImage::getLastImageStartEndInfo(int offset) const
{
    typedef geo2d::StartEndInfo::ImageState ImageState;
    
    
    auto fGetBorderBgStripeY = [] (const ROIValidRange & rValidRange) 
    {
        auto & rIndex = rValidRange.borderBackgroundStripe;
        auto oStripeCenter = rValidRange.mStripePositioning.computeStripeCoordinateInImage(rIndex, StripePositioning::noClip);
        return int(oStripeCenter.y);
    };
    
    //initialize startendinfo as background
    geo2d::StartEndInfo info;
    info.m_oImageState = m_lastResult.mImageValidRange.viewImageState();
    info.m_oImageStateEvaluation = geo2d::StartEndInfo::ImageStateEvaluation::Unknown; //to be filled later when seam is evaluated
    info.threshBackground = m_parameters.m_stripeParameters.thresholdBackground;
    info.threshMaterial = m_parameters.m_threshMaterialForEdgeRecognition; //m_parameters.m_stripeParameters.threshBeginMaterial;
    info.m_oStartValidRangeY = -1;
    info.m_oEndValidRangeY = -1;
    info.isTopDark = true;
    info.isBottomDark = true;
    info.isTopMaterial = false;
    info.isBottomMaterial = false;
    info.isCropped = true;
    info.borderBgStripeY = -1;

    auto fApplyOffset = [&info, &offset] ()
    {
        assert(info.isCropped);
        if ( offset != 0 )
        {
            if ( info.isTopDark )
            {
                info.m_oStartValidRangeY = std::max(0, info.m_oStartValidRangeY + offset);
            }
            if ( info.isBottomDark )
            {
                info.m_oEndValidRangeY = std::max(0, info.m_oEndValidRangeY - offset);
            }
        }
    };

    auto fUpdateValidRangeWithPartialEdge = [&info, &fGetBorderBgStripeY] (const ROIValidRange & rValidRange)
    {
        info.isCropped = true;
        info.isBottomDark = rValidRange.mAppearance == Appearance::BackgroundOnBottom;
        info.isTopDark = rValidRange.mAppearance == Appearance::BackgroundOnTop;
        info.isBottomMaterial = !info.isBottomDark;
        info.isTopMaterial = !info.isTopDark;

        info.borderBgStripeY = fGetBorderBgStripeY(rValidRange);
        info.m_oStartValidRangeY = rValidRange.m_oStartValidRangeY;
        info.m_oEndValidRangeY = rValidRange.m_oEndValidRangeY;  
    };

    const auto & rLeftEdgeDescription = m_lastResult.mImageValidRange.viewSubRange(true);
    const auto & rRightEdgeDescription = m_lastResult.mImageValidRange.viewSubRange(false);
    const auto & rContentTypeLeft = rLeftEdgeDescription.mAppearance;
    const auto & rContentTypeRight = rRightEdgeDescription.mAppearance;

    switch ( m_lastResult.mImageValidRange.viewImageState() )
    {
        case ImageState::Unknown:
        case ImageState::OnlyBackground:
        case ImageState::Invalid:
            //info is already correct
            break;
        case ImageState::OnlyLeftEdgeVisible:
            assert( isEdgeVisible(rContentTypeLeft) != isEdgeVisible(rContentTypeRight));
            fUpdateValidRangeWithPartialEdge(rLeftEdgeDescription);;
            fApplyOffset();
            info.leftEdge = m_lastResult.mLeftEdgePosition.line;
            info.isBottomDark = info.isBottomDark || rContentTypeRight == Appearance::AllBackground;
            info.isTopDark = info.isTopDark || rContentTypeRight == Appearance::AllBackground;
            info.isTopMaterial = info.isTopMaterial || rContentTypeRight == Appearance::AllMaterial;
            info.isBottomMaterial = info.isBottomMaterial || rContentTypeRight == Appearance::AllMaterial;
            break;
        case ImageState::OnlyRightEdgeVisible:
            assert( isEdgeVisible(rContentTypeLeft) != isEdgeVisible(rContentTypeRight));
            fUpdateValidRangeWithPartialEdge(rRightEdgeDescription);
            fApplyOffset();
            info.rightEdge = m_lastResult.mRightEdgePosition.line;
            info.isBottomDark = info.isBottomDark || rContentTypeLeft == Appearance::AllBackground;
            info.isTopDark = info.isTopDark || rContentTypeLeft == Appearance::AllBackground;
            info.isTopMaterial = info.isTopMaterial || rContentTypeLeft == Appearance::AllMaterial;
            info.isBottomMaterial = info.isBottomMaterial || rContentTypeLeft == Appearance::AllMaterial;
            break;
        case ImageState::FullEdgeVisible:
        {
            assert(isEdgeVisible(rContentTypeLeft) && isEdgeVisible(rContentTypeRight) );
            assert(rLeftEdgeDescription.m_oStartValidRangeY != -1 &&  rRightEdgeDescription.m_oStartValidRangeY != -1);
            assert(rContentTypeLeft == rContentTypeRight);
            info.m_oStartValidRangeY = std::max(rLeftEdgeDescription.m_oStartValidRangeY, rRightEdgeDescription.m_oStartValidRangeY);
            info.m_oEndValidRangeY = std::min(rLeftEdgeDescription.m_oEndValidRangeY, rRightEdgeDescription.m_oEndValidRangeY);
            info.isCropped = true;
            info.isTopDark = rContentTypeLeft == Appearance::BackgroundOnTop;
            info.isBottomDark = rContentTypeLeft == Appearance::BackgroundOnBottom;
            info.isTopMaterial = !info.isTopDark;
            info.isBottomMaterial = !info.isBottomDark;
            auto borderBgStripeYLeft = fGetBorderBgStripeY(rLeftEdgeDescription);
            auto borderBgStripeYRight = fGetBorderBgStripeY(rRightEdgeDescription);
            info.borderBgStripeY =  info.isTopDark ? std::min(borderBgStripeYLeft, borderBgStripeYRight) 
                                                        : std::max(borderBgStripeYLeft, borderBgStripeYRight);  
            fApplyOffset();
            info.leftEdge = m_lastResult.mLeftEdgePosition.line;
            info.rightEdge = m_lastResult.mRightEdgePosition.line;
        }
            break;
            
        case ImageState::OnlyMaterial:
            assert(rLeftEdgeDescription.m_oStartValidRangeY == rRightEdgeDescription.m_oStartValidRangeY || rLeftEdgeDescription.mAppearance == Appearance::NotAvailable || rRightEdgeDescription.mAppearance == Appearance::NotAvailable);
            assert(rLeftEdgeDescription.m_oEndValidRangeY == rRightEdgeDescription.m_oEndValidRangeY || rLeftEdgeDescription.mAppearance == Appearance::NotAvailable || rRightEdgeDescription.mAppearance == Appearance::NotAvailable);
            info.m_oStartValidRangeY = rLeftEdgeDescription.m_oStartValidRangeY == -1 ? rRightEdgeDescription.m_oStartValidRangeY : rLeftEdgeDescription.m_oStartValidRangeY;
            assert(info.m_oStartValidRangeY == 0);
            info.m_oEndValidRangeY = rLeftEdgeDescription.m_oEndValidRangeY == -1 ? rRightEdgeDescription.m_oEndValidRangeY : rLeftEdgeDescription.m_oEndValidRangeY;
            assert(info.m_oEndValidRangeY > 0 && "endValidRange must be imageHeight"); 
            info.isTopDark = false;
            info.isBottomDark = false;
            info.isTopMaterial = true;
            info.isBottomMaterial = true;
            info.isCropped = false;
            info.m_oEndValidRangeY += 1; //FIXME
            break;
        
    }
    return info;
    
};

const StripePositioning & StartEndDetectionInImage::viewStripePositionSearch(bool left) const
{
    int index = (left || m_StripesSearchResult.size() == 1) ? 0 : 1;
    return m_StripesSearchResult[index].first;
}

const StripesResult& StartEndDetectionInImage::viewStripes(bool left) const
{
    int index = (left || m_StripesSearchResult.size() == 1) ? 0 : 1;
    return m_StripesSearchResult[index].second;
}


const std::vector< precitec::geo2d::Point >& StartEndDetectionInImage::viewLastEdgePoints(bool left) const
{
    return m_EdgeSearchResults [left? 0 : 1].points;
}

} //end namespace
}
}


