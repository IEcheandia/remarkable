
#include <string>
#include <fstream>
#include <numeric>  //accumulate
#include "circleFitImpl.h"
#include "module/moduleLogger.h"
#include "image/image.h"
#include <common/bitmap.h>
#include <system/tools.h>
#include "filter/algoPoint.h"

#define DEBUG_CIRCLEHOUGH 0

static int export_counter = 0;
namespace
{
    int adjustRadiusStep(double radiusStart, double radiusEnd, double & radiusStep)
    {
        constexpr int MAX_CANDIDATES_RADII = 20;
        int numCandidatesRadii = ((radiusEnd-radiusStart)/radiusStep+1);
        if (numCandidatesRadii > MAX_CANDIDATES_RADII)
        {
            auto newRadiusStep =  (radiusEnd - radiusStart)/(MAX_CANDIDATES_RADII-1);
            wmLog(precitec::eDebug, "Radius step too low (%f), expected %d iterations from radius %f to %f, setting radius step to %f \n",
                radiusStep, numCandidatesRadii, radiusStart, radiusEnd, newRadiusStep );
            radiusStep = newRadiusStep;
        }
        return ((radiusEnd-radiusStart)/radiusStep+1);
    }

}

namespace precitec {
	namespace filter {

using namespace precitec::image;

////////////////// CircleFitImpl

CircleFitImpl::CircleFitImpl()
{
	m_resultX = m_resultY = m_resultR = 0;
}

CircleFitImpl::~CircleFitImpl()
{
}

Circle CircleFitImpl::GetResult()
{
    return _resultCircle;
}

void CircleFitImpl::DoCircleFit(std::vector<geo2d::DPoint> data,
								int partStart,
								int partEnd,
								int iMinRadius )
{
	// Anzahl x/y-Punkte im Array
	int totalSize = data.size();

	if (totalSize <=3)
	{
		m_resultX = 0;
		m_resultY = 0;
		m_resultR = 0;
		return;
	}

	bool adjust = false;

	// Schauen, ob die Raender des zu nehmenden Bereichs sinnvoll liegen: 0 <= partStart <= partEnd <= 100
	if (partStart<0) adjust = true;
	if (partEnd>100) adjust = true;
	if (partStart>partEnd) adjust = true;

	if (adjust)
	{
		partStart = 0;
		partEnd = 100;
	}

    double x_ = 0, y_ = 0; // Variablen zum Speichern der Mittelwerte in x und y
    int count = 0; // Zaehlt die Anzahl der in Frage kommenden Punkte

    // Ausdehnung in X bestimmen. Ist zum Rausrechnen der "Nase" sinnvoll.
    double ReX=-1000000;
    double LiX= 1000000;

	for(int i=0; i<totalSize; i++)
	{
		double x = data[i].x;
        if (x>ReX) ReX = x;
        if (x<LiX) LiX = x;
	}

    double startX = LiX + (ReX-LiX)*(partStart/100.0);
    double endeX = LiX + (ReX-LiX)*(partEnd/100.0);

	if ( (endeX - startX) < 3 )
	{
		// Ist ungefaehr eine vertikale Gerade!
		m_resultX = 0;
		m_resultY = 0;
		m_resultR = -1;   // Als "Markierung" fuer vert. Gerade
		return;
	}

    // zunaechst die Mittelwerte bestimmen
	for(int i=0; i<totalSize; i++)
	{
		double x = data[i].x;
		if (x<startX) continue;
		if (x>endeX) continue;
		double y = data[i].y;

        count++;
        x_ += x;
        y_ += y;
	}

	if (count <=3)
	{
		// Zu wenig Punkte fuer den Kreisfit
		m_resultX = 0;
		m_resultY = 0;
		m_resultR = 0;
		return;
	}

    x_ = x_/count;
    y_ = y_/count;

    // jetzt die notwendigen Summen bilden. Siehe PDF dazu.
    double u, v;
    double Suu=0, Suv=0, Svv=0, Suvv=0, Svuu=0, Suuu=0, Svvv=0;

	for(int i=0; i<totalSize; i++)
	{
		double x = data[i].x;
		if (x<startX) continue;
		if (x>endeX) continue;
		double y = data[i].y;

        u = x - x_;
        v = y - y_;
        Suu += u*u;
        Suv += u*v;
        Svv += v*v;
        Suvv += u*v*v;
        Svuu += v*u*u;
        Suuu += u*u*u;
        Svvv += v*v*v;
    }

	if (    ( fabs(Suuu) < 100.0 )
		 && ( fabs(Svvv) < 100.0 )
	   )
	{
		// Ist eine Gerade!
		m_resultX = 0;
		m_resultY = 0;
		m_resultR = -2;   // Als "Markierung" fuer schraege Gerade

		return;
	}

    // Matrix: erste Zeile a1 a2, zweite Zeile a3 a4
    double a1 = Suu;
    double a2 = Suv;
    double a3 = Suv;
    double a4 = Svv;

    double b1 = 0.5*(Suuu+Suvv);
    double b2 = 0.5*(Svvv+Svuu);

	double vc = (b1*a3-b2*a1) / (a2*a3-a4*a1);
    double uc = (b1 - a2*vc) / a1;

    double r = sqrt(uc*uc + vc*vc + (Suu+Svv)/count);

	if (r < iMinRadius)
	{
		// Ist ungefaehr eine Gerade!
		m_resultX = 0;
		m_resultY = 0;
		m_resultR = -3;   // Als "Markierung" fuer Kreis ueber schraege Gerade

		return;
	}

	_resultCircle.m_middleX = uc+x_+0.5;
	_resultCircle.m_middleY = vc+y_+0.5;
	_resultCircle.m_radius = r;

	m_resultX = (int)(uc+x_+0.5);
	m_resultY = (int)(vc+y_+0.5);
	m_resultR = (int)(r+0.5);

}

void CircleFitImpl::DrawCircleToImage(precitec::image::BImage& p_rImageOut, int middleX, int middleY, double radius)
{
	int maxY = p_rImageOut.size().height - 1;
	int maxX = p_rImageOut.size().width - 1;

	double angleRad;
    for (int angle=0; angle<360; angle++)
    {
        angleRad = angle * M_PI / 180.0;
		int x = (int)(0.4 + middleX + radius * cos(angleRad));
		if (x<0) x=0;
		if (x>maxX) x=maxX;
		int y = (int)(0.4 + middleY + radius * sin(angleRad));
		if (y<0) y=0;
		if (y>maxY) y=maxY;
		p_rImageOut[y][x] = 255;
        //result.AddOverlayPoint((int)(0.4 + x0 + radius * cos(angleRad)), (int)(0.4 + y0 + radius * sin(angleRad)));
    }
}

int CircleFitImpl::getX()
{
	return m_resultX;
}

int CircleFitImpl::getY()
{
	return m_resultY;
}

int CircleFitImpl::getR()
{
	return m_resultR;
}


//////////////////////// LookupTable


CircleFitImpl::LookUpTables::LookUpTables()
{
    double angleRad;
    for (int angle=0; angle<360; angle++)
    {
        angleRad = angle * M_PI / 180.0;
        sinTable[angle] = cos(angleRad);
        cosTable[angle] = sin(angleRad);
    }
}


////////////////// CircleHoughImpl

CircleHoughImpl::CircleHoughImpl()
:m_accumulationMatrix{}
{
}

CircleHoughImpl::~CircleHoughImpl()
{

}

Circle CircleHoughImpl::GetResult()
{
    return _resultCircle;
}

CircleHoughImpl::AccumulationMatrix::AccumulationMatrix()
{
    _oAccuMaSize = 0;
	_maxY = 0;
	_maxX = 0;
}

CircleHoughImpl::AccumulationMatrix::~AccumulationMatrix()
{
    if (_oAccuMaSize > 0) delete [] _pAccuMa;
}

void CircleHoughImpl::AccumulationMatrix::update(double radius, geo2d::Point minAllowedCenterPosition, geo2d::Point maxAllowedCenterPosition)
{
    geo2d::Size allowedCenterArea {maxAllowedCenterPosition.x - minAllowedCenterPosition.x +1,
        maxAllowedCenterPosition.y - minAllowedCenterPosition.y +1};

    if (allowedCenterArea.width <= 0 || allowedCenterArea.height <= 0)
    {
        resizeArray({0,0});
        assert(empty());
        return;
    }


    //we store the offset information in order to be able to convert between accumulator matrix coordinates and image coordinates
    m_accumulatorTrafoX = -minAllowedCenterPosition.x;
    m_accumulatorTrafoY = -minAllowedCenterPosition.y;


    resizeArray(allowedCenterArea);

    m_accumulatorOffsetsLookUpTableCoarse.update(radius, m_accumulatorTrafoX, m_accumulatorTrafoY);
    m_accumulatorOffsetsLookUpTableFine.update(radius, m_accumulatorTrafoX, m_accumulatorTrafoY);

#ifndef NDEBUG

    assert(extractPosition(0).xImage <= minAllowedCenterPosition.x);
    assert(extractPosition(0).yImage <= minAllowedCenterPosition.y);

    assert(extractPosition(_maxX*(_maxY - 1) + _maxX -1).xImage >= maxAllowedCenterPosition.x);
    assert(extractPosition(_maxX*(_maxY - 1) + _maxX -1).yImage >= maxAllowedCenterPosition.y);

    for (int index : {fromImageCoordToAccumulatorIndex(minAllowedCenterPosition.x, minAllowedCenterPosition.y),
                        fromImageCoordToAccumulatorIndex(maxAllowedCenterPosition.x, maxAllowedCenterPosition.y)})
    {
        assert(index >= 0 && index < _maxX * _maxY);
    }

#endif
}

bool CircleHoughImpl::DoSingleCircleHough(const image::BImage & p_rImageIn, double radius, byte threshold, SearchType searchOutsideROI, bool coarse)
{
    //there is an offset between the allowed candidate area and the image size depending on the search type
    geo2d::Point minAllowedCenterPosition{0,0};
    geo2d::Point maxAllowedCenterPosition{p_rImageIn.width()-1, p_rImageIn.height() -1};

    int intRadius = std::round(radius);
    switch (searchOutsideROI)
    {
        case SearchType::OnlyCenterInsideROI:
            // the area allowed for the circle center candidates is the whole image
            break;
        case SearchType::AllowCenterOutsideROI:
            // the area allowed for the circle center candidates is big: the whole image plus a border as wide as the radius
            minAllowedCenterPosition.x = -intRadius;
            minAllowedCenterPosition.y =  -intRadius;
            maxAllowedCenterPosition.x = p_rImageIn.width()-1 + intRadius;
            maxAllowedCenterPosition.y = p_rImageIn.height()-1 + intRadius;
            break;
        case SearchType::OnlyCompleteCircleInsideROI:
            // the area allowed for the circle center candidates is small : whole image minus a border as wide as the radius
            minAllowedCenterPosition.x = intRadius;
            minAllowedCenterPosition.y = intRadius;
            maxAllowedCenterPosition.x = p_rImageIn.width()-1 - intRadius;
            maxAllowedCenterPosition.y = p_rImageIn.height()-1 - intRadius;
            break;
    }

    //the accumulator matrix will have the same size as the allowed candidate area


    m_accumulationMatrix.update( radius, minAllowedCenterPosition, maxAllowedCenterPosition);
    if (m_accumulationMatrix.empty())
    {
        return false;
    }
    // search on the whole roi (even in the most restrictive case, the whole roi may contain a valid circle point)
    const int startX = 0;
    const int endX = p_rImageIn.size().width;
    const int startY = 0;
    const int endY = p_rImageIn.size().height;

    
    if (coarse)
    {
        for(int y=startY; y<endY; y++)
        {
            auto p_curVal= p_rImageIn.rowBegin(y) + startX;
            for (int x=startX; x<endX; x++, ++p_curVal )
            {
                if (*p_curVal > threshold)
                {
                    m_accumulationMatrix.increaseScore<true>(x,y);
                }
            }
        }
    }
    else
    {

        for(int y=startY; y<endY; y++)
        {
            auto p_curVal= p_rImageIn.rowBegin(y) + startX;
            for (int x=startX; x<endX; x++, ++p_curVal )
            {
                if (*p_curVal > threshold)
                {
                    m_accumulationMatrix.increaseScore<false>(x,y);
                }
            }
        }
    }

    return true;
}

bool CircleHoughImpl::DoSingleCircleHough(std::vector<geo2d::DPoint> p_rPointList, double radius, geo2d::Point minAllowedCenterPosition, geo2d::Point maxAllowedCenterPosition, bool coarse)
{
    m_accumulationMatrix.update( radius, minAllowedCenterPosition, maxAllowedCenterPosition);
    if (m_accumulationMatrix.empty())
    {
        return false;
    }
    // search on the whole roi (even in the most restrictive case, the whole roi may contain a valid circle point)
    if (coarse)
    {
        for(auto point : p_rPointList)
        {
            m_accumulationMatrix.increaseScore<true>(point.x, point.y);
        }
    }
    else
    {
        for(auto point : p_rPointList)
        {
            m_accumulationMatrix.increaseScore<false>(point.x, point.y);
        }

    }

    return true;
}


CircleHoughImpl::PositionInAccuMa CircleHoughImpl::AccumulationMatrix::extractMax() const
{
    assert(_maxX*_maxY > 0);
	// Max in Akkumulator-Matrix suchen
	auto itMax = std::max_element(_pAccuMa, _pAccuMa + _maxX*_maxY);
    auto index = itMax - _pAccuMa;
    return extractPosition(index);

}


CircleHoughImpl::PositionInAccuMa CircleHoughImpl::AccumulationMatrix::extractPosition(unsigned int index) const
{

    PositionInAccuMa result;
    result.xAccuMa = index % _maxX;
    result.yAccuMa = index / _maxX;
    result.valueAccuMa = _pAccuMa[index];
    result.xImage = result.xAccuMa - m_accumulatorTrafoX;
    result.yImage = result.yAccuMa - m_accumulatorTrafoY;
    return result;
}

geo2d::DPoint CircleHoughImpl::AccumulationMatrix::extractMeanImagePosition(const PositionInAccuMa & startPosition) const
{
    if (startPosition.valueAccuMa == 0)
    {
        return {startPosition.xImage, startPosition.yImage};
    }

    struct ConnectedPointsHelper
    {
        int sumX = 0;
        int sumY = 0;
        int count = 0;
        int xCenter=0;
        bool update(int xFirst, int n, int y)
        {
            if (n == 0)
            {
                return false;
            }

            //update the center x of the connected points in this line (integer division)
            xCenter = xFirst + (n-1)/2;
            sumX += (xCenter*n + ((n-1)%2)*(n/2));
            sumY += n* y;
            count += n;
            return true;
        }

    };

    ConnectedPointsHelper oConnectedPointsHelper;

    // search the biggest blob above threshold, connected to the input position (fast search, not a real 4-neighborhood)

    const auto & threshold = startPosition.valueAccuMa;


    auto findInLine = [&threshold, &oConnectedPointsHelper, this](const int x, const int y)
    {
        const auto rowBegin = _pAccuMa + y * _maxX;
        const auto rowEnd = rowBegin + _maxX;

        //check points at the right of x (x included)
        int const * curPointAccuma = rowBegin + x;
        if (curPointAccuma == rowEnd || *curPointAccuma < threshold)
        {
            return oConnectedPointsHelper.update(x, 0, y);
        }
        while(curPointAccuma < rowEnd && *curPointAccuma  >= threshold)
        {
            curPointAccuma++;
        }
        int xEnd = curPointAccuma - rowBegin;

        //check points at the left of x (x excluded)
        curPointAccuma = rowBegin + x -1;
        if (curPointAccuma < rowBegin || *curPointAccuma < threshold)
        {
            return oConnectedPointsHelper.update(x, (xEnd - x   ), y);
        }
        while(curPointAccuma >= rowBegin && *curPointAccuma  >= threshold)
        {
            curPointAccuma --;
        }

        int xFirstIncluded = curPointAccuma - rowBegin + 1;

        return oConnectedPointsHelper.update(xFirstIncluded, (xEnd - xFirstIncluded), y);
    };

    //check input line
    bool found = findInLine(startPosition.xAccuMa,startPosition.yAccuMa);
    const int xCenterInputLine = oConnectedPointsHelper.xCenter;

    // check below input line
    oConnectedPointsHelper.xCenter = xCenterInputLine;
    for (int y = startPosition.yAccuMa+1; y < _maxY && found; y++)
    {
        found =  findInLine(oConnectedPointsHelper.xCenter,y);
    }

    // check above input line
    oConnectedPointsHelper.xCenter = xCenterInputLine;
    for (int y = startPosition.yAccuMa-1; y >= 0 && found; y--)
    {
        found =  findInLine(oConnectedPointsHelper.xCenter,y);
    }

    assert(oConnectedPointsHelper.count > 0);

    double meanX = oConnectedPointsHelper.sumX / double(oConnectedPointsHelper.count);
    double meanY = oConnectedPointsHelper.sumY / double(oConnectedPointsHelper.count);

    return {meanX - m_accumulatorTrafoX , meanY - m_accumulatorTrafoY};

}


unsigned int CircleHoughImpl::AccumulationMatrix::fromImageCoordToAccumulatorIndex(int xImage, int yImage) const
{
    auto xAccuMa = xImage + m_accumulatorTrafoX;
    auto yAccuMa = yImage + m_accumulatorTrafoY;
    return yAccuMa * _maxX + xAccuMa;
}

void CircleHoughImpl::extractCandidates(std::vector<hough_circle_t> & p_rResult, const PointMatrix & p_rMatrix, int numberMax,
									double curRadius, CircleHoughParameters::ScoreType scoreType, double tolerance_degrees, bool coarse)
{
        //prepare computation of the final score
        const double sumScoresAccumulator = coarse ? NUMBER_OF_SAMPLES_COARSE : NUMBER_OF_SAMPLES_FINE;
        if (scoreType == CircleHoughParameters::ScoreType::LongestConnectedArc)
        {
            m_verificationOffsetsLookUpTable.update(curRadius);
        }

		PositionInAccuMa maxInAccuMa;
		for (int i = 0; i < numberMax; i++)
		{
			if (i > 0)
			{
				// Optional Bereich loeschen, um weitere Maxima zu finden
				m_accumulationMatrix.deleteArea(maxInAccuMa.xAccuMa, maxInAccuMa.yAccuMa, curRadius/2);
			}
			maxInAccuMa = m_accumulationMatrix.extractMax();
            auto refinedMaxPosition = m_accumulationMatrix.extractMeanImagePosition(maxInAccuMa);
            Circle curCircle {refinedMaxPosition.x, refinedMaxPosition.y, curRadius};
            switch(scoreType)
            {
                case CircleHoughParameters::ScoreType::Accumulator:
                    p_rResult.emplace_back(curCircle , maxInAccuMa.valueAccuMa * 100.0/ sumScoresAccumulator  );
                    break;
                case CircleHoughParameters::ScoreType::LongestConnectedArc:
                {
                    double score = m_verificationOffsetsLookUpTable.computeScore(p_rMatrix, {maxInAccuMa.xImage, maxInAccuMa.yImage}, tolerance_degrees);
                    p_rResult.emplace_back( curCircle, score);
                }
            }

#if DEBUG_CIRCLEHOUGH
			m_accumulationMatrix.exportMatrix("radius"+std::to_string((int)(curRadius+0.5))+"_"+std::to_string(i), p_rResult.back());
#endif
        }

}

std::vector<hough_circle_t> CircleHoughImpl::DoCircleHough(const precitec::image::BImage& p_rImageIn, byte threshold,
									SearchType searchOutsideROI, const CircleHoughParameters & parameters)
{
    auto & radiusStart = parameters.m_oRadiusStart;
    auto & radiusEnd = parameters.m_oRadiusEnd;
    auto radiusStep = parameters.m_oRadiusStep; //copy, to modify if the number of iterations is too high
    auto & numberMax = parameters.m_oNumberMax;

	std::vector<hough_circle_t> oResult;
	if (radiusStart > radiusEnd || radiusStart <= 0 || radiusEnd <= 0 || radiusStep <= 0)
	{
		return oResult;
	}


	int numCandidatesRadii = adjustRadiusStep(radiusStart, radiusEnd, radiusStep);
	oResult.reserve(numberMax * numCandidatesRadii);

    PointMatrix matrix(p_rImageIn, threshold);
	for (double curRadius = radiusEnd; curRadius >= radiusStart; curRadius -= radiusStep)
	{
		bool ok = DoSingleCircleHough( p_rImageIn, curRadius, threshold, searchOutsideROI, parameters.m_oCoarse);
		if (!ok)
		{
			continue;
		}
		extractCandidates(oResult,  matrix, numberMax, curRadius, parameters.m_oScoreType, parameters.m_oConnectedArcToleranceDegrees, parameters.m_oCoarse);
	}
	return oResult;
}


//verify candidate circle on input image
template<int NUMBER_OF_VERIFICATION_SAMPLES>
double CircleHoughImpl::VerificationLookUpTable<NUMBER_OF_VERIFICATION_SAMPLES>::computeScore(const PointMatrix & p_rMatrix,  geo2d::DPoint center, double tolerance_degrees)
{
            int lastIndex = NUMBER_OF_VERIFICATION_SAMPLES - 1;
            assert(lastIndex == (int) m_table.size() -1);

            int score = 0; //length of the longest connected arc


            int connectedContourPoints = 0;
            int missingContourPoints = 0;
            const int toleranceMissingPoints = NUMBER_OF_VERIFICATION_SAMPLES * tolerance_degrees /360;
            auto checkContourPoint = [&p_rMatrix, &connectedContourPoints, &missingContourPoints, &center](geo2d::DPoint offset)
            {

                if  ( p_rMatrix.valid(center + offset))
                {
                    missingContourPoints = 0;
                    connectedContourPoints++;
                }
                else
                {
                    missingContourPoints++;
                }
            };


            bool initialized = false;
            int partialFirstArc = 0;

            for (int index = 0; index <= lastIndex; index ++)
            {
                auto & offset = m_table[index];
                checkContourPoint(offset);
                if (missingContourPoints > toleranceMissingPoints)
                {
                    if (!initialized)
                    {
                        partialFirstArc = connectedContourPoints;
                        initialized = true;
                    }
                    else
                    {
                        score = std::max(connectedContourPoints, score);
                    }
                    connectedContourPoints = 0;
                }
            }

            //join the partial first and last arc
            connectedContourPoints += partialFirstArc;
            score = std::max(connectedContourPoints, score);

            return double(score) * 360.0 / double(NUMBER_OF_VERIFICATION_SAMPLES) ; // return result in degrees
}

std::vector<hough_circle_t> CircleHoughImpl::DoCircleHough(const std::vector<geo2d::DPoint> & p_rPointList, const CircleHoughParameters & parameters)
{
    if (p_rPointList.size() == 0)
    {
        return {};
    }

    auto & radiusEnd = parameters.m_oRadiusEnd;

    auto boundingBoxCorners = pointListBoundingBoxCorners(p_rPointList);

    geo2d::Point minAllowedCenterPosition ( std::floor(boundingBoxCorners.first.x - radiusEnd),
                                            std::floor(boundingBoxCorners.first.y - radiusEnd));
    geo2d::Point maxAllowedCenterPosition ( std::ceil(boundingBoxCorners.first.x + radiusEnd),
                                            std::ceil(boundingBoxCorners.first.y + radiusEnd));

    return DoCircleHough(p_rPointList, minAllowedCenterPosition, maxAllowedCenterPosition, parameters);
}

//TODO use double or int for posiion?
std::vector<hough_circle_t> CircleHoughImpl::DoCircleHough(const std::vector<geo2d::DPoint> & p_rPointList,
                                                           geo2d::Point minAllowedCenterPosition, geo2d::Point maxAllowedCenterPosition,
                                                           const CircleHoughParameters & parameters)
{
#if DEBUG_CIRCLEHOUGH
    m_accumulationMatrix.exportInput("input", p_rPointList);
#endif

	std::vector<hough_circle_t> oResult;

    auto & radiusStart = parameters.m_oRadiusStart;
    auto & radiusEnd = parameters.m_oRadiusEnd;
    auto radiusStep = parameters.m_oRadiusStep; //copy radiusStep, it will be modified if necessary
    auto & numberMax = parameters.m_oNumberMax;

	if (radiusStart > radiusEnd || radiusStart <= 0 || radiusEnd <= 0 || radiusStep <= 0)
	{
		return oResult;
	}

	int numCandidatesRadii = adjustRadiusStep(radiusStart, radiusEnd, radiusStep);
	oResult.reserve(numberMax * numCandidatesRadii);

    bool verifyConnectedArc = (parameters.m_oScoreType == CircleHoughParameters::ScoreType::LongestConnectedArc);
    const bool hasPointMatrix = verifyConnectedArc;

    PointMatrix pointMatrix;
    geo2d::DPoint topLeftPoint, bottomRightCorner;
    if (hasPointMatrix)
    {
        pointMatrix = PointMatrix{p_rPointList};
        topLeftPoint = pointMatrix.topLeftCorner();
        bottomRightCorner = pointMatrix.bottomRightCorner();
    }
    else
    {
        std::tie(topLeftPoint, bottomRightCorner) = pointListBoundingBoxCorners(p_rPointList);
    }
    minAllowedCenterPosition.x = std::max(minAllowedCenterPosition.x, (int)(std::floor(topLeftPoint.x - radiusEnd)));
    minAllowedCenterPosition.y = std::max(minAllowedCenterPosition.y, (int)(std::floor(topLeftPoint.y - radiusEnd)));
    maxAllowedCenterPosition.x = std::min(maxAllowedCenterPosition.x, (int)(std::ceil(bottomRightCorner.x + radiusEnd)));
    maxAllowedCenterPosition.y = std::min(maxAllowedCenterPosition.y, (int)(std::ceil(bottomRightCorner.y + radiusEnd)));


	for (double curRadius = radiusEnd; curRadius >= radiusStart; curRadius -= radiusStep)
	{
        std::vector<geo2d::DPoint> oCandidateCircleCenters;
		bool ok = DoSingleCircleHough( p_rPointList, curRadius, minAllowedCenterPosition, maxAllowedCenterPosition, parameters.m_oCoarse);
        if (!ok)
        {
            continue;
        }
        if (verifyConnectedArc)
        {
            m_verificationOffsetsLookUpTable.update(curRadius);
        }

		extractCandidates(oResult, pointMatrix, numberMax, curRadius, parameters.m_oScoreType, parameters.m_oConnectedArcToleranceDegrees, parameters.m_oCoarse);

	}

	return oResult;
}

void CircleHoughImpl::DrawCircleToImage(precitec::image::BImage& p_rImageOut, int middleX, int middleY, double radius)
{
	int maxY = p_rImageOut.size().height - 1;
	int maxX = p_rImageOut.size().width - 1;

	unsigned char color[5];

	color[0]=0;
	color[1]=64;
	color[2]=128;
	color[3]=192;
	color[4]=255;

	double angleRad;
    for (int angle=0; angle<360; angle++)
    {
        angleRad = angle * M_PI / 180.0;
		int x = (int)(0.4 + middleX + radius * cos(angleRad));
		if (x<0) x=0;
		if (x>maxX) x=maxX;
		int y = (int)(0.4 + middleY + radius * sin(angleRad));
		if (y<0) y=0;
		if (y>maxY) y=maxY;
		p_rImageOut[y][x] = color[angle % 5];
    }
}



void CircleHoughImpl::AccumulationMatrix::deleteArea(int x, int y, int size)
{
    int first_x= std::max(0, x-size);
    int last_x = std::min(x+size, _maxX -1);
    int n = last_x - first_x + 1;

    for (int _y=std::max(0,y-size), last_y = std::min(y+size, _maxY -1); _y <= last_y; _y++)
    {
        int index = first_x + _y*_maxX;
        std::fill_n(_pAccuMa + index, n, 0);
    }

}

void CircleHoughImpl::AccumulationMatrix::exportInput(std::string label, BImage image) const
{

    export_counter++;

    auto imageCopy = image.isContiguos() ? image : BImage(image.size());
    if (!image.isContiguos())
    {
        image.copyPixelsTo(imageCopy);
    }

    std::ostringstream oFilename;
    oFilename << "/tmp/circleHough_" << export_counter <<"__" << label << ".bmp";

    fileio::Bitmap oBitmap(oFilename.str(), imageCopy.width(), imageCopy.height(),false);
    if (oBitmap.isValid())
    {
        oBitmap.save(imageCopy.data());
    }
    else
    {
        wmLog(eError, oFilename.str() + "Error saving\n" );
    };
}


void CircleHoughImpl::AccumulationMatrix::exportInput(std::string label, std::vector<geo2d::DPoint> pointList) const
{
    export_counter++;

    std::ostringstream oFilename;
    oFilename << "/tmp/circleHough_" << export_counter <<"__" << label << ".bmp";


    PointMatrix pointMatrix(pointList);
    const auto & equivalentImage = pointMatrix.m_image;

    fileio::Bitmap oBitmap(oFilename.str(), equivalentImage.width(), equivalentImage.height(),false);
    if (oBitmap.isValid())
    {
        oBitmap.save(equivalentImage.data());
    }
    else
    {
        wmLog(eError, oFilename.str() + "Error saving\n" );
    };

}


void CircleHoughImpl::AccumulationMatrix::exportMatrix(std::string label, hough_circle_t chosenCandidate) const
{
    BImage rImage(geo2d::Size( _maxX, _maxY));

    auto minmax = std::minmax_element(_pAccuMa, _pAccuMa + _maxX*_maxY);
    int scoreMin = *minmax.first;
    int delta = (*minmax.second -scoreMin) + 1;
    assert(rImage.isContiguos());
    std::transform(_pAccuMa, _pAccuMa + _maxX*_maxY, rImage.begin(), [&](int score){return std::max(50,(score - scoreMin)*255/delta);});

    std::ostringstream nameTemplate;
    nameTemplate << "/tmp/circleHough_" << export_counter << "_accum_" + label;

    fileio::Bitmap oBitmap(nameTemplate.str()+".bmp", rImage.width(), rImage.height(),false);
    if (oBitmap.isValid())
    {
        oBitmap.save(rImage.data());
    }
    else
    {
        wmLog(eError, "Error saving accumulator image %s\n", label.c_str() );
    };

    {
        std::ofstream headers("/tmp/circleHough_info.txt");
        headers << "Counter;label;"
                    "CandidateX;CandidateY;CandidateScore;"
                    "MatrixW;MatrixH;"
                    "MaxMatrixX;MaxMatrixY;MaxMatrixValue;"
                    "MaxImageX;MaxImageY; ;\n";
    }

    std::ofstream oInfoCsv("/tmp/circleHough_info.csv",std::ios_base::app);
    if (oInfoCsv.good())
    {
        auto info_max = extractPosition(minmax.second - _pAccuMa);

        oInfoCsv << export_counter << ";" << label <<";"

            << chosenCandidate.first.m_middleX <<";" << chosenCandidate.first.m_middleY << ";" << chosenCandidate.second << ";"
            <<_maxX<<";"<<_maxY<<";"
            << info_max.xAccuMa <<";" << info_max.yAccuMa << ";" << info_max.valueAccuMa << ";"
            << info_max.xImage << ";" << info_max.yImage << ";";

        oInfoCsv << "\n";
    }
}

template <int NUMBER_OF_SAMPLES>
void CircleHoughImpl::AccumulationMatrix::OffsetLookUpTable<NUMBER_OF_SAMPLES>::update(double radius, int m_accumulatorTrafoX, int m_accumulatorTrafoY)
{
    static const std::array< std::pair< double, double >, NUMBER_OF_SAMPLES > sCircleLookupTable = []()
    {
        std::cout << "create Circle Lookup Table" << std::endl;
        std::array< std::pair< double, double >, NUMBER_OF_SAMPLES > oCircleLookupTable;
        double delta_rad = M_PI * 2 / NUMBER_OF_SAMPLES;
        auto itYX = oCircleLookupTable.begin();
        for (int i = 0; i < NUMBER_OF_SAMPLES; i++, itYX++)
        {
            double angle_rad = i * delta_rad;
            itYX->first = std::sin(angle_rad); //y
            itYX->second = std::cos(angle_rad); //x
        }
        assert(itYX == oCircleLookupTable.end());
        std::sort(oCircleLookupTable.begin(),oCircleLookupTable.end()); //sorted by y, not by angle
        return oCircleLookupTable;
    }();

    m_radius = radius;

    auto ityx = m_data.begin();
    for (auto & rCircleyx : sCircleLookupTable)
    {
        *ityx = {radius * rCircleyx.first - m_accumulatorTrafoY - 0.5, // for the sign, check the usage in  DoSingleCircleHough
                 radius * rCircleyx.second - m_accumulatorTrafoX - 0.5
                };
        ++ityx;
    }
    assert( ityx == m_data.end());
}

template<bool coarse>
void CircleHoughImpl::AccumulationMatrix::increaseScore(double imageX, double imageY)
{

    auto evaluate = [this](double imageX, double imageY, std::pair< double, double > yx)
    {
        double x0 = imageX- yx.second; // x - radius * cos(theta) + m_accumulatorTrafoX  + 0.5
        if (x0 < 0.5 || x0 >= _maxX)
        {
            return;
        }
        double y0 = imageY - yx.first;
        if (y0 < 0.5 || y0 >= _maxY)
        {
            return;
        }
        int index = static_cast<int>(x0) + static_cast<int>(y0)*_maxX;
        assert(index < (_maxX * _maxY));
        _pAccuMa[index] ++;
    };

    if (coarse)
    {
        for (auto & yx : m_accumulatorOffsetsLookUpTableCoarse.getOffsets())
        {
            evaluate(imageX, imageY, yx);
        }
    }
    else
    {
        for (auto & yx : m_accumulatorOffsetsLookUpTableFine.getOffsets())
        {
            evaluate(imageX, imageY, yx);
        }
    }
}

template<int NUMBER_OF_VERIFICATION_SAMPLES>
CircleHoughImpl::VerificationLookUpTable<NUMBER_OF_VERIFICATION_SAMPLES>::VerificationLookUpTable()
{
    update(0.0);
}

template<int NUMBER_OF_VERIFICATION_SAMPLES>
void CircleHoughImpl::VerificationLookUpTable<NUMBER_OF_VERIFICATION_SAMPLES>::update(double radius)
{
    //similar to updateAccumulatorOffsetsLookUpTable, but not sorted by y
    static const std::array< geo2d::DPoint, NUMBER_OF_VERIFICATION_SAMPLES > sVerificationCircleLookupTable = []()
        {
            std::cout << "create Verification Circle Lookup Table" << std::endl;
            std::array< geo2d::DPoint, NUMBER_OF_VERIFICATION_SAMPLES > oCircleLookupTable;
            double delta_rad = M_PI * 2 / NUMBER_OF_VERIFICATION_SAMPLES;
            auto it = oCircleLookupTable.begin();
            for (int i = 0; i < NUMBER_OF_VERIFICATION_SAMPLES; i++, it++)
            {
                double angle_rad = i * delta_rad;
                it->y = std::sin(angle_rad); //y
                it->x = std::cos(angle_rad); //x
            }
            assert(it == oCircleLookupTable.end());
            return oCircleLookupTable;
        }();

    auto it = m_table.begin();
    for (auto & rUnitOffset : sVerificationCircleLookupTable)
    {
            it->x = radius * rUnitOffset.x - 0.5;
            it->y = radius * rUnitOffset.y - 0.5;

        ++it;
    }
    assert( it == m_table.end());

}

void CircleHoughImpl::AccumulationMatrix::resizeArray(geo2d::Size newSize)
{
    if ( (_maxX * _maxY) < (newSize.area()) || _oAccuMaSize == 0 ) // First Call oder Dimensionen haben sich geaendert
    {
        if (_oAccuMaSize > 0) delete [] _pAccuMa; // Neue Dim., aber existiert => loeschen vor Neuanlegen

        _maxY = newSize.height;
        _maxX = newSize.width;

        _oAccuMaSize = _maxX*_maxY;
        _pAccuMa = new int [_oAccuMaSize](); //value initialization
    }
    else
    {
        //accumulator matrix has already enough elements, update only the bounds

        _maxY = newSize.height;
        _maxX = newSize.width;

        assert((int)_oAccuMaSize >= _maxX*_maxY);
        std::fill_n(_pAccuMa, _maxX*_maxY, 0); //Nullen der AccuMa muss immer vor SingleHough passieren -> Evtl. ueber Flag steuern
    }
    assert(std::all_of(_pAccuMa, _pAccuMa+(_maxX*_maxY), [](int val) {
        return val == 0;
    }));
}


CircleHoughImpl::PointMatrix::PointMatrix()
    :m_origin(0,0){}

CircleHoughImpl::PointMatrix::PointMatrix( BImage image, byte threshold)
: m_image(image), m_origin(0,0), m_threshold(threshold)
{
//to verify: with this costructor, a change of m_image could change the input image (shallow copy)
}

CircleHoughImpl::PointMatrix::PointMatrix( const std::vector<geo2d::DPoint> & pointList)
: m_threshold(250)
{
    if (pointList.empty())
    {
        m_image.clear();
        return;
    }
    geo2d::DPoint minCoordinates, maxCoordinates;
    std::tie(minCoordinates, maxCoordinates) = pointListBoundingBoxCorners(pointList);
    m_origin.x = std::floor(minCoordinates.x) - 1;
    m_origin.y = std::floor(minCoordinates.y) - 1;
    geo2d::Size imageSize( std::ceil(maxCoordinates.x) - m_origin.x + 1,
                        std::ceil(maxCoordinates.y) - m_origin.y + 1);

    m_image.resizeFill(imageSize,0);
    std::for_each(pointList.begin(), pointList.end(), [this](geo2d::DPoint point)
    {
        int x = point.x - m_origin.x;
        int y = point.y - m_origin.y;
        m_image.setValue(x, y, 255); //by construction, bigger than m_threshold;
        assert(valid(point));
    });

}


bool CircleHoughImpl::PointMatrix::valid(geo2d::DPoint point) const
{
    int x = point.x - m_origin.x;
    int y = point.y - m_origin.y;
    return (x >= 0 && y >= 0 && x < m_image.width() && y < m_image.height()
            && m_image.getValue(x,y) > m_threshold);

}



} // namespace filter
} // namespace precitec
