/**
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			OS
 *  @date			01/2015
 *  @file
 *  @brief			Performs a circle hough transformation
 */
#define _USE_MATH_DEFINES						/// pi constant

// local includes
#include "circleHough.h"

#include "image/image.h"				///< BImage
#include "overlay/overlayPrimitive.h"	///< overlay
#include "module/moduleLogger.h"
#include <math.h>
#include "circleFitImpl.h"

#include <fliplib/TypeToDataTypeImpl.h>



using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {


const std::string CircleHough::m_oFilterName 		( std::string("CircleHough") );

CircleHough::CircleHough() :
	TransformFilter( CircleHough::m_oFilterName, Poco::UUID{"C3112E68-6C4C-414A-BDCA-C45A5BD59035"} ),
	m_pPipeInImageFrame	( nullptr ),
	m_pPipeInRadiusStart( nullptr ),       //Added
	m_pPipeInRadiusEnd ( nullptr ),        //Added
	m_oPipeOutX	( this, "CenterX" ),
	m_oPipeOutY	( this, "CenterY" ),
	m_oPipeOutR	( this, "Radius" ),
	m_oPipeOutScore( this, "Score"),
	m_oRadiusStep       ( 5 ),
	m_oNumberMax        ( 1 ),
	m_oIntensityThreshold ( 10 ),
	m_oScoreType(CircleHoughParameters::ScoreType::Accumulator),
	m_oScoreThreshold ( -1 ),
    m_oSearchOutsideROI(SearchType::OnlyCompleteCircleInsideROI),
    m_oPaintInputImage(false)
{
	// Defaultwerte der Parameter setzen
	parameters_.add("RadiusStep",		Parameter::TYPE_double,	static_cast<double>(m_oRadiusStep));
	parameters_.add("NumberOfMax",		Parameter::TYPE_int,	static_cast<int>(m_oNumberMax));
	parameters_.add("IntensityThreshold",		Parameter::TYPE_int,	static_cast<int>(m_oIntensityThreshold));
    parameters_.add("ScoreType", Parameter::TYPE_int, static_cast<int>(m_oScoreType));
	parameters_.add("ScoreThreshold",		Parameter::TYPE_double,	static_cast<int>( m_oScoreThreshold ));
	parameters_.add("SearchOutsideROI",		Parameter::TYPE_bool,	m_oSearchOutsideROI != SearchType::OnlyCompleteCircleInsideROI);

    setInPipeConnectors({{Poco::UUID("4DE9DDD8-02E1-4FF0-9BEB-BE322DC2863E"), m_pPipeInImageFrame, "image", 1, "image"},
    {Poco::UUID("95E65F18-760A-4341-9A6C-E6492F76066E"), m_pPipeInRadiusStart, "radiusStart", 1, "radiusStart"},
    {Poco::UUID("E3B9C38A-82F9-4625-BCEB-2130D608920B"), m_pPipeInRadiusEnd, "radiusEnd", 1, "radiusEnd"}});
    setOutPipeConnectors({{Poco::UUID("254A9293-34A0-4161-B443-0DC1E070EFCE"), &m_oPipeOutX, "CenterX", 0, ""},
    {Poco::UUID("D10C46CD-A1C2-4694-AC34-ACEA739EA32E"), &m_oPipeOutY, "CenterY", 0, ""},
    {Poco::UUID("B89E611A-C65D-431C-A72C-344D6D33C5EB"), &m_oPipeOutR, "Radius", 0, ""},
    {Poco::UUID("1D6961FC-E7FE-4939-9198-AF0E695839E1"), &m_oPipeOutScore, "Score", 0, ""}});
    setVariantID(Poco::UUID("EDE6C56E-ECA1-491E-B3DB-EE5A098EAD40"));       //From Attributes.json
}

/*virtual*/ void CircleHough::setParameter() {
	TransformFilter::setParameter();
	m_oIntensityThreshold		= parameters_.getParameter("IntensityThreshold");
	m_oRadiusStep     			= parameters_.getParameter("RadiusStep");
	m_oNumberMax     			= parameters_.getParameter("NumberOfMax");

    switch(parameters_.getParameter("ScoreType").convert<int>())
    {
        case (int) CircleHoughParameters::ScoreType::Accumulator:
        default:
            m_oScoreType = CircleHoughParameters::ScoreType::Accumulator;
            break;
        case (int) CircleHoughParameters::ScoreType::LongestConnectedArc:
            m_oScoreType = CircleHoughParameters::ScoreType::LongestConnectedArc;
            break;
    }
	m_oScoreThreshold     = parameters_.getParameter("ScoreThreshold");

    bool oBoolSearchOutsideROI  = parameters_.getParameter("SearchOutsideROI");
    //SearchType::OnlyCenterInsideROI is implemented in CircleHoughImpl, but using it requires change the type of m_oSearchOutsideROI from bool to enum
    if (oBoolSearchOutsideROI)
    {
        m_oSearchOutsideROI = SearchType::AllowCenterOutsideROI;
    }
    else
    {
        m_oSearchOutsideROI = SearchType::OnlyCompleteCircleInsideROI;
    }

} // setParameter

/*virtual*/ bool CircleHough::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
    if ( p_rPipe.tag() == "image" )
    {
        m_pPipeInImageFrame		= dynamic_cast<image_pipe_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "radiusStart" )
    {
        m_pPipeInRadiusStart = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "radiusEnd" )
    {
        m_pPipeInRadiusEnd = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
    }
    else
    {
        wmLog(eWarning, "Unknown pipe %s \n", p_rPipe.tag().c_str());
        return false;
    }

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

/*virtual*/ void CircleHough::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
    poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInRadiusStart != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInRadiusEnd != nullptr); // to be asserted by graph editor

    // get data from frame

    const ImageFrame&		rFrameIn			( m_pPipeInImageFrame->read(m_oCounter) );
    const BImage&			rImageIn			( rFrameIn.data() );
    const auto & rRadiusStartIn = m_pPipeInRadiusStart->read(m_oCounter);
    const auto & rRadiusEndIn = m_pPipeInRadiusEnd->read(m_oCounter);

    m_oSpTrafo = rFrameIn.context().trafo();
    m_numValidCircles = 0;
    m_oPaintInputImage = false;

    // input validity check
    if ( rImageIn.isValid() == false
        || rRadiusStartIn.ref().size() == 0 || rRadiusStartIn.ref().getRank().front() == 0
        || rRadiusEndIn.ref().size() == 0 || rRadiusEndIn.ref().getRank().front() == 0
    )
    {
        GeoDoublearray				oEmptyVector	( rFrameIn.context(), geo2d::Doublearray(1), rFrameIn.analysisResult() );	// signal null image
        assert(oEmptyVector.ref().getRank().front() == 0);
        preSignalAction();
        m_oPipeOutX.signal( oEmptyVector );
        m_oPipeOutY.signal( oEmptyVector );
        m_oPipeOutR.signal( oEmptyVector );
        m_oPipeOutScore.signal( oEmptyVector);

        return;
    } // if

    if (rRadiusStartIn.ref().size() > 1 || rRadiusEndIn.ref().size() > 1)
    {
        wmLog(eDebug, "Filter '%s': Received multiple incoming data items, expected only one per pipe. Will process first element, rest will be discarded.\n", m_oFilterName.c_str() );
    }

#if DEBUG_CIRCLEHOUGH
    auto exportImage = [](std::string name, const BImage & image)
    {
        auto imageCopy = image.isContiguos() ? image : BImage(image.size());
        if (!image.isContiguos())
        {
            image.copyPixelsTo(imageCopy);
        }

        std::ostringstream oFilename;
        oFilename << "/tmp/circleHough_" << export_counter <<"__" << name << ".bmp";

        fileio::Bitmap oBitmap(oFilename.str(), imageCopy.width(), imageCopy.height(),false);
        if (oBitmap.isValid())
        {
            oBitmap.save(imageCopy.data());
        }
        else
        {
            wmLog(eError, oFilename.str() + "Error saving\n" );
        };
    };

    export_counter++;

    exportImage("input_" + std::to_string(rFrameIn.context().imageNumber()), rImageIn);
#endif

    m_oRadiusStart = rRadiusStartIn.ref().getData().front();
    m_oRadiusEnd = rRadiusEndIn.ref().getData().front();

    if (m_oVerbosity >= VerbosityType::eHigh)
    {
        m_oPaintInputImage = true;
        m_oResImageOut.resize(rImageIn.size());
        auto oWidth = rImageIn.width();
        for (int i=0, h = rImageIn.height(); i < h; ++i)
        {
            //threshold image
            std::transform(rImageIn[i], rImageIn[i] + oWidth, m_oResImageOut[i],
                [this](byte pixel){return (pixel > m_oIntensityThreshold) ? 255 :  0;});
        }

#if DEBUG_CIRCLEHOUGH
        exportImage("threshold_" + std::to_string(rFrameIn.context().imageNumber()), m_oResImageOut);
#endif
    }
    m_oCircles.clear();

    doCircleHough(rImageIn); // image processing

    //sort by score value - descending order
    std::sort(m_oCircles.begin(), m_oCircles.end(), [] (const hough_circle_t & a, const hough_circle_t & b) {
                                                        return a.second > b.second;});

    GeoDoublearray	oGeoX	( rFrameIn.context(), geo2d::Doublearray(), rFrameIn.analysisResult() );
    GeoDoublearray	oGeoY	( rFrameIn.context(), geo2d::Doublearray(), rFrameIn.analysisResult() );
    GeoDoublearray	oGeoR	( rFrameIn.context(), geo2d::Doublearray(), rFrameIn.analysisResult() );
    GeoDoublearray	oGeoScore( rFrameIn.context(), geo2d::Doublearray(), rFrameIn.analysisResult() );

    if (m_oCircles.size() > 0)
    {
        m_numValidCircles =  m_oScoreThreshold < 0 ? 1 :
            std::distance(m_oCircles.begin(), std::find_if(m_oCircles.begin(), m_oCircles.end(), [this](const hough_circle_t & a)
                            {
                                return a.second < m_oScoreThreshold;
                            }));

        //alway output only one circle, the score threshold decides the rank
        auto oNumOutputCircles = 1;
        for (auto pArray : {&oGeoX.ref(), &oGeoY.ref(), &oGeoR.ref(), &oGeoScore.ref()})
        {
            pArray->getData().reserve(oNumOutputCircles);
            pArray->getRank().reserve(oNumOutputCircles);
        }
        for (int i = 0; i < oNumOutputCircles; i++)
        {
            auto & rHoughCircle = m_oCircles[i];
            int rankCircle =  (m_oScoreThreshold < 0 || rHoughCircle.second >= m_oScoreThreshold ) ? eRankMax : eRankMin;
            oGeoX.ref().getData().push_back(rHoughCircle.first.m_middleX);
            oGeoY.ref().getData().push_back(rHoughCircle.first.m_middleY);
            oGeoR.ref().getData().push_back(rHoughCircle.first.m_radius);
            oGeoScore.ref().getData().push_back(rHoughCircle.second);
            oGeoScore.ref().getRank().push_back(eRankMax);
            for (auto pArray : {&oGeoX.ref(), &oGeoY.ref(), &oGeoR.ref()})
            {
                pArray->getRank().push_back(rankCircle) ;
            }
        }

        //set a valid global rank
        oGeoX.rank(1.0);
        oGeoY.rank(1.0);
        oGeoR.rank(1.0);
        oGeoScore.rank(1.0);
    }
    else
    {
        oGeoX.ref().assign(1);
        oGeoY.ref().assign(1);
        oGeoR.ref().assign(1);
        oGeoScore.ref().assign(1);
        assert(oGeoR.ref().getRank().front() == 0);
    }


    preSignalAction();
    m_oPipeOutX.signal( oGeoX );
    m_oPipeOutY.signal( oGeoY );
    m_oPipeOutR.signal( oGeoR );
    m_oPipeOutScore.signal( oGeoScore );

} // proceed

/*virtual*/ void CircleHough::paint()
{
    if ((m_oVerbosity <= eNone) || m_oSpTrafo.isNull())
    {
        return;
    }

    const Trafo					&rTrafo(*m_oSpTrafo);
    OverlayCanvas				&rCanvas(canvas<OverlayCanvas>(m_oCounter));
    OverlayLayer				&rLayerImage(rCanvas.getLayerImage());
    OverlayLayer				&rLayerContour(rCanvas.getLayerContour());
	OverlayLayer				&rLayerText( rCanvas.getLayerText());


    const auto		oPosition = rTrafo(Point(0, 0));
    if (m_oPaintInputImage)
    {
        const auto		oTitle = OverlayText("CircleHough input Image", Font(), Rect(150, 18), Color::Black());
        rLayerImage.add<OverlayImage>(oPosition, m_oResImageOut, oTitle);
    }
    if (m_oVerbosity >= VerbosityType::eHigh)
    {
        geo2d::Point position {m_oSpTrafo->dx(), m_oSpTrafo->dy()};
        auto oTxt = std::string{"r start="} + std::to_string( (int) (m_oRadiusStart))
                    + std::string{" r end="} + std::to_string( (int) (m_oRadiusEnd));
        if (m_numValidCircles  > 0)
        {
            oTxt += std::string{" fitted radius ="} + std::to_string( (int) (m_oCircles.front().first.m_radius) );
            oTxt += std::string{" score ="} + std::to_string( (int) (m_oCircles.front().second) );
        }
        rLayerText.add<OverlayText>(oTxt, Font(14), geo2d::Rect(position.x, position.y, 300, 20), Color::Red());
    }
    //use a different color for each radius
    static std::vector<Color> colors = {
        Color::Red(), Color::Cyan(), Color::Green(), Color::Yellow(), Color::Orange(), Color::Magenta()
    };

    if (m_numValidCircles == 0)
    {
        return;
    }

    std::map<double,Color> legend;

    int numCirclesToPaint = (m_oVerbosity >= VerbosityType::eMedium) ? m_numValidCircles : 1;
    double minScore =  m_oCircles.size() <= 1 ? 0.0 : m_oCircles.back().second -1;
    double deltaScore = m_oCircles.size() <= 1 ? 1.0 : m_oCircles.front().second - minScore;
    if (deltaScore == 0)
    {
        //avoid division by zero when computing overlay transparency
        deltaScore = 1.0;
        minScore = 0.0;
    }

    for (int i = 0; i < numCirclesToPaint; i++)
    {
        auto & rHoughCircle = m_oCircles[i];

        auto & rCircle = rHoughCircle.first;
        auto itColor = legend.find(rCircle.m_radius);
        Color curColor;
        if (itColor == legend.end())
        {
            auto color_index = (legend.size()) % colors.size();
            curColor = colors[color_index];
            legend[rCircle.m_radius] = curColor ;
        }
        else
        {
            curColor = itColor->second;
        }
        int x = oPosition.x + (int) std::round(rCircle.m_middleX);
        int y = oPosition.y + (int) std::round(rCircle.m_middleY);

        curColor.alpha = (rHoughCircle.second - minScore) * 255.0 / deltaScore;

        rLayerContour.add<OverlayCircle>(x,y, (int) std::round(rCircle.m_radius), curColor);
        if (m_oVerbosity >= VerbosityType::eHigh)
        {
            geo2d::Point position { x + (int) rCircle.m_radius + 2, y};
            auto oTxt = std::string{"r="} + std::to_string( (int) (rCircle.m_radius))
                        + std::string{" ("} + std::to_string(rHoughCircle.second) + std::string{")"};
            rLayerText.add<OverlayText>(oTxt, Font(14), geo2d::Rect(position.x, position.y, 200, 20), curColor);
        }

    }

} // paint

void CircleHough::doCircleHough(const BImage& p_rImageIn)
{
    CircleHoughImpl algoImpl;

    CircleHoughParameters oParameters;
    oParameters.m_oRadiusStart = m_oRadiusStart;
    oParameters.m_oRadiusEnd = m_oRadiusEnd;
    oParameters.m_oRadiusStep = m_oRadiusStep;
    oParameters.m_oNumberMax = m_oNumberMax;
    oParameters.m_oCoarse = true;
    oParameters.m_oScoreType = m_oScoreType;
    oParameters.m_oConnectedArcToleranceDegrees = 1.0;

    wmLog(eDebug, "CircleHough: using hough coarse = %d candidates = %d verify %d \n",
          oParameters.m_oCoarse, oParameters.m_oNumberMax, int(oParameters.m_oScoreType == CircleHoughParameters::ScoreType::LongestConnectedArc)); //FIXME remove debug statement

    m_oCircles = algoImpl.DoCircleHough(p_rImageIn, m_oIntensityThreshold, m_oSearchOutsideROI, oParameters);
}


} // namespace filter
} // namespace precitec
