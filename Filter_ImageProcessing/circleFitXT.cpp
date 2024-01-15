/**
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			LB
 *  @date			12/2020
 *  @file
 *  @brief			Performs a circle fit
 */


// local includes
#include "circleFitXT.h"


#include "image/image.h"				///< BImage
#include "overlay/overlayPrimitive.h"	///< overlay
#include "module/moduleLogger.h"
#include "filter/algoArray.h"
#include "filter/algoPoint.h"
#include <fliplib/TypeToDataTypeImpl.h>
#include "circleFitImpl.h"


using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {




CircleFitXT::CircleFitXT() :
    TransformFilter( "CircleFitXT", Poco::UUID{"e893f2cd-3e7a-4c49-ab45-654c0af8b71f"} ),
	m_pPipeInPointList(nullptr),
	m_pPipeInRadiusMin(nullptr),
    m_pPipeInRadiusMax(nullptr),
    m_pPipeInValidXStart(nullptr),
    m_pPipeInValidYStart(nullptr),
    m_pPipeInValidXEnd(nullptr),
    m_pPipeInValidYEnd(nullptr),
	m_oPipeOutCircleX ( this, "CircleX" ),
	m_oPipeOutCircleY ( this, "CircleY" ),
	m_oPipeOutCircleR ( this, "CircleR" ),
	m_oPipeOutScore( this, "Score"),
	m_oAlgorithm(Algorithm::Hough),
	m_oHoughRadiusStep(1.0),
	m_oHoughScoreType(CircleHoughParameters::ScoreType::Accumulator),
    m_oHoughScoreThreshold(-1),
    m_oHoughCandidates(1)
{

    parameters_.add("Algorithm", Parameter::TYPE_int, static_cast<int>(m_oAlgorithm));
    parameters_.add("HoughRadiusStep", Parameter::TYPE_double, m_oHoughRadiusStep);
    parameters_.add("HoughScoreType", Parameter::TYPE_int, static_cast<int>(m_oHoughScoreType));
    parameters_.add("HoughScoreThreshold", Parameter::TYPE_int, m_oHoughScoreThreshold);
    parameters_.add("HoughCandidates", Parameter::TYPE_int, m_oHoughCandidates);

	m_paintX = m_paintY = m_paintR = 0;

    setInPipeConnectors({
        {Poco::UUID("71762a79-3a07-49b1-af48-810ae43f5248"), m_pPipeInPointList, "PointList", 1, "PointList"},
        {Poco::UUID("8324b97a-4335-4859-ae14-26115b61a3a4"), m_pPipeInRadiusMin, "RadiusMin", 1, "RadiusMin"},
        {Poco::UUID("edc0a03a-7076-461e-ab70-cba6e386fad4"), m_pPipeInRadiusMax, "RadiusMax", 1, "RadiusMax"},
        {Poco::UUID("04ad9498-3315-4a66-8a36-ec907fc497d1"), m_pPipeInValidXStart, "XStart", 1, "XStart"},
        {Poco::UUID("78982ff1-e754-49be-9e1b-eba3f19a2ea6"), m_pPipeInValidYStart, "YStart", 1, "YStart"},
        {Poco::UUID("71f85c9b-3935-4478-90d0-7fc599f474f8"), m_pPipeInValidXEnd, "XEnd", 1, "XEnd"},
        {Poco::UUID("421ce952-c0ca-4dea-8a05-caf3dd990c2d"), m_pPipeInValidYEnd, "YEnd", 1, "YEnd"}});
    
    setOutPipeConnectors({
        {Poco::UUID("58806ce0-5773-47c8-84d7-d5af1ce10595"), &m_oPipeOutCircleX, "CircleX", 0, ""},
        {Poco::UUID("f93e72f4-fd58-4006-a6ce-74603a9a5c85"), &m_oPipeOutCircleY, "CircleY", 0, ""},
        {Poco::UUID("5b1b5c35-663f-46d6-8ece-483e281cca56<"), &m_oPipeOutCircleR, "CircleR", 0, ""},
        {Poco::UUID("872cdb36-6616-4308-8be9-8488eaa64c48"), &m_oPipeOutScore, "Score", 0, ""}
    });
    
    setVariantID(Poco::UUID("178cd9aa-6e2d-46c8-9741-11586ad43d92"));
}

void CircleFitXT::setParameter() {
	TransformFilter::setParameter();
    m_oAlgorithm = static_cast<Algorithm>(parameters_.getParameter("Algorithm").convert<int>());
    m_oHoughRadiusStep = parameters_.getParameter("HoughRadiusStep").convert<double>();
    m_oHoughScoreType = static_cast<CircleHoughParameters::ScoreType>(parameters_.getParameter("HoughScoreType").convert<int>()); //default: Accumulator
    m_oHoughScoreThreshold = parameters_.getParameter("HoughScoreThreshold");
    m_oHoughCandidates = parameters_.getParameter("HoughCandidates");

} // setParameter

bool CircleFitXT::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    const auto &inPipes = inPipeConnectors();
    if (p_rPipe.tag() == inPipes[0].tag())
    {
        m_pPipeInPointList = dynamic_cast<pointlist_pipe_t*> (&p_rPipe);
    }
    else if (p_rPipe.tag() == inPipes[1].tag())
    {
        m_pPipeInRadiusMin = dynamic_cast<scalar_pipe_t*> (&p_rPipe);
    }
    else if (p_rPipe.tag() == inPipes[2].tag())
    {
        m_pPipeInRadiusMax = dynamic_cast<scalar_pipe_t*> (&p_rPipe);
    }
    else if (p_rPipe.tag() == inPipes[3].tag())
    {
        m_pPipeInValidXStart = dynamic_cast<scalar_pipe_t*> (&p_rPipe);
    }
    else if (p_rPipe.tag() == inPipes[4].tag())
    {
        m_pPipeInValidYStart = dynamic_cast<scalar_pipe_t*> (&p_rPipe);
    }
    else if (p_rPipe.tag() == inPipes[5].tag())
    {
        m_pPipeInValidXEnd = dynamic_cast<scalar_pipe_t*> (&p_rPipe);
    }
    else if (p_rPipe.tag() == inPipes[6].tag())
    {
        m_pPipeInValidYEnd = dynamic_cast<scalar_pipe_t*> (&p_rPipe);
    }
    else
    {
        wmLog(eWarning, "Unknown pipe %d for CircleFitXT \n", p_rPipe.tag().c_str());
        return false;
    }
    return BaseFilter::subscribe( p_rPipe, p_oGroup );

}

void CircleFitXT::proceedGroup(const void* sender, PipeGroupEventArgs& e)
{

	poco_assert_dbg( m_pPipeInPointList != nullptr); // to be asserted by graph editor


	const auto & rGeoVecDPointArrayIn = m_pPipeInPointList->read(m_oCounter);
    const auto & rGeoRadiusMinIn =  m_pPipeInRadiusMin->read(m_oCounter);
    const auto & rGeoRadiusMaxIn =  m_pPipeInRadiusMax->read(m_oCounter);
    const auto & rGeoXStartIn = m_pPipeInValidXStart->read(m_oCounter);
    const auto & rGeoYStartIn = m_pPipeInValidYStart->read(m_oCounter);
    const auto & rGeoXEndIn = m_pPipeInValidXEnd->read(m_oCounter);
    const auto & rGeoYEndIn = m_pPipeInValidYEnd->read(m_oCounter);

    const auto oGeoAnalysisResult = rGeoVecDPointArrayIn.analysisResult();
    
    if (inputIsInvalid(rGeoVecDPointArrayIn ) 
        || inputIsInvalid( rGeoRadiusMinIn ) || inputIsInvalid( rGeoRadiusMaxIn )
        || inputIsInvalid( rGeoXStartIn ) || inputIsInvalid( rGeoXStartIn )
        || inputIsInvalid( rGeoXEndIn ) || inputIsInvalid( rGeoXEndIn ))
    {
        m_oSpTrafo = nullptr;
        
        const interface::GeoDoublearray oNullResult( rGeoVecDPointArrayIn.context(), geo2d::Doublearray{1,0,eRankMin}, oGeoAnalysisResult, 0.0 );
        // send the data out ...
        preSignalAction();
        m_oPipeOutCircleX.signal( oNullResult );
        m_oPipeOutCircleY.signal( oNullResult );
        m_oPipeOutCircleR.signal( oNullResult );
        m_oPipeOutScore.signal( oNullResult );
        return;
    }

    auto hasDifferentSampling = [& rGeoVecDPointArrayIn] (const ImageContext & rContext)
    {
        return rContext.SamplingX_ != rGeoVecDPointArrayIn.context().SamplingX_ || rContext.SamplingY_ !=  rGeoVecDPointArrayIn.context().SamplingY_;
    };

    if ( hasDifferentSampling (rGeoRadiusMinIn.context())
        || hasDifferentSampling (rGeoRadiusMaxIn.context())
        || hasDifferentSampling (rGeoXStartIn.context())
        || hasDifferentSampling (rGeoYStartIn.context())
        || hasDifferentSampling (rGeoXEndIn.context())
        || hasDifferentSampling (rGeoYEndIn.context())
    )
    {
        std::ostringstream oMsg;
        oMsg << name() << ": Sampling in input scalar data not supported \n";
        wmLog(eWarning, oMsg.str());
    }

    unsigned int oNbInput = rGeoVecDPointArrayIn.ref().size();
    struct AlwaysUseFirstIndex
    {
        bool m_radiusMin;
        bool m_radiusMax;
        bool m_xStart;
        bool m_yStart;
        bool m_xEnd;
        bool m_yEnd;
    };
    
    auto checkAlwaysUseFirstIndex = [&oNbInput](const interface::GeoDoublearray & rGeoArray, std::string description)
        {
            auto oSize = rGeoArray.ref().size();
            assert(oSize != 0);
            if ( oSize != oNbInput && oSize != 1)
            {
                wmLog(eDebug, "Filter CircleFitXT: Received %u %s values for %u inputs. Only process first element, rest will be discarded.\n", oSize, description.c_str(), oNbInput);
            }
            return (oSize != oNbInput);
        };
        
    AlwaysUseFirstIndex oUseFirstIndex;
    oUseFirstIndex.m_radiusMin = checkAlwaysUseFirstIndex( rGeoRadiusMinIn, "RadiusMin");
    oUseFirstIndex.m_radiusMax = checkAlwaysUseFirstIndex( rGeoRadiusMaxIn, "RadiusMax");
    oUseFirstIndex.m_xStart = checkAlwaysUseFirstIndex( rGeoXStartIn, "XStart");
    oUseFirstIndex.m_yStart = checkAlwaysUseFirstIndex( rGeoYStartIn, "YStart");
    oUseFirstIndex.m_xEnd = checkAlwaysUseFirstIndex( rGeoXStartIn, "XEnd");
    oUseFirstIndex.m_yEnd = checkAlwaysUseFirstIndex( rGeoYStartIn, "YEnd");
        
	geo2d::Doublearray oOutX;
	geo2d::Doublearray oOutY;
	geo2d::Doublearray oOutR;
    geo2d::Doublearray oOutScore;
    oOutX.reserve(oNbInput);
    oOutY.reserve(oNbInput);
    oOutR.reserve(oNbInput);
    oOutScore.reserve(oNbInput);
    
    auto rContext = rGeoVecDPointArrayIn.context();
	m_oSpTrafo = rContext.trafo();
    bool m_oPaintResampled = true; //TODO filter parameter
    if (m_oPaintResampled)
    {
        m_oPaintSamplingX = rContext.SamplingX_;
        m_oPaintSamplingY = rContext.SamplingY_;
        if (m_oPaintSamplingX != m_oPaintSamplingY)
        {
            wmLog(eWarning, "Image stretched (samplingX %f, samplingY %f), not supported by paint routine\n", m_oPaintSamplingX, m_oPaintSamplingY);
        }
    }
    else
    {
        //do not resample point, for paint purposes the sampling is equivalent to 1
        m_oPaintSamplingX = 1.0;
        m_oPaintSamplingY = 1.0;
    }

    m_paintInPoints.clear();
    CircleHoughParameters oParameters;
    oParameters.m_oRadiusStep = m_oHoughRadiusStep;
    oParameters.m_oNumberMax = m_oHoughCandidates;
    oParameters.m_oCoarse = true;
    oParameters.m_oScoreType = m_oHoughScoreType;
    
    for (unsigned int pointListIndex = 0; pointListIndex < oNbInput; pointListIndex++)
    {

        if ( rGeoRadiusMinIn.ref().getRank()[oUseFirstIndex.m_radiusMin ? 0 : pointListIndex] == eRankMin
            || rGeoRadiusMaxIn.ref().getRank()[oUseFirstIndex.m_radiusMax ? 0 : pointListIndex] == eRankMin
            || rGeoXStartIn.ref().getRank()[oUseFirstIndex.m_xStart ? 0 : pointListIndex] == eRankMin
            || rGeoYStartIn.ref().getRank()[oUseFirstIndex.m_yStart ? 0 : pointListIndex] == eRankMin
            || rGeoXEndIn.ref().getRank()[oUseFirstIndex.m_xEnd ? 0 : pointListIndex] == eRankMin
            || rGeoYEndIn.ref().getRank()[oUseFirstIndex.m_yEnd ? 0 : pointListIndex] == eRankMin
        )
        {
            oOutX.getData().push_back(0.0);
            oOutY.getData().push_back(0.0);
            oOutR.getData().push_back(0.0);
            oOutX.getRank().push_back(eRankMin);
            oOutY.getRank().push_back(eRankMin);
            oOutR.getRank().push_back(eRankMin);
            continue;
        }


        const auto & rPointListArray = rGeoVecDPointArrayIn.ref()[pointListIndex];

        std::vector<geo2d::DPoint> validPointList;

        validPointList.reserve(rPointListArray.size());
        for (int index = 0, n = rPointListArray.size(); index < n ; index++)
        {
            if (rPointListArray.getRank()[index] != eRankMin)
            {
                validPointList.emplace_back(rPointListArray.getData()[index]);
            }
        } //FIXME is copying really necessary?

        double minRadius =  rGeoRadiusMinIn.ref().getData()[oUseFirstIndex.m_radiusMin ? 0 : pointListIndex];
        double maxRadius =  rGeoRadiusMaxIn.ref().getData()[oUseFirstIndex.m_radiusMax ? 0 : pointListIndex];
        double xStart = rGeoXStartIn.ref().getData()[oUseFirstIndex.m_xStart ? 0 : pointListIndex] + rGeoXStartIn.context().getTrafoX() - m_oSpTrafo->dx();
        double yStart = rGeoYStartIn.ref().getData()[oUseFirstIndex.m_yStart ? 0 : pointListIndex] + rGeoYStartIn.context().getTrafoY() - m_oSpTrafo->dy() ;
        double xEnd = rGeoXEndIn.ref().getData()[oUseFirstIndex.m_xEnd ? 0 : pointListIndex] + rGeoXEndIn.context().getTrafoX() - m_oSpTrafo->dx();
        double yEnd = rGeoYEndIn.ref().getData()[oUseFirstIndex.m_yEnd ? 0 : pointListIndex] + rGeoYEndIn.context().getTrafoY() - m_oSpTrafo->dy();

        m_paintAdvanced = m_oVerbosity >= eHigh;
        if (m_paintAdvanced)
        {

            m_paintValidCenterStart.x = xStart;
            m_paintValidCenterStart.y = yStart;
            m_paintValidCenterEnd.x = xEnd;
            m_paintValidCenterEnd.y = yEnd;
            ImageContext sourceImageContext{};
            m_paintInPoints = validPointList;
        }
        
        double currentX(0), currentY(0), currentR(0), currentScore(0);
        int currentRank =  eRankMin;

        switch(m_oAlgorithm)
        {
            case Algorithm::LeastSquares:
            {
                CircleFitImpl algoImpl;
                algoImpl.DoCircleFit( validPointList, 0, 100, minRadius);
                currentX = algoImpl.getX();
                currentY = algoImpl.getY();
                currentR = algoImpl.getR();
                if ((currentR >= minRadius && currentR <= maxRadius)
                    && (currentX >= xStart && currentX <= xEnd)
                    && (currentY >= yStart && currentY <= yEnd)
                    )
                {
                    currentRank = eRankMax;
                    currentScore = 100.0; // fit quality not implemented, score is just 100.0 or 0.0
                }
                else
                {
                    currentRank = eRankMin;
                    currentScore = 0.0;
                }
                break;
            }
            case Algorithm::Hough:
            {
                CircleHoughImpl algoImpl;
                oParameters.m_oRadiusStart = minRadius;
                oParameters.m_oRadiusEnd = maxRadius;
                oParameters.m_oConnectedArcToleranceDegrees = 1.0;

                geo2d::Point validAreaStart{(int)(std::floor(xStart)), (int)(std::floor(yStart))};
                geo2d::Point validAreaEnd{(int)(std::ceil(xEnd)), (int)(std::ceil(yEnd))};
                auto candidateCircles = algoImpl.DoCircleHough( validPointList, validAreaStart, validAreaEnd, oParameters);
                if ( candidateCircles.empty())
                {
                    currentR = 0;
                    currentRank = eRankMin;
                    currentScore = 0.0;
                }
                else
                {
                    auto maxElement = std::max_element(candidateCircles.begin(), candidateCircles.end(),
                                                    [] (const hough_circle_t & a, const hough_circle_t & b) {return a.second < b.second;});

                    currentX = maxElement->first.m_middleX;
                    currentY = maxElement->first.m_middleY;
                    currentR = maxElement->first.m_radius;
                    currentScore = maxElement->second;


                    if (maxElement->second >= m_oHoughScoreThreshold)
                    {
                        currentRank = eRankMax;
                    }
                    else
                    {
                        currentRank = eRankMin;
                    }
                }

                break;
            }
        }
        
        oOutX.getData().push_back(currentX);
        oOutY.getData().push_back(currentY);
        oOutR.getData().push_back(currentR);
        oOutScore.getData().push_back(currentScore);
        oOutX.getRank().push_back(currentRank);
        oOutY.getRank().push_back(currentRank);
        oOutR.getRank().push_back(currentRank);
        oOutScore.getRank().push_back(eRankMax);
        
    }

    m_paintX = oOutX.getData().back();
    m_paintY = oOutY.getData().back();
    m_paintR = oOutR.getData().back();


	const interface::GeoDoublearray oGeoDoubleOutX( rContext, oOutX, oGeoAnalysisResult, 1.0 );
	const interface::GeoDoublearray oGeoDoubleOutY( rContext, oOutY, oGeoAnalysisResult, 1.0 );
	const interface::GeoDoublearray oGeoDoubleOutR( rContext, oOutR, oGeoAnalysisResult, 1.0 );
	const interface::GeoDoublearray oGeoDoubleOutScore( rContext, oOutScore, oGeoAnalysisResult, 1.0 );
	// send the data out ...
	preSignalAction();
	m_oPipeOutCircleX.signal( oGeoDoubleOutX );
	m_oPipeOutCircleY.signal( oGeoDoubleOutY );
	m_oPipeOutCircleR.signal( oGeoDoubleOutR );
	m_oPipeOutScore.signal( oGeoDoubleOutScore );
}

void CircleFitXT::paint()
{
	if ((m_oVerbosity <= eNone)) {
		return;
	}

    if (m_oSpTrafo.isNull() || m_paintR <= 0)
    {
        return;
    }

	OverlayCanvas				&rCanvas(canvas<OverlayCanvas>(m_oCounter));
	OverlayLayer				&rLayerContour			( rCanvas.getLayerContour());
	OverlayLayer				&rLayerPosition			( rCanvas.getLayerPosition());

    geo2d::DPoint centerOnCanvas;

    centerOnCanvas.x = transformX(m_paintX, m_oPaintSamplingX, m_oSpTrafo->dx(), 1.0, 0.0);
    centerOnCanvas.y = transformY(m_paintY, m_oPaintSamplingY, m_oSpTrafo->dy(), 1.0, 0.0);

	rLayerPosition.add<OverlayCross>(centerOnCanvas.x, centerOnCanvas.y, Color::Green() );

    if ((m_oVerbosity <= eLow)) {
		return;
	}
    auto radiusOnCanvas = transformX(m_paintR, m_oPaintSamplingX, 0, 1.0, 0.0);
    rLayerContour.add<OverlayCircle>(centerOnCanvas.x, centerOnCanvas.y, radiusOnCanvas, Color::Green());

	if(!m_paintAdvanced)
	{
        return;
    }




    geo2d::DPoint validCenterStartOnCanvas, validCenterEndOnCanvas;
    validCenterStartOnCanvas.x = transformX(m_paintValidCenterStart.x, m_oPaintSamplingX, m_oSpTrafo->dx(), 1.0, 0);
    validCenterStartOnCanvas.y = transformX(m_paintValidCenterStart.y, m_oPaintSamplingY, m_oSpTrafo->dy(), 1.0, 0);
    validCenterEndOnCanvas.x = transformX(m_paintValidCenterEnd.x, m_oPaintSamplingX, m_oSpTrafo->dx(), 1.0, 0);
    validCenterEndOnCanvas.y = transformX(m_paintValidCenterEnd.y, m_oPaintSamplingY, m_oSpTrafo->dy(), 1.0, 0);

    rLayerContour.add<OverlayRectangle>( validCenterStartOnCanvas.x,
                                         validCenterStartOnCanvas.y,
                                        (validCenterEndOnCanvas.x + 1 - validCenterStartOnCanvas.x),
                                        (validCenterEndOnCanvas.y + 1 - validCenterStartOnCanvas.y),
                                        Color::Blue()
                                        );
    if (m_oVerbosity < eMax)
    {
        return;
    }



    double radius2 = radiusOnCanvas * radiusOnCanvas;
    for (auto & point : m_paintInPoints)
    {
        geo2d::DPoint pointOnCanvas;
        pointOnCanvas.x = transformX(point.x, m_oPaintSamplingX, m_oSpTrafo->dx(), 1.0, 0.0);
        pointOnCanvas.y = transformY(point.y, m_oPaintSamplingY, m_oSpTrafo->dy(), 1.0, 0.0);
        if ( std::abs(geo2d::distance2(point, centerOnCanvas) - radius2) < 250)
        {
            rLayerContour.add<OverlayPoint>((int) std::round(pointOnCanvas.x),
                                            (int) std::round(pointOnCanvas.y), Color::Cyan());
        }
        else
        {
            rLayerContour.add<OverlayPoint>((int) std::round(pointOnCanvas.x),
                                            (int) std::round(pointOnCanvas.y), Color::Blue());
        }
    }



} // paint


} // namespace filter
} // namespace precitec
