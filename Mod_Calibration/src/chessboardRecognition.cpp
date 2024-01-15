#include "calibration/chessboardRecognition.h"
#include <cmath>

namespace precitec
{
namespace calibration_algorithm
{

ChessboardRecognitionAlgorithm::ChessboardRecognitionAlgorithm(const precitec::image::BImage & rImgSource, int providedThreshold,
                                   PreviewType previewType)
    :  m_oCornerGrid(rImgSource.size()), m_previewType(previewType)
{
    m_validGridMap = false;
    if (!rImgSource.isValid())
    {
        return;
    }

    auto preProcessedImage = preprocessImage(rImgSource);

    if (previewType == PreviewType::AfterSmoothing)
    {
        m_previewImage = preProcessedImage;
        return;
    }

    int border = FilteringAlgorithms::m_oDetectionMaskSize;
    bool useProvidedThreshold = (providedThreshold != -1);
    const int threshold = useProvidedThreshold ?
                                providedThreshold : guessThreshold(rImgSource,border);
    m_threshold = threshold;
    std::vector<int> m_thresholdCandidates(1, threshold);
    if (!useProvidedThreshold)
    {
        for (int i = 1; i < 3; i++)
        {
            m_thresholdCandidates.push_back(threshold + i);
            m_thresholdCandidates.push_back(threshold - i);
        }
    }

        auto oInnerSize = validImageSizeAfterFiltering(rImgSource.size(), border);

        //assuming that the chessboard should cover the full image
        double expectedIntensitySum =  oInnerSize.area()*255/2.0;
        double tolerance = 0.2;
        int minExpectedIntensitySum = (1 -tolerance) * expectedIntensitySum;
        int maxExpectedIntensitySum = (1 + tolerance) * expectedIntensitySum;


    for (auto & thresholdCandidate : m_thresholdCandidates)
    {
        wmLog(eInfo, "Using threshold %d \n", thresholdCandidate);
        auto oBinarizedImage = computeBinaryImage(preProcessedImage, thresholdCandidate, previewType, &m_previewImage);
        if (!useProvidedThreshold)
        {
            int pixelSum = 0;
            oBinarizedImage.for_each([&pixelSum](byte pixelValue) { pixelSum += pixelValue;}, border, border, oInnerSize.width, oInnerSize.height);
            if (pixelSum < minExpectedIntensitySum || pixelSum > maxExpectedIntensitySum)
            {
                wmLog(eDebug, "Binarized image has low contrast (%f), skip to next candidate  \n", double(pixelSum) /double(125.0*oInnerSize.area()));
                continue;
            }
        }

        auto oThresholdedPoints = thresholdCornerPoints(oBinarizedImage, 200);
        ClusterCenters oClusters(oThresholdedPoints);
        m_oRecognizedCorners = oClusters.getRawCorners();

        std::tie( m_validGridMap, m_oCornerGrid) = generateGridMap(m_oRecognizedCorners, oClusters.m_oMinSquareSize, rImgSource.size() );
        if (m_validGridMap)
        {
            m_threshold = thresholdCandidate;
            break;
        }
    }
}

precitec::image::Size2d ChessboardRecognitionAlgorithm::validImageSizeAfterFiltering(const precitec::image::Size2d & rImageSize, int border)
{
    auto oInnerSize = rImageSize;
    oInnerSize.width -= (2*border);
    oInnerSize.height -= (2*border);
    if (oInnerSize.width <= 0 || oInnerSize.height <= 0)
    {
        return {0,0};
    }
    return oInnerSize;
}

int ChessboardRecognitionAlgorithm::guessThreshold(const precitec::image::BImage & preProcessedImage, int border)
{
    if (!preProcessedImage.isValid())
    {
        return 125;
    }
    auto oInnerSize = validImageSizeAfterFiltering(preProcessedImage.size(), border);
    if (oInnerSize.area() == 0)
    {
        return 125;
    }

    int pixelSum = 0;
    preProcessedImage.for_each([&pixelSum](byte pixelValue) { pixelSum += pixelValue;}, border, border, oInnerSize.width, oInnerSize.height);
    return pixelSum / oInnerSize.area();
}

precitec::image::BImage ChessboardRecognitionAlgorithm::preprocessImage(const precitec::image::BImage & rImgSource)
{
    using precitec::image::BImage;
    BImage oTempImage( rImgSource.size());
    rImgSource.copyPixelsTo(oTempImage);
    FilteringAlgorithms oFiltering;
    oTempImage = FilteringAlgorithms::convolution(oTempImage, oFiltering.maskGaussian());

    return oTempImage;
}

precitec::image::BImage ChessboardRecognitionAlgorithm::computeBinaryImage(const precitec::image::BImage & rImgSource , int oThreshold,
                                PreviewType previewType, image::BImage * p_previewImage)
{
    FilteringAlgorithms oFiltering;
    using precitec::image::BImage;
    BImage oTempImage( rImgSource.size());


    oTempImage = oFiltering.binarize(rImgSource, oThreshold);
    if (previewType == PreviewType::AfterBinarization)
    {
        if (p_previewImage)
        {
            *p_previewImage = oTempImage;
        }
        return oTempImage;
    }

    oTempImage = oFiltering.morphBW(oTempImage, oFiltering.maskDilation(), [](int&){});
    if (previewType == PreviewType::AfterDilation)
    {
        if (p_previewImage)
        {
            *p_previewImage = oTempImage;
        }
        return oTempImage;
    }

    oTempImage = oFiltering.morphBW(oTempImage, oFiltering.maskErosion(), [](int&){});
    if (previewType == PreviewType::AfterErosion)
    {
        if (p_previewImage)
        {
            *p_previewImage = oTempImage;
        }
        return oTempImage;
    }

    return oTempImage;
}

std::vector<precitec::geo2d::DPoint> ChessboardRecognitionAlgorithm::thresholdCornerPoints(const precitec::image::BImage & rBinarizedImage, byte oThreshold,
                                PreviewType previewType, image::BImage * p_previewImage)
{
    using precitec::image::BImage;
    using precitec::geo2d::DPoint;
    using precitec::LogType;
    using precitec::calibration_algorithm::FilteringAlgorithms;

    std::vector<DPoint> oThresholdedPoints;

    FilteringAlgorithms oImageFilterImplementation;

    for (auto filterType : { FilteringAlgorithms::CornerDetectionType::eBlackToWhite ,
                            FilteringAlgorithms::CornerDetectionType::eWhiteToBlack  })
    {
        bool returnForPreview = false;

        switch (filterType)
        {
            case (FilteringAlgorithms::CornerDetectionType::eBlackToWhite):
                returnForPreview =  (previewType == PreviewType::AfterCornerDetectionB2W);
                break;
            case (FilteringAlgorithms::CornerDetectionType::eWhiteToBlack):
                returnForPreview =  (previewType == PreviewType::AfterCornerDetectionW2B);
                break;
        }

        BImage oFilteredImage = oImageFilterImplementation.convolution(rBinarizedImage, oImageFilterImplementation.detectionMask(filterType));
        if (!oFilteredImage.isValid())
        {
            return {};
        }

        if (returnForPreview)
        {
            if (p_previewImage)
            {
                *p_previewImage = oFilteredImage;
            }
            return {};
        }


        //threshold and extract  corner coordinates

        auto oValMax = 0;

        int border = oImageFilterImplementation.detectionMask(FilteringAlgorithms::eWhiteToBlack).size();
        assert(border == FilteringAlgorithms::m_oDetectionMaskSize);

        for (int y= border, yMax = oFilteredImage.height() - border ; y < yMax ; ++y)
        {
            int x0 = border;
            auto * pLine = oFilteredImage.rowBegin(y) + x0;

            for (int x=x0, xMax = oFilteredImage.width() - border; x < xMax; ++x, pLine++)
            {
                auto & rVal = *pLine;
                if ( rVal >= oThreshold  )
                {
                    oThresholdedPoints.push_back({double(x), double(y)});
                }
                if ( rVal > oValMax )
                {
                    oValMax = rVal;
                }

            }


        }

        if ( oThreshold > oValMax*0.9 )
        {
            //wmLog( LogType::eDebug, "Corner extraction: filtered image doesn't have enough contrast \n");
        }

    //std::cout << "After filtering " << curOperation << ": " << oThresholdedPoints.size() << " candidate points found" << std::endl;
    }//for B2W, W2B

    return oThresholdedPoints;
}

std::pair<bool, precitec::math::CalibrationCornerGrid> ChessboardRecognitionAlgorithm::generateGridMap(const point_list_t & rRawCorners, const int p_oMinSquareSize,  geo2d::Size p_oValidArea)
{
    using precitec::LogType;
    using precitec::wmLog;
    using precitec::math::CalibrationCornerGrid;
    using precitec::geo2d::coord2D;

    const int oMaxSquareWidth = 200;  //hardcoded, just for checking during generateGridMap

    precitec::math::CalibrationCornerGrid oCornerGrid2D3D(p_oValidArea);

    //m_CornerGrid2D3D.reset();

    if (rRawCorners.size() == 0)
    {
        return {false, oCornerGrid2D3D};
    }

    bool oValidCornerGrid=true;
    auto itRawCornersEnd = rRawCorners.cend();

    //First loop: recognize lines (with y tolerance)

    //the loop relies on the fact that m_oGridY are sorted first by y (with tolerance), then by x

    //unsigned int warningsBeforeCurrentLine = m_oGridMapWarnings.length();

    int linesWithWarning = 0;

    int warningsInCurrentLine = 0;
    int warningPointOnDeformedLine = 0;
    int warningMaxXDistance = 0;
    int warningOverlappingLines = 0;
    int errorSplitLine = 0;

    auto fCheckSquareDimension = [&p_oMinSquareSize, &warningsInCurrentLine, &warningPointOnDeformedLine, &warningMaxXDistance]
            (double x, double y, const system::tGridMapLine2D & rCurrentLine, int oYSum)
        {
        if ( rCurrentLine.size() == 0)
        {
            return;
        }
        for ( auto && storedCorner : rCurrentLine )
        {
            if ( (y - storedCorner.ScreenY) > p_oMinSquareSize )
            {
                //wmLog(LogType::eWarning, "Point [%i,%i]: y coordinate too distant from other points, deformed line yAvg=%f point [%i,%i] \n",
                //        rCurrentLine[0].ScreenX, rCurrentLine[0].ScreenY, double(oYSum / double( rCurrentLine.size())),
                //        storedCorner.ScreenX, storedCorner.ScreenY );
                warningsInCurrentLine ++;
                warningPointOnDeformedLine++;
            }
        }

        auto oLastX = rCurrentLine.back().ScreenX;

#ifndef NDEBUG
        auto oLastY = rCurrentLine.back().ScreenY;
        assert((y - oLastY) <= p_oMinSquareSize && "Vertical distance too big, should have been checked by generateGridMap ");
        assert(x >= oLastX && "not growing, error in isEndOfLine");
#endif
        auto oSquareWidth = x - oLastX;
        if ( oSquareWidth > oMaxSquareWidth || oSquareWidth == 0)
        {
            //wmLog(eWarning, "Unexpected square width in line %f between points [%i,%i] [%f,%f]\n",
            //    double(oYSum) / double( rCurrentLine.size()),
            //    oLastX, oLastX, x, y);
            warningsInCurrentLine ++;
            warningMaxXDistance++;
        }
    };


    auto fEndOfLine = [&itRawCornersEnd, &warningOverlappingLines, &p_oMinSquareSize](point_list_t::const_iterator index)
    {

        auto nextIt = next(index);
        if ( index == itRawCornersEnd || nextIt == itRawCornersEnd ) //|| next(nextIt) == itRawCornersEnd)
        {
            return true;
        }
        auto curX = index->x;
        auto curY = index->y;

        auto nextX = nextIt->x;
        auto nextY = nextIt->y;

        bool HorizontallyAligned = (nextY - curY ) <= p_oMinSquareSize;
        bool GrowingX = (nextX - curX) > 0;

        bool isEndOfLine = ! (HorizontallyAligned && GrowingX); //ensure both conditions!
        if ( HorizontallyAligned && !GrowingX )
        {
            //if ( warningOverlappingLines == 0 )
            {
                //wmLog(eWarning,"Possible overlapping line cur point %f %f next point %f %f \n",
                //    curX, curY, nextX, nextY
                //);
            }
            warningOverlappingLines++;
        }
        return isEndOfLine;
    };  //end fEndOfLine

    system::tGridMapLine2D  oCurrentLine;
    system::tGridMap unorderedGridMap;
    int oYSum = 0;

    for (auto itCorner = rRawCorners.begin(), itPrevCorner = itRawCornersEnd; itCorner != itRawCornersEnd; itCorner++)
    {
        if (itCorner != rRawCorners.begin())
        {
            itPrevCorner = (itPrevCorner == itRawCornersEnd) ? rRawCorners.begin() : next(itPrevCorner);
            assert(next(itPrevCorner) == itCorner);
        }

        auto x = itCorner->x;
        auto y = itCorner->y;



        if (itPrevCorner != itRawCornersEnd && oCurrentLine.size() > 0)
        {
            if (std::abs(y - itPrevCorner->y) > p_oMinSquareSize)
            {
                // assumptions not respected!
                auto prevCorner = *itPrevCorner;
                //wmLog(eWarning, "Unexpected y distance %f: coordinate [%f,%f] after [%f,%f], error in insetrtIntoGrid\n", y- prevCorner.y, x,y, prevCorner.x, prevCorner.y);
                warningsInCurrentLine ++;
                return {false, oCornerGrid2D3D};
            }
        }


        fCheckSquareDimension(x,y,oCurrentLine, oYSum);


        oCurrentLine.push_back(coord2D(x,y));
        oYSum += y;

        //store computed line

        if ( fEndOfLine(itCorner) )  // store the previous line and reset line counters
        {
            assert(oCurrentLine.size() > 0);
            int oYAvg = std::round(oYSum / double(oCurrentLine.size()));

            auto oInsertionResult = unorderedGridMap.insert(std::make_pair(oYAvg, oCurrentLine));
            if ( !oInsertionResult.second )
            {
                oValidCornerGrid = false;
                //wmLog(eWarning, "Could not insert line %d \n", oYAvg);
                warningsInCurrentLine ++;
                errorSplitLine++;
            };
            if (warningsInCurrentLine > 0)
            {
                ++linesWithWarning;
            }
            oCurrentLine.clear();
            oYSum = 0;
            warningsInCurrentLine = 0;
        }
    }

    //display warnings


    if ( !oValidCornerGrid )
    {
        return {false, oCornerGrid2D3D};
    }

    // second loop: compute cell connectivity
    //former setCorners


    bool cornerDataOk = CalibrationCornerGrid::computeCornerData(oCornerGrid2D3D, unorderedGridMap, p_oMinSquareSize, true); //TODO: choose when to linearize
    if (!cornerDataOk)
    {
        //wmLog(eWarning, "Invalid Corner Connectivity\n");
    }
    oValidCornerGrid &= cornerDataOk;

    return {oValidCornerGrid, oCornerGrid2D3D};

}

ClusterCenters::ClusterCenters(const std::vector<precitec::geo2d::DPoint> & rThresholdedPoints, int pClusterRadius)
{
    auto oClusters = estimateClusters(rThresholdedPoints, pClusterRadius);
    computeClusterCenters(oClusters);
}

std::map<ClusterCenters::cluster_key_t, point_list_t> ClusterCenters::estimateClusters (const std::vector<precitec::geo2d::DPoint> &  rThresholdedPoints, int pClusterRadius)
{
    using geo2d::DPoint;
    std::map<cluster_key_t, point_list_t > oClusterCenters;

    //look for closest cluster center

    //calibrater::createClusterCenters(oClusters, 15);
    for (unsigned int i=0; i < rThresholdedPoints.size(); i++)
    {
        DPoint oCurPoint(rThresholdedPoints[i]);

        bool oAddToExistingCluster = false;

        for (auto& clusterKeyValues: oClusterCenters)
        {
            cluster_key_t clusterCenter = clusterKeyValues.first;
            if (std::abs(oCurPoint.x - clusterCenter[0]) < pClusterRadius
                    && std::abs(oCurPoint.y - clusterCenter[1]) < pClusterRadius )
            {
                oAddToExistingCluster = true;
                clusterKeyValues.second.push_back(oCurPoint);
                break;
            }

        }
        if (!oAddToExistingCluster)
        {
            cluster_key_t newClusterKey = { int(oCurPoint.x), int(oCurPoint.y)};
            oClusterCenters[newClusterKey].push_back(oCurPoint);
        }
    }
    return oClusterCenters;


}
int ClusterCenters::insertIntoGrid(double x, double y)  //insert into m_RawCorners, sorting according to minsquaresize
{
    int p_oX(x);
    int p_oY(y);


    int oYTolerance =  m_oMinSquareSize / 2;  //when tolerance is small, there will be more incomplete lines (adjusted in generate grid)
    assert (oYTolerance <= m_oMinSquareSize);
    assert(INDEXMAX > m_RawCorners.size());


    auto itEnd = m_RawCorners.end();
    auto itCorners = std::find_if(m_RawCorners.begin(), itEnd, [&p_oY, &oYTolerance](geo2d::DPoint& p){
        return p.y >= (p_oY - oYTolerance);});


    int tmp_x = itCorners != itEnd ? itCorners->x : -1;
    int tmp_y = itCorners != itEnd ? itCorners->y : -1;


    itCorners = std::find_if(itCorners, itEnd,
        [&p_oY, &p_oX , &oYTolerance, &tmp_x, &tmp_y](geo2d::DPoint& p)
        {
            bool isOutsideOfRow = (
                std::abs(p.y - p_oY) > oYTolerance ||
                p.y > (tmp_y + oYTolerance) ||
                p.x < tmp_x ||
                p.x >= p_oX);
            tmp_x = p.x;
            return isOutsideOfRow;
        }
    );




    //insert coordinate at the found position
    //p_oY will be inserted at position oIndex, before what is already there
    auto next_y = itCorners != itEnd  ? itCorners->y: INDEXMAX;
    auto next_x = itCorners != itEnd  ? itCorners->x: INDEXMAX;
    auto prev_y = -1.0;
    auto prev_x = -1.0;
    if (itCorners != m_RawCorners.begin())
    {
        auto itPrev = std::prev(itCorners);
        prev_x = itPrev->x;
        prev_y = itPrev->y;

    }


    bool validIndex = true;
    validIndex &= (p_oY >= (prev_y - oYTolerance));

    validIndex &= (p_oY <= (next_y + 2 * oYTolerance));
    if (p_oY > (next_y +  oYTolerance))
    {
        //wmLog(eWarning, "Curved line according to tolerance %d \n", oYTolerance);
    }
    //checking x makes sense only on the same line
    bool overlappingX = false;
    if ( std::abs(p_oY - prev_y) <= oYTolerance )
    {
        validIndex &= (p_oX >= prev_x);
        if ( p_oX == prev_x ) overlappingX = true;
    }
    if ( std::abs(p_oY - next_y) <= oYTolerance )
    {
        validIndex &= (p_oX <= next_x);
        if ( p_oX == next_x ) overlappingX = true;
    }
    if ( overlappingX )
    {
        //wmLog(LogType::eDebug, "Warning, overlapping x (%f %f) %d %d (%f %f)\n", prev_x, prev_y, p_oX, p_oY, next_x, next_y);

    }

    int oIndex = std::distance(m_RawCorners.begin(), itCorners);
    if ( !validIndex )
    {
        //wmLog(LogType::eDebug, "Invalid position for point %d %d - between %f %f and %f %f [%d]\n", p_oX, p_oY, prev_x, prev_y, next_x, next_y, oIndex);
        return -1;
    }
    assert(validIndex);
    m_RawCorners.insert(itCorners,geo2d::DPoint(p_oX,p_oY) );

    return oIndex;
}

void ClusterCenters::computeClusterCenters(const std::map<cluster_key_t, point_list_t>& rClusterCenters )
{
    point_vector_t oInvalidCorners;
    point_list_t oRawCorners;
    for (auto & clusterKeyValues: rClusterCenters )
    {
        int sumX = 0;
        int sumY =0;

        auto oClusterElements = clusterKeyValues.second;
        double n = oClusterElements.size();


        for (auto & rCoord:oClusterElements)
        {
            sumX += rCoord.x;
            sumY += rCoord.y;
        }
        int index = insertIntoGrid( sumX/n, sumY/n);  //update m_RawCorners
        if ( index < 0 )
        {
            m_invalidCorners.push_back(geo2d::DPoint(sumX/n, sumY/n));
        }
    }

    if (m_invalidCorners.size() > 0 )
    {
        std::cout << " Found " << m_invalidCorners.size() << " invalid points" << std::endl;

    }
}

}
}
